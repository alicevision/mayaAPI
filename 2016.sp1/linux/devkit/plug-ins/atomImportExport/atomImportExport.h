/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomImportExport.h
//
//
//		Import and exports .atom files.
//

#ifndef _atomImport
#define _atomImport

#include <map>
#include <maya/MPxFileTranslator.h>
#include <maya/MSelectionList.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MTime.h>
#include <maya/MAnimCurveClipboardItemArray.h>
#include "atomFileUtils.h"
#include "atomCachedPlugs.h"
#include "atomAnimLayers.h"

class MArgList;
class atomNodeNameReplacer;

class atomEditsHelper {
public:
	atomEditsHelper(atomNodeNameReplacer* replacer, atomTemplateReader* templateReader): fReplacer(replacer), fTemplateReader(templateReader) {}
	virtual ~atomEditsHelper() {}

	atomNodeNameReplacer* 	fReplacer;
	atomTemplateReader* 	fTemplateReader;
};




// The atomImport command object
//
class atomImport : public MPxFileTranslator {
public:
	atomImport ();
	virtual ~atomImport (); 

	MStatus				doIt (const MArgList &argList);
	static void *		creator ();

	virtual MStatus		reader(	const MFileObject& file,
						const MString& optionsString,
						FileAccessMode mode);

	virtual bool		haveReadMethod() const;
	virtual bool		haveWriteMethod() const;
	virtual bool		canBeOpened() const;
	virtual MString 	defaultExtension() const;
	virtual MFileKind	identifyFile(	const MFileObject& fileName,
										const char* buffer,
										short size) const;

    static void 		connectionFailedCallback(MPlug& srcPlug,
												 MPlug& dstPlug,
												 const MString& srcName,
												 const MString& dstName,
												 void* clientData);

private:
	MStatus				importAnim(MSelectionList &list,ifstream&, const MString&, atomNodeNameReplacer &, MString&,
									atomTemplateReader &, bool replaceLayers);
	static bool 		replaceNameAndFindPlug(const MString&,
											   atomNodeNameReplacer&,
											   MPlug&);
	static bool 		checkPlugAgainstTemplate(const MString&,
												 const MPlug&,
												 atomTemplateReader*);

private:

	atomReader			fReader;
	

};

class MArgList;
class atomWriter;

// The atomImport command object
//
class atomExport : public MPxFileTranslator {
public:
	atomExport ();
	virtual ~atomExport (); 

	static void *		creator ();

	virtual MStatus 	writer(	const MFileObject& file,
						const MString& optionsString,
						FileAccessMode mode );

	virtual bool		haveReadMethod() const;
	virtual bool		haveWriteMethod() const;
	virtual MString 	defaultExtension() const;
	virtual MFileKind	identifyFile(	const MFileObject& fileName,
										const char* buffer,
										short size) const;
private:
	MStatus				exportSelected(	ofstream&, MString &,
										std::set<std::string> &attrStrings,bool includeChildren, 
										bool useSpecifiedTimes, MTime &startTime,
										MTime &endTime,
										bool statics, bool cached, bool sdk, bool constraint, bool layers,
										const MString& exportEditsFile,
										atomTemplateReader &reader);


	void				writeStaticAndCached (MPlugArray &animtablePlugs, atomCachedPlugs *cachedPlugs,bool statics, bool cached,ofstream &animFile, 
									std::set<std::string> &attrStrings,
									MString &name, unsigned int depth,
									unsigned int childCount,bool &hasAnimatable,
									atomTemplateReader &reader);

	MStatus				writeAnimCurves(ofstream &animFile,MString &nodeName,atomCachedPlugs *cachedPlugs,atomNodeWithAnimLayers *layerPlugs,
									MString &command, bool &haveAnimatedCurves,	atomTemplateReader &reader);

	bool				setUpCache(MSelectionList &sList,  std::vector<atomCachedPlugs *> &cachedPlugs,atomAnimLayers &animLayers,
									bool sdk, bool constraint, bool layers,
									std::set<std::string> &attrStrings, atomTemplateReader &templateReader,
									MTime &startTime, MTime &endTime,MAngle::Unit angularUnit,
									MDistance::Unit	linearUnit);

	bool				setUpAnimLayers(MSelectionList &sList,atomAnimLayers &animLayers, 
									std::vector<atomNodeWithAnimLayers *> &nodesWithAnimLayers,
									std::set<std::string> &attrStrings, atomTemplateReader &templateReader);


	//MStatus				writeSetDrivenKeys(ofstream &animFile, MFnDependencyNode &fnNode,MString &name, bool &hasSetDrivenKey);
	atomWriter			fWriter;
};
#endif

