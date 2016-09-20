/** Copyright 2015 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomImport.cc
//
//	Description:
//		Imports and Exports .atom Files		
//
//

#include <stdlib.h>
#include <string.h>
#include <vector>
#include <maya/MFStream.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MFnPlugin.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MAnimCurveClipboard.h>

#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MItDag.h>
#include <maya/MItDependencyNodes.h>

#include <maya/MFnAnimCurve.h>
#include <maya/MAnimUtil.h>
#include <maya/MFnDagNode.h>
#include <maya/MDagPath.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>
#include <maya/MProgressWindow.h>
#include <maya/MAnimControl.h>
#include <maya/MDagModifier.h>

#include <maya/MMessage.h>
#include <maya/MSceneMessage.h>


#include "atomImportExport.h"
#include "atomFileUtils.h"
#include "atomImportExportStrings.h"
#include "atomNodeNameReplacer.h"
#include "atomCachedPlugs.h"

#if defined (OSMac_)
#	include <sys/param.h>
extern "C" int strcasecmp (const char *, const char *);
#endif
//-----------------------------------------------------------------------------
//	anim Importer
//-----------------------------------------------------------------------------

//todo add a script if we want const char *const animImportOptionScript = "animImportOptions";
const char *const animImportDefaultOptions = 
	"targetTime=4;copies=1;option=replace;pictures=0;connect=0;match=hierarchy;";

// Register all strings used by the plugin C++ code
/*
Not Used
static MStatus registerMStringResources(void)
{
	MStringResource::registerString(kNothingSelected);
	MStringResource::registerString(kPasteFailed);
	MStringResource::registerString(kAnimCurveNotFound);		
	MStringResource::registerString(kInvalidAngleUnits);
	MStringResource::registerString(kInvalidLinearUnits);
	MStringResource::registerString(kInvalidTimeUnits);
	MStringResource::registerString(kInvalidVersion);
	MStringResource::registerString(kSettingToUnit);
	MStringResource::registerString(kMissingKeyword);
	MStringResource::registerString(kCouldNotReadAnim);
	MStringResource::registerString(kCouldNotCreateAnim);	
	MStringResource::registerString(kUnknownKeyword);
	MStringResource::registerString(kClipboardFailure);
	MStringResource::registerString(kSettingTanAngleUnit);
	MStringResource::registerString(kUnknownNode);
	MStringResource::registerString(kCouldNotKey);
	MStringResource::registerString(kMissingBrace);
	MStringResource::registerString(kCouldNotExport);
	return MS::kSuccess;
}
*/


atomImport::atomImport()
: MPxFileTranslator()
{
}

atomImport::~atomImport()
{
}


void *atomImport::creator()
{
	return new atomImport();
}


bool atomImport::replaceNameAndFindPlug(const MString& origName,
										atomNodeNameReplacer& replacer,
										MPlug& replacedPlug)
{
	bool rtn = false;
	
	// get the node name
	//
	MStringArray nameParts;
	origName.split('.', nameParts);

	// Perform any necessary replacement
	//
	MString tmpName(nameParts[0]);
	// TODO: type & hierarchy info -- does the replacer store enough info
	// to help us find that out since in the case of export edits we don't
	// have that info for sources
	//
	if (replacer.findNode(atomNodeNameReplacer::eDag,tmpName,0,0)) {
		MString newName(tmpName);
		newName += (".");

		// add the attribute name(s) back on again
		//
		unsigned int ii;
		MString attrName;
		for (ii = 1; ii < nameParts.length(); ++ii) {
			if (ii > 1) {
				attrName += (".");
			}
			attrName += nameParts[ii];
		}
		newName += attrName;

		MSelectionList tmpList;
		if (MS::kSuccess == tmpList.add(newName)) {
			tmpList.getPlug(0,replacedPlug);
			rtn = !replacedPlug.isNull();
			if (!rtn) {
				// test for the special case of the pivot component
				//
				MDagPath path;
				MObject component;
				if (MS::kSuccess == tmpList.getDagPath(0,path,component) &&
					component.apiType() == MFn::kPivotComponent)
				{
					MObject node;
					tmpList.getDependNode(0,node);
					MFnDependencyNode fnNode(node);
					replacedPlug = fnNode.findPlug(attrName,false);
					rtn = !replacedPlug.isNull();
				}
			}
		}
	}
	return rtn;
}


bool
atomImport::checkPlugAgainstTemplate(const MString& nodeName,
									 const MPlug& plug,
									 atomTemplateReader* templateReader)
{
	// Check if the plug is filtered out
	//
	if (NULL != templateReader) {
		// get long attribute name
		MString plugName = plug.partialName(true,false,false,false,false,true);
		MStringArray nameParts;
		plugName.split('.', nameParts);
		MString leafAttr = nameParts[nameParts.length()-1];
		return (templateReader->findNodeAndAttr(nodeName,leafAttr));
	}
	// No template, nothing to do
	//
	return true;
}

void atomImport::connectionFailedCallback(MPlug& srcPlug,
										  MPlug& dstPlug,
										  const MString& srcName,
										  const MString& dstName,
										  void* clientData)
{
//	MString output = "Connection failed callback: ";
//	output += srcName;  output += " ";	output += dstName;
//	MGlobal::displayInfo(output);

	atomEditsHelper* helper = (NULL != clientData) ? (atomEditsHelper*)clientData : NULL;
	atomNodeNameReplacer* replacer = (NULL != helper) ? helper->fReplacer : NULL;
	atomTemplateReader* templateReader = (NULL != helper) ? helper->fTemplateReader : NULL;

	if (NULL != replacer && srcPlug.isNull()) {
		// Import of the edits didn't find a match for the source name, use the 
		// replacer and see if that helps
		//
		if (replaceNameAndFindPlug(srcName,*replacer,srcPlug)) {
			if (!dstPlug.isNull()) {
				// we've found the proper source plug to use and we already
				// had a dest, so connect them up and we're done
				//
				MDagModifier mod;
				mod.connect(srcPlug,dstPlug);
				return;
			}
		}
	}

	if (NULL != replacer && dstPlug.isNull()) {
		// Import of the edits didn't find a match for the dest name, use the 
		// replacer and see if that helps
		//
		if (replaceNameAndFindPlug(dstName,*replacer,dstPlug)) {
			MStringArray dstParts;
			dstName.split('.', dstParts);			
			if (!checkPlugAgainstTemplate(dstParts[0],dstPlug,templateReader))
				return;

			if (!srcPlug.isNull()) {
				// we've found the proper dest plug to use and we already
				// had a source, so connect them up and we're done
				//
				MDagModifier mod;
				mod.connect(srcPlug,dstPlug);
				return;
			}
		}
	}
	
	if (!dstPlug.isNull()) {
		
		MObject dstNode = dstPlug.node();

		// Check whether the failed connection was to a setDrivenKey curve
		//
		if (dstNode.hasFn(MFn::kAnimCurveUnitlessToAngular) ||
			dstNode.hasFn(MFn::kAnimCurveUnitlessToDistance) ||
			dstNode.hasFn(MFn::kAnimCurveUnitlessToTime) ||
			dstNode.hasFn(MFn::kAnimCurveUnitlessToUnitless)) {

			// If so, create a stand-in driver for that curve
			//
			MDagModifier mod;
			MObject locator = mod.createNode( "locator", MObject::kNullObj );
			MFnDependencyNode fnLoc(locator);

			MStringArray nameParts;
			srcName.split('.', nameParts);
			MString leafAttr(nameParts[nameParts.length()-1]);
			MPlug leafPlug = fnLoc.findPlug(leafAttr);
			if (!leafPlug.isNull()) {
				mod.connect(leafPlug,dstPlug);

				// rename the locator to the name of the original source
				// so that any subsequent connections will work
				//
				fnLoc.setName(nameParts[0]);
			}
		}
	}
}


MStatus atomImport::reader(	const MFileObject& file,
								const MString& options,
								FileAccessMode mode)
{
	MStatus status = MS::kFailure;

	MString fileName = file.fullName();
#if defined (OSMac_)	
	char fname[MAXPATHLEN];
	strcpy (fname, fileName.asChar());
	ifstream animFile(fname);
#else
	ifstream animFile(fileName.asChar());
#endif
	// 	Parse the options. The options syntax is in the form of
	//	"flag=val;flag1=val;flag2=val"
	//

	if(animFile.good()==false)
		return status;
	MString pasteFlags;
	MString	prefix;
	MString	suffix;
	MString	search;
	MString	replace;
	MString	mapFile;
	bool replaceLayers = false;
	MString	exportEditsFile;	
	bool includeChildren = false;
	atomNodeNameReplacer::ReplaceType type = atomNodeNameReplacer::eHierarchy;
	MString templateName;
	MString viewName;
	bool useTemplate = false;

	if (options.length() > 0) {
		//	Set up the flags for the paste command.
		//
		const MString flagSrcTime("srcTime");
		const MString flagDstTime("dstTime");
		const MString flagOldDstTime("time");
		const MString flagCopies("copies");
		const MString flagOption("option");
		const MString flagConnect("connect");
		const MString flagMatch("match");
		const MString flagSearch("search");
		const MString flagReplace("replace");
		const MString flagPrefix("prefix");
		const MString flagSuffix("suffix");
		const MString flagMapFile("mapFile");
		const MString flagHierarchy("hierarchy");
		const MString flagString("string");
		const MString flagSelected("selected");
		const MString flagTemplate("template");
		const MString flagView("view");
		const MString optionChildrenToo("childrenToo");
		const MString optionTemplate("template");
		const MString flagExportEdits("exportEdits");
		MString copyValue;
		MString flagValue;
		MString connectValue;
		MString match;
		MString srcTimeValue;
		MString dstTimeValue;

		//	Start parsing.
		//
		MStringArray optionList;
		MStringArray theOption;
		options.split(';', optionList);

		unsigned nOptions = optionList.length();
		for (unsigned i = 0; i < nOptions; i++) {

			theOption.clear();
			optionList[i].split('=', theOption);
			if (theOption.length() < 1) {
				continue;
			}

			if (theOption[0] == flagCopies && theOption.length() > 1) {
				copyValue = theOption[1];;
			} else if (theOption[0] == flagOption && theOption.length() > 1) {
				flagValue = theOption[1];
			} else if (theOption[0] == flagConnect && theOption.length() > 1) {
				if (theOption[1].asInt() != 0) {
					connectValue += theOption[1];
				}
			} 
			else if( theOption[0] == flagTemplate && theOption.length() > 1)
			{
				templateName = theOption[1];
			}
			else if( theOption[0] == flagView && theOption.length() > 1)
			{
				viewName = theOption[1];
			}
			else if (theOption[0] == flagSrcTime && theOption.length() > 1) {
				srcTimeValue += theOption[1];
			}
			else if ((theOption[0] == flagDstTime || theOption[0] == flagOldDstTime )&& theOption.length() > 1) {
				dstTimeValue += theOption[1];
			}
			else if (theOption[0] == flagMatch && theOption.length() > 1) {
				match =  theOption[1];
			} 
			else if (theOption[0] == flagSearch && theOption.length() > 1) {
				search =  theOption[1];
			} 
			else if (theOption[0] == flagReplace && theOption.length() > 1) {
				replace =  theOption[1];
			} 
			else if (theOption[0] == flagPrefix && theOption.length() > 1) {
				prefix =  theOption[1];
			} 
			else if (theOption[0] == flagSuffix && theOption.length() > 1) {
				suffix =  theOption[1];
			} 
			else if (theOption[0] == flagMapFile && theOption.length() > 1) {
				mapFile =  theOption[1];
			}
			else if (theOption[0] == flagSelected && theOption.length() > 1) {
				includeChildren =   (theOption[1] == optionChildrenToo) ? true : false;
				if(theOption[1] == optionTemplate)
					useTemplate = true;
			}
			else if (theOption[0] == flagExportEdits && theOption.length() > 1) {
				exportEditsFile =  theOption[1];
			}
		}
	
		if (copyValue.length() > 0) {
			pasteFlags += " -copies ";
			pasteFlags += copyValue;
			pasteFlags += " ";
		} 
		if (flagValue.length() > 0) {
			pasteFlags += " -option \"";
			pasteFlags += flagValue;
			pasteFlags += "\" ";
			if(flagValue == MString("replace"))
				replaceLayers = true;
		} 
		if (connectValue.length() > 0) {
			pasteFlags += " -connect ";
			pasteFlags += connectValue;
			pasteFlags += " ";
		} 
		if (dstTimeValue.length() > 0) {
			bool useQuotes = !dstTimeValue.isDouble();
			pasteFlags += " -time ";
			if (useQuotes) pasteFlags += "\"";
			pasteFlags +=  dstTimeValue;
			if (useQuotes) pasteFlags += "\"";
			pasteFlags += " ";
		} 		
		if (srcTimeValue.length() > 0) 
		{
			MTime lClipStartTime;
			MTime lClipEndTime;
			MStringArray lTimes;

			if ( MStatus::kSuccess == srcTimeValue.split( L':', lTimes ) )
			{
				if ( lTimes.length() > 0 )
				{
					double lImportStartFrame = lTimes[0].asDouble();
					double lImportEndFrame   = lImportStartFrame;
					
					if ( lTimes.length() > 1 )
					{
						lImportEndFrame = lTimes[1].asDouble();
					}
					fReader.setImportFrameRange( lImportStartFrame, lImportEndFrame );
				}
				else
				{
					fReader.clearImportFrameRange();
				}
			}
		} 
        else
        {
            fReader.clearImportFrameRange();
        }
		if(match.length() >0)
		{
			if(match == flagHierarchy)
				type = atomNodeNameReplacer::eHierarchy;
			else if(match == flagString)
				type = atomNodeNameReplacer::eSearchReplace;
			else if(match == flagMapFile)
				type = atomNodeNameReplacer::eMapFile;
		} //not set, then we leave what we had

		
	}

	//	If the selection list is empty, there is nothing to import.
	//
	MSelectionList sList;
	std::vector<unsigned int> depths;
	atomTemplateReader templateReader;
	if(useTemplate == true)
	{
		templateReader.setTemplate(templateName,viewName);
		includeChildren = false;
		templateReader.selectNodes(); //make the selection set be us.
	}
	SelectionGetter::getSelectedObjects(includeChildren,sList,depths);
	if (sList.isEmpty()) {
		MString msg = MStringResource::getString(kNothingSelected, status);
		MGlobal::displayError(msg);
		return (MS::kFailure);
	}


	atomNodeNameReplacer replacer(type,sList,depths,prefix,suffix,search,replace,mapFile);
	if (mode == kImportAccessMode) {
		status = importAnim(sList,animFile,pasteFlags,replacer,exportEditsFile,templateReader,replaceLayers);
	}

	animFile.close();
	return status;
}

bool atomImport::haveReadMethod() const
{
	return true;
}

bool atomImport::haveWriteMethod() const
{
	return false;
}

bool atomImport::canBeOpened() const
{
	return false;
}

MString atomImport::defaultExtension() const
{
	return MString("atom");
}

MPxFileTranslator::MFileKind atomImport::identifyFile(
								const MFileObject& fileName,
								const char* buffer,
								short size) const
{
	const char *name = fileName.name().asChar();
	int   nameLength = (int)strlen(name);

	if ((nameLength > 5) && !strcasecmp(name+nameLength-5, ".atom")) {
		return kIsMyFileType;
	}

	//	Check the buffer to see if this contains the correct keywords
	//	to be a anim file.
	//
	if (strncmp(buffer, "atomVersion", 11) == 0) {
		return kIsMyFileType;
	}

	return	kNotMyFileType;
}

MStatus 
atomImport::importAnim(MSelectionList &sList,ifstream &animFile, const MString &pasteFlags, atomNodeNameReplacer & replacer,
						MString& exportEditsFile,atomTemplateReader &templateReader, bool replaceLayers)
{
	MStatus status = MS::kFailure;
	MGlobal::setActiveSelectionList(sList);

	MString atomExportEdits;
	atomLayerClipboard atomClipboard;
	bool removeExportEditsFile = false; //if this becomes true we need to remove the temporary edit file we create.
	if (MS::kSuccess != 
			(status = fReader.readAtom(animFile, atomClipboard,
									   sList,replacer,atomExportEdits,removeExportEditsFile,templateReader, replaceLayers
				))) {

		return status;
	}
	if (atomExportEdits.length() > 0 && exportEditsFile.length() == 0) {
		// If the user specified an edits file via command line, we'll
		// use it instead of the one from the file
		//
		exportEditsFile = atomExportEdits;
	}
	else
		removeExportEditsFile = false; //don't remove it we are using the one from the commandline.
	if (exportEditsFile.length() > 0) {
		atomEditsHelper helper(&replacer,&templateReader);
		
		MCallbackId c_id = MSceneMessage::addConnectionFailedCallback( &atomImport::connectionFailedCallback,&helper);
		replacer.setAddMainPlaceholderNamespace(true);

		// Import the exportEdits file
		//
		MSelectionList sel;
		MGlobal::getActiveSelectionList(sel);
		MString command = "doImportAtomOfflineFile(1,{\"";
		command += exportEditsFile;
		command += "\"})";
		status =  MGlobal::executeCommand(command, false, false);
		
		MSceneMessage::removeCallback(c_id);
		MGlobal::setActiveSelectionList(sel);
		replacer.setAddMainPlaceholderNamespace(false);
		if(removeExportEditsFile)
			remove(exportEditsFile.asChar());

	}

	status =  atomClipboard.pasteKeys(fReader.getStartTime(), fReader.getEndTime(), 
						(float) fReader.getStartUnitless(), (float) fReader.getEndUnitless(),pasteFlags);
	//Restore the oldUnits
	MDistance::setUIUnit(fReader.getOldDistanceUnit());
	MTime::setUIUnit(fReader.getOldTimeUnit());
	return status;

}

//-----------------------------------------------------------------------------
//	anim Exporter
//-----------------------------------------------------------------------------

//todo const char *const animExportOptionScript = "atomExportOptions";
const char *const animExportDefaultOptions = "whichRange=1;range=0:10;options=keys;hierarchy=none;controlPoints=0;useChannelBox=0;copyKeyCmd=";

const int kDefaultPrecision = 8;	//	float precision.

atomExport::atomExport()
: MPxFileTranslator()
{
}

atomExport::~atomExport()
{
}

void *atomExport::creator()
{
	return new atomExport();
}

MStatus atomExport::writer(	const MFileObject& file,
								const MString& options,
								FileAccessMode mode)
{
	MStatus status = MS::kFailure;


	MString fileName = file.fullName();
#if defined (OSMac_)
	char fname[MAXPATHLEN];
	strcpy (fname, fileName.asChar());
	ofstream animFile(fname);
#else
	ofstream animFile(fileName.asChar());
#endif
	//	Defaults.
	//
	MString copyFlags("copyKey -cb api -fea 1 ");
	int precision = kDefaultPrecision;
	bool statics = false;
	bool includeChildren = false;
	std::set<std::string> attrStrings;
	//	Parse the options. The options syntax is in the form of
	//	"flag=val;flag1=val;flag2=val"
	//
	bool useSpecifiedRange = false;
	bool useTemplate = false;
	bool cached = false;
	bool constraint = false;
	bool sdk = false;
	bool animLayers = true;
	MString templateName;
	MString viewName;
	MTime startTime = MAnimControl::animationStartTime();
	MTime endTime = MAnimControl::animationEndTime();
	MString	exportEditsFile;

	MString exportFlags;
	if (options.length() > 0) {
		const MString flagPrecision("precision");
		const MString flagStatics("statics");
		const MString flagConstraint("constraint");
		const MString flagSDK("sdk");
		const MString flagAnimLayers("animLayers");
		const MString flagCopyKeyCmd("copyKeyCmd");
		const MString flagSelected("selected");
		const MString flagTemplate("template");
		const MString flagView("view");
		const MString optionChildrenToo("childrenToo");
		const MString optionTemplate("template");
		const MString flagAttr("at");
		const MString flagWhichRange("whichRange");
		const MString flagRange("range");
		const MString flagExportEdits("exportEdits");		
		const MString flagCached("baked");		
		
		//	Start parsing.
		//
		MStringArray optionList;
		MStringArray theOption;
		options.split(';', optionList);

		unsigned nOptions = optionList.length();
		for (unsigned i = 0; i < nOptions; i++) {
			theOption.clear();
			optionList[i].split('=', theOption);
			if (theOption.length() < 1) {
				continue;
			}

			if (theOption[0] == flagPrecision && theOption.length() > 1) {
				if (theOption[1].isInt()) {
					precision = theOption[1].asInt();
				}
			} 
			else if( theOption[0] == flagTemplate && theOption.length() > 1)
			{
				templateName = theOption[1];
			}
			else if( theOption[0] == flagView && theOption.length() > 1)
			{
				viewName = theOption[1];
			}
			else if (	theOption[0] == 
						flagWhichRange && theOption.length() > 1) {
				if (theOption[1].isInt()) 
					useSpecifiedRange = (theOption[1].asInt() ==1) ? false : true;
			}
			else if (	theOption[0] == 
						flagRange && theOption.length() > 1) 
			{
				MStringArray rangeArray;
				theOption[1].split(':',rangeArray);
				if(rangeArray.length()==2)
				{
					if(rangeArray[0].isDouble())
					{
						double val = rangeArray[0].asDouble();
						startTime = MTime(val,MTime::uiUnit());
					}
					else if(rangeArray[0].isInt())
					{
						double val = (double)rangeArray[0].asInt();
						startTime = MTime(val,MTime::uiUnit());
					}
					if(rangeArray[1].isDouble())
					{
						double val = rangeArray[1].asDouble();
						endTime = MTime(val,MTime::uiUnit());
					}
					else if(rangeArray[1].isInt())
					{
						double val = (double)rangeArray[1].asInt();
						endTime = MTime(val,MTime::uiUnit());
					}
				}
			}
			else if (	theOption[0] == 
						flagStatics && theOption.length() > 1) {
				if (theOption[1].isInt()) {
					statics = (theOption[1].asInt()) ? true : false;
				}
			}
			else if (	theOption[0] == 
						flagSDK && theOption.length() > 1) {
				if (theOption[1].isInt()) {
					sdk = (theOption[1].asInt()) ? true : false;
				}
			}
			else if (	theOption[0] == 
						flagConstraint && theOption.length() > 1) {
				if (theOption[1].isInt()) {
					constraint = (theOption[1].asInt()) ? true : false;
				}
			}
			else if (	theOption[0] == 
						flagAnimLayers && theOption.length() > 1) {
				if (theOption[1].isInt()) {
					animLayers = (theOption[1].asInt()) ? true : false;
				}
			}
			else if (	theOption[0] == 
						flagCached && theOption.length() > 1) {
				if (theOption[1].isInt()) {
					cached = (theOption[1].asInt()) ? true : false;
				}
			}
			else if (theOption[0] == flagSelected && theOption.length() > 1) {
				includeChildren = (theOption[1] == optionChildrenToo) ? true : false;
				if(theOption[1] == optionTemplate)
					useTemplate = true;
			} 
			else if (theOption[0] == flagAttr && theOption.length() > 1) {
				std::string str(theOption[1].asChar());
				attrStrings.insert(str);
			} 
			else if (	theOption[0] == 
						flagCopyKeyCmd && theOption.length() > 1) {

				//	Replace any '>' characters with '"'. This is needed
				//	since the file translator option boxes do not handle
				//	escaped quotation marks.
				//
				const char *optStr = theOption[1].asChar();
				size_t nChars = strlen(optStr);
				char *copyStr = new char[nChars+1];

				copyStr = strcpy(copyStr, optStr);
				for (size_t j = 0; j < nChars; j++) {
					if (copyStr[j] == '>') {
						copyStr[j] = '"';
					}
				}
		
				copyFlags += copyStr;
				delete [] copyStr;
			}
			else if (theOption[0] == flagExportEdits && theOption.length() > 1)
			{
				exportEditsFile =  theOption[1];
			}
		}
	}
	
	//	Set the precision of the ofstream.
	//
	animFile.precision(precision);


	atomTemplateReader templateReader;
	if(useTemplate == true)
	{
		includeChildren = false;
		templateReader.setTemplate(templateName,viewName);
		templateReader.selectNodes(); //make the template nodes be the selection
	}
	status = exportSelected(animFile, copyFlags, attrStrings, includeChildren,
							useSpecifiedRange, startTime, endTime, statics,
							cached,sdk,constraint, animLayers, exportEditsFile,templateReader);

	animFile.flush();
	animFile.close();

	return status;
}

bool atomExport::haveReadMethod() const
{
	return false;
}

bool atomExport::haveWriteMethod() const
{
	return true;
}

MString atomExport::defaultExtension() const
{
	return MString("atom");
}

MPxFileTranslator::MFileKind atomExport::identifyFile(
								const MFileObject& fileName,
								const char* buffer,
								short size) const
{
	const char *name = fileName.name().asChar();
	int   nameLength = (int)strlen(name);

	if ((nameLength > 5) && !strcasecmp(name+nameLength-5, ".atom")) {
		return kIsMyFileType;
	}

	return	kNotMyFileType;
}


MStatus atomExport::writeAnimCurves(ofstream &animFile,MString &nodeName,atomCachedPlugs *cachedPlugs,
					atomNodeWithAnimLayers *layerPlugs, MString &command, bool &haveAnimatedCurves,
					atomTemplateReader &templateReader)
{
	int result = 0;
	MString templateAttrs("");
	if(templateReader.isTemplateSet())
	{
		templateAttrs = templateReader.attributesForNode(nodeName);
		if(templateAttrs.length()==0) //no attrs set for this node so abort
			return MS::kSuccess; //still success no failure just template worked and filtered it out.
	}
	MString layerName;
	std::set<std::string> layerNames;
	std::set<std::string>::iterator iter;
	bool isLayered = layerPlugs && layerPlugs->isNodeLayered(layerNames);
	iter = layerNames.begin();
	
	do //do loop at least once
	{
		if(isLayered && iter != layerNames.end())
		{
			std::string val = *iter;
			layerName = MString(val.c_str());
		}
		else
			isLayered =false;

		MString copyFromOne;
		if(isLayered == false || layerName.length()==0 )
			copyFromOne = command + MString(" ") + templateAttrs + MString(" ") + nodeName;
		else
			copyFromOne = command + MString(" -al ") + layerName + MString(" ") + templateAttrs + MString(" ") + nodeName;
		haveAnimatedCurves = true;
		if (MS::kSuccess != (MGlobal::executeCommand(copyFromOne, result, false, true))) 
		{
			haveAnimatedCurves = false;
		}

		if (result == 0 || MAnimCurveClipboard::theAPIClipboard().isEmpty()) 
		{
			haveAnimatedCurves = false;
		}

		if (haveAnimatedCurves && MS::kSuccess != (	fWriter.writeClipboard(animFile, 
							MAnimCurveClipboard::theAPIClipboard(),cachedPlugs,layerName))) 
		{
			haveAnimatedCurves = false;
			return (MS::kFailure);
		}
		if(isLayered && iter != layerNames.end())
		{
			++iter;

		}
	}while (isLayered && iter != layerNames.end());
	return MS::kSuccess;
}

MStatus atomExport::exportSelected(	ofstream &animFile, 
									MString &copyFlags,
									std::set<std::string> &attrStrings,
									bool includeChildren, 
									bool useSpecifiedTimes, 
									MTime &startTime,
									MTime &endTime,
									bool statics,
									bool cached,
									bool sdk,
									bool constraint,
									bool layers,
									const MString& exportEditsFile,
									atomTemplateReader &templateReader)
{
	MStatus status = MS::kFailure;

	//	If the selection list is empty, then there are no anim curves
	//	to export.
	//
	MSelectionList sList;
	std::vector<unsigned int> depths;


	SelectionGetter::getSelectedObjects(includeChildren,sList,depths);
	if (sList.isEmpty()) {
		MString msg = MStringResource::getString(kNothingSelected, status);
		MGlobal::displayError(msg);
		return (MS::kFailure);
	}
	//	Copy any anim curves to the API clipboard.
	//
	MString command(copyFlags);
	


	// Always write out header
	if (!fWriter.writeHeader(animFile,useSpecifiedTimes,
							 startTime,endTime)) {
		return (MS::kFailure);
	}


	atomAnimLayers animLayers;
	std::vector<atomNodeWithAnimLayers *> nodesWithAnimLayers;
	if(layers)
	{
		bool hasAnimLayers =  animLayers.getOrderedAnimLayers(); //any layers in the scene?
		hasAnimLayers = setUpAnimLayers(sList,animLayers, nodesWithAnimLayers,attrStrings,templateReader);
		//any layers on our selection?
		if(hasAnimLayers)
		{
			//add the layers to the sList...
			unsigned int oldLength = sList.length();
			animLayers.addLayersToStartOfSelectionList(sList);
			unsigned int diffLength = sList.length() - oldLength;
			atomNodeWithAnimLayers * nullPad = NULL;
			for(unsigned int k =0 ;k < diffLength;++k) //need to pad the beginning of the nodesWithAnimlayers with any layer that was added
			{
				nodesWithAnimLayers.insert(nodesWithAnimLayers.begin(),nullPad);
				depths.insert(depths.begin(),0);
			}
		}
	}
	
	//if caching is on, we pre iterate through the objects, find 
	//each plug that's cached and then cache the data all at once
	std::vector<atomCachedPlugs *> cachedPlugs;
	if(cached)
	{
		bool passed = setUpCache(sList,cachedPlugs,animLayers,sdk, constraint, layers, attrStrings,templateReader,startTime, endTime,
			fWriter.getAngularUnit(), fWriter.getLinearUnit()); //this sets it up and runs the cache;
		if(passed == false) //failed for some reason, one reason is that the user canceled the computation
		{
			//first delete everything though
			//delete any cachedPlugs objects that we created.
			for(unsigned int z = 0; z< cachedPlugs.size(); ++z)
			{
				if(cachedPlugs[z])
					delete cachedPlugs[z];
			}
			//and delete any any layers too
			for(unsigned int zz = 0; zz< nodesWithAnimLayers.size(); ++zz)
			{
				if(nodesWithAnimLayers[zz])
					delete nodesWithAnimLayers[zz];
			}
			MString msg = MStringResource::getString(kCachingCanceled, status);
			MGlobal::displayError(msg);
			return (MS::kFailure);
		}
	}

	unsigned int numObjects = sList.length();

	bool computationFinished = true;
	//not sure if in a headless mode we may want to not show the progress, should
	//still run if that's the case
	bool hasActiveProgress = false;
	if (MProgressWindow::reserve()) {
		hasActiveProgress = true;
		MProgressWindow::setInterruptable(true);
   		MProgressWindow::startProgress();
    	MProgressWindow::setProgressRange(0, numObjects);
		MProgressWindow::setProgress(0);
		MStatus stringStat;
		MString msg = MStringResource::getString(kExportProgress, stringStat);
		if(stringStat == MS::kSuccess)
			MProgressWindow::setTitle(msg);
	}

	if (exportEditsFile.length() > 0) {
		fWriter.writeExportEditsFilePresent(animFile);
	}

	if(layers)
	{
		animLayers.writeAnimLayers(animFile,fWriter);
	}

	bool haveAnyAnimatableStuff = false; //will remain false if no curves or statics
	for (unsigned int i = 0; i < numObjects; i++) 
	{
		if(hasActiveProgress)
			MProgressWindow::setProgress(i);
		MString localCommand;
		bool haveAnimatedCurves = false; //local flag, if true this node has animated curves
		bool haveAnimatableChannels = false; //local flag, if true node has some animatable statics

		MDagPath path;
		MObject node;
		if (sList.getDagPath (i, path) == MS::kSuccess) 
		{
			MString name = path.partialPathName();
			//if the name is in the template, only then write it out...
			if(templateReader.findNode(name)== false)
				continue;

			//we use this to both write out the cached plugs for this node but for also to not write out
			//the plugs which are cached when writing anim curves.
			atomCachedPlugs * cachedPlug = NULL;
			if(cached && i < cachedPlugs.size())
				cachedPlug = cachedPlugs[i];

			atomNodeWithAnimLayers  *layerPlug = NULL;
			if(layers && i < nodesWithAnimLayers.size())
				layerPlug = nodesWithAnimLayers[i];

			unsigned int depth = depths[i];
			unsigned int childCount = path.childCount();
			MObject object = path.node();
			atomNodeNameReplacer::NodeType nodeType = (object.hasFn(MFn::kShape)) ? atomNodeNameReplacer::eShape : atomNodeNameReplacer::eDag;
			fWriter.writeNodeStart(animFile,nodeType,name,depth,childCount);
			
			
			MPlugArray animatablePlugs;
			MSelectionList localList;
			localList.add(object);
			MAnimUtil::findAnimatablePlugs(localList,animatablePlugs);
			
			if(writeAnimCurves(animFile,name,cachedPlug, layerPlug, command, haveAnimatedCurves,templateReader) != MS::kSuccess )
			{
				return (MS::kFailure);
			}
			else if(haveAnimatedCurves)
			{
				haveAnyAnimatableStuff = true;
			}
			if(statics||cached)
			{
				writeStaticAndCached (animatablePlugs,cachedPlug,statics,cached,animFile,attrStrings,name,depth,childCount, haveAnimatableChannels,templateReader);
			}
			fWriter.writeNodeEnd(animFile);

		}
		else if (sList.getDependNode (i, node) == MS::kSuccess) {
			
			if (!node.hasFn (MFn::kDependencyNode)) {
				return (MS::kFailure);
			}
			MPlugArray animatablePlugs;
			MFnDependencyNode fnNode (node, &status);
			MString name = fnNode.name();
			atomNodeNameReplacer::NodeType nodeType = atomNodeNameReplacer::eDepend;
			atomNodeWithAnimLayers  *layerPlug = NULL;
			//if a layer we get our own attrs
			if(i< animLayers.length())
			{
				MPlugArray plugs;
				animLayers.getPlugs(i,animatablePlugs);
				nodeType = atomNodeNameReplacer::eAnimLayer;
			}
			else
			{
				if(templateReader.findNode(name)== false)
				{
					continue;
				}
				MSelectionList localList;
				localList.add(node);
				MAnimUtil::findAnimatablePlugs(localList,animatablePlugs);
				if(layers && i < nodesWithAnimLayers.size())
					layerPlug = nodesWithAnimLayers[i];
			}
			//we use this to both write out the cached plugs for this node but for also to not write out
			//the plugs which are cached when writing anim curves.
			atomCachedPlugs * cachedPlug = NULL;
			if(cached && i < cachedPlugs.size())
				cachedPlug = cachedPlugs[i];

			fWriter.writeNodeStart(animFile,nodeType,name);

			if(writeAnimCurves(animFile,name, cachedPlug,layerPlug,command, haveAnimatedCurves,templateReader) != MS::kSuccess )
			{
				return (MS::kFailure);
			}
			else if(haveAnimatedCurves)
			{
				haveAnyAnimatableStuff = true;
			}


			if(statics||cached)
			{
				writeStaticAndCached (animatablePlugs,cachedPlug,statics,cached,animFile,attrStrings,name,0,0,haveAnimatableChannels,templateReader);
			}
			fWriter.writeNodeEnd(animFile);
		}
		if(haveAnimatableChannels==true)
			haveAnyAnimatableStuff = true;

		if  (hasActiveProgress && MProgressWindow::isCancelled())
		{
			computationFinished = false;
			break;
		}
		
	}
	
	if (exportEditsFile.length() > 0) {
		fWriter.writeExportEditsFile(animFile,exportEditsFile);
	}
	
	//delete any cachedPlugs objects that we created.
	for(unsigned int z = 0; z< cachedPlugs.size(); ++z)
	{
		if(cachedPlugs[z])
			delete cachedPlugs[z];
	}
	//and delete any any layers too
	for(unsigned int zz = 0; zz< nodesWithAnimLayers.size(); ++zz)
	{
		if(nodesWithAnimLayers[zz])
			delete nodesWithAnimLayers[zz];
	}
	if(computationFinished == false) //failed for some reason, one reason is that the user canceled the computation
	{
		MString msg = MStringResource::getString(kSavingCanceled, status);
		MGlobal::displayError(msg);
		return (MS::kFailure);
	}

	if(hasActiveProgress)
		MProgressWindow::endProgress();

	if(haveAnyAnimatableStuff == false)
	{
		MString msg = MStringResource::getString(kAnimCurveNotFound, status);
		MGlobal::displayError(msg);
		return (MS::kFailure);
	}
	else return (MS::kSuccess);
}	
/*
MStatus				
atomExport::writeSetDrivenKeys(ofstream &animFile, MFnDependencyNode &fnNode,MString &name, bool &hasSetDrivenKey)
{
	MPlugArray plugArray;
	MStatus status = fnNode.getConnections (plugArray);
	bool animated = false;
	if (status == MS::kSuccess)
	{
		unsigned int numPlugs = plugArray.length();
		for (unsigned int i = 0; i < numPlugs; i++)
		{
			MObjectArray animationNodes;
			MPlugArray drivers;
			if(MAnimUtil::findSetDrivenKeyAnimation(plugArray[i],animationNodes,
				drivers,&status ))
			{
				for(unsigned int k=0; k < animationNodes.length();++k)
				{
					MObject animNode =animationNodes[k];
					if (animNode.hasFn (MFn::kDependencyNode))
					{
						MFnDependencyNode fnNode(animNode);
						MString name = fnNode.name();
						MString what = name + " ";
					}
					else if(animNode.hasFn (MFn::kDagNode)) 
					{
						MFnDagNode dagNode (animNode);
						MDagPath dagPath;
						if (dagNode.getPath(dagPath) == MS::kSuccess) 
						{
							MString name = dagPath.fullPathName();
							MString what = name + " ";
						}
					}
				}
			}
		}

	}
	return status;
}
*/

void
atomExport::writeStaticAndCached (MPlugArray &animatablePlugs, atomCachedPlugs *cachedPlugs,bool statics, bool cached,ofstream &animFile, std::set<std::string> &attrStrings,
					MString &name, unsigned int depth,
					unsigned int childCount,bool &hasAnimatable,atomTemplateReader &templateReader )
{

	unsigned int numPlugs = animatablePlugs.length();
	if (numPlugs != 0) {
		hasAnimatable = true;
		if(statics)
			fWriter.writeStaticValues (animFile, animatablePlugs,attrStrings, name, depth, childCount,templateReader);//for now just using partial path names, TODO decide which to really use
		if(cached && cachedPlugs != NULL)
			fWriter.writeCachedValues (animFile, cachedPlugs,attrStrings, name, depth, childCount,templateReader);//for now just using partial path names, TODO decide which to really use

	}
}


bool
atomExport::setUpCache(MSelectionList &sList, std::vector<atomCachedPlugs *> &cachedPlugs,atomAnimLayers &animLayers,
						bool sdk, bool constraint, bool layers,
						std::set<std::string> &attrStrings, atomTemplateReader &templateReader,
						MTime &startTime, MTime &endTime, MAngle::Unit angularUnit,
						MDistance::Unit	linearUnit)
{
	if(endTime<startTime)
		return false; //should never happen but just in case.
	unsigned int numObjects = sList.length();
	cachedPlugs.resize(numObjects);

	double dStart = startTime.value();
	double dEnd = endTime.value() +  (.0000001); //little nudge in case of round off errors
	MTime::Unit unit = startTime.unit();
	double tickStep = MTime(1.0,unit).value();
	unsigned int numItems = ((unsigned int)((dEnd - dStart)/tickStep)) + 1;
	bool somethingIsCached = false; //if nothing get's cached no reason to run computation loop
	for (unsigned int i = 0; i < numObjects; i++) 
	{
		atomCachedPlugs *plug = NULL;
		//make sure it's a NULL, and preset it in case we skip this node
		cachedPlugs[i] = plug;

		MDagPath path;
		MObject node;
		MString name;
		if (sList.getDagPath (i, path) == MS::kSuccess) 
		{
			node = path.node();
			name = path.partialPathName();
		}
		else if (sList.getDependNode (i, node) == MS::kSuccess) {
			
			if (!node.hasFn (MFn::kDependencyNode)) {
				continue;
			}
			MFnDependencyNode fnNode (node);
			name = fnNode.name();
		}
		if(node.isNull()==false)
		{
			if(i< animLayers.length())
			{
				MPlugArray plugs;
				animLayers.getPlugs(i,plugs);
				std::set<std::string> tempAttrStrings;
				atomTemplateReader tempTemplateReader;
				plug = new atomCachedPlugs(name,node,plugs,sdk,constraint,layers,
					tempAttrStrings,tempTemplateReader,numItems,angularUnit,
				linearUnit);
				if(plug->hasCached() ==false)
					delete plug;
				else
				{
					cachedPlugs[i] = plug;
					somethingIsCached = true;
				}
			}
			else
			{
				if(templateReader.findNode(name)== false)
				{
					continue;
				}
				MSelectionList localList;
				localList.add(node);
				MPlugArray animatablePlugs;
				MAnimUtil::findAnimatablePlugs(localList,animatablePlugs);
				plug = new atomCachedPlugs(name,node,animatablePlugs,sdk,constraint,layers,attrStrings,templateReader,numItems,angularUnit,
					linearUnit);
				if(plug->hasCached() ==false)
					delete plug;
				else
				{
					cachedPlugs[i] = plug;
					somethingIsCached = true;
				}
			}
		}
	}
	
	bool computationFinished = true; //if no interrupt happens we will finish the computation
	if(somethingIsCached)
	{
		bool hasActiveProgress = false;
		if (MProgressWindow::reserve()) {
			hasActiveProgress = true;
			MProgressWindow::setInterruptable(true);
   			MProgressWindow::startProgress();
    		MProgressWindow::setProgressRange(0, numObjects);
			MProgressWindow::setProgress(0);
			MStatus stringStat;
			MString msg = MStringResource::getString(kBakingProgress, stringStat);
			if(stringStat == MS::kSuccess)
				MProgressWindow::setTitle(msg);
		}

		unsigned int count =0;
		for(double tick = dStart; tick <= dEnd; tick += tickStep)
		{
			if(hasActiveProgress)
				MProgressWindow::setProgress(count);
			MTime time(tick,unit);
			MDGContext ctx(time);
			for(unsigned int z = 0; z< cachedPlugs.size(); ++z)
			{
				if(cachedPlugs[z])
					cachedPlugs[z]->calculateValue(ctx,count);
			}

			if  (hasActiveProgress && MProgressWindow::isCancelled())
			{
				computationFinished = false;
				break;
			}
			++count;
		}
		if(hasActiveProgress)
			MProgressWindow::endProgress();
	}
	return computationFinished;

}



//new anim layer stuff prototype

bool
atomExport::setUpAnimLayers(MSelectionList &sList,atomAnimLayers &animLayers, std::vector<atomNodeWithAnimLayers *> &nodesWithAnimLayers,
						std::set<std::string> &attrStrings, atomTemplateReader &templateReader)
					
{
	unsigned int numObjects = sList.length();
	nodesWithAnimLayers.resize(numObjects);

	bool somethingIsAnimLayered = false; 
	for (unsigned int i = 0; i < numObjects; i++) 
	{
		atomNodeWithAnimLayers *nodeWithLayer = NULL;
		//make sure it's a NULL, and preset it in case we skip this node
		nodesWithAnimLayers[i] = nodeWithLayer;

		MDagPath path;
		MObject node;
		if (sList.getDagPath (i, path) == MS::kSuccess) 
		{
			MString name = path.partialPathName();
			//if the name is in the template, only then write it out...
			if(templateReader.findNode(name)== false)
			{
				continue;
			}
			node = path.node();
		}
		else if (sList.getDependNode (i, node) == MS::kSuccess) {
			
			if (!node.hasFn (MFn::kDependencyNode)) {
				continue;
			}
			MFnDependencyNode fnNode (node);
			MString name = fnNode.name();
			if(templateReader.findNode(name)== false)
			{
				continue;
			}
		}
		if(node.isNull()==false)
		{
			MSelectionList localList;
			localList.add(node);
			MPlugArray animatablePlugs;
			MAnimUtil::findAnimatablePlugs(localList,animatablePlugs);
			unsigned int numPlugs = animatablePlugs.length();
			MPlugArray cachedPlugs;
			for (unsigned int k = 0; k < numPlugs; k++)
			{
				MPlug plug = animatablePlugs[k];
				MObjectArray layers;
				MPlugArray	 plugs;
				if(MAnimUtil::findAnimationLayers(plug,layers,plugs) && layers.length() > 0)
				{
					bool layerAdded = animLayers.addAnimLayers(layers);
					if(layerAdded)
					{
						if(nodeWithLayer == NULL)
							nodeWithLayer = new atomNodeWithAnimLayers();
						nodeWithLayer->addPlugWithLayer(plug,layers,plugs);
					}
					somethingIsAnimLayered = somethingIsAnimLayered == false ? layerAdded : true;
				}
			}
			nodesWithAnimLayers[i] = nodeWithLayer;

		}
	}
	return somethingIsAnimLayered;
}

MStatus initializePlugin(MObject obj)
{
	MStatus stat = MS::kFailure;
	MFnPlugin plugIn(obj, PLUGIN_COMPANY, "1.0", "Any");
	
	// This would be done first, so the strings are available. But we have no UI yet
	//TODO
/*	stat = impPlugIn.registerUIStrings(registerMStringResources, "atomImportExportInitStrings");
	if (stat != MS::kSuccess)
	{
		stat.perror("registerUIStrings");
		return stat;
	}
	*/
	stat = plugIn.registerFileTranslator("atomImport", "none",
											atomImport::creator,
											(char *)NULL,
											(char *)animImportDefaultOptions, 
											true);

	if (stat != MS::kSuccess) {
		return stat;
	}

	stat = plugIn.registerFileTranslator("atomExport", "",
										atomExport::creator,
										(char *)NULL,
										(char *)animExportDefaultOptions,
										true);


	MGlobal::sourceFile ( "atomLayerCommands.mel" ) ;

	return stat;
}

MStatus uninitializePlugin(MObject obj)
{
	MStatus stat = MS::kFailure;

	MFnPlugin plugIn(obj);
	stat = plugIn.deregisterFileTranslator("atomImport");

	if (stat != MS::kSuccess) {
		return stat;
	}

	stat = plugIn.deregisterFileTranslator("atomExport");

	return stat;
}











