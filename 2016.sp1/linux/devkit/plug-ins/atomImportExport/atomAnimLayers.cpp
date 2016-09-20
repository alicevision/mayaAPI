/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomAnimLayers.cpp
//
//
//		Information on animation layers, used by import and export.
#include <maya/MFnAttribute.h>
#include <maya/MAnimCurveClipboard.h>
#include <maya/MGlobal.h>

#include <string.h>

#include "atomAnimLayers.h"
#include "atomImportExportStrings.h"

const char *kAnimLayers = "animLayers";


//return true if we have animLayers in the scene
bool atomAnimLayers::getOrderedAnimLayers()
{
	MStatus status = MS::kFailure;
	MString melCommand = MString("atomGetAllLayersOrdered();");
	mOrderedAnimLayerNames.setLength(0);
	status = MGlobal::executeCommand(melCommand, mOrderedAnimLayerNames, false, false );
	return (status==MS::kSuccess && mOrderedAnimLayerNames.length() >0);
}
//given a specified layerName and the name of the previous layer it follows,
//create the layer
void atomAnimLayers::createAnimLayer(const MString &layerName, const MString &prevLayerName)
{
	MStatus status = MS::kFailure;
	if(prevLayerName.length() !=0)
	{
		//move the created layer to after the previous
		MString melCommand = MString("animLayer -mva ") +  MString("\"") + prevLayerName +  MString("\"") + MString(" ") +  MString("\"")+ 
			layerName +  MString("\"")+ MString(";");
		status = MGlobal::executeCommand(melCommand, false, false );
	}
	else
	{
		if(layerName == MString("BaseAnimation")) //todo need a better way to handle the root layer
		{
			MString melCommand("animLayer;");
			status = MGlobal::executeCommand(melCommand, false, false );
		}
		else //not the root so just create it, it will go at the end which is fine since we don't know where it shoudl go in the list.
		{
			MString melCommand = MString("animLayer ") + MString("\"") + layerName + MString("\"") + MString(";");
			status = MGlobal::executeCommand(melCommand, false, false );

		}
	}
}

//check and if the specified attrName on the specified nodeName exists on the specified layerName
bool atomAnimLayers::isAttrInAnimLayer(const MString &nodeName, const MString &attrName, const MString &layerName)
{
	MStatus status = MS::kFailure;
	MStringArray resultArray;
	MString melCommand = MString("animLayer -query -attribute ") + MString("\"") + layerName + MString("\"");
	status = MGlobal::executeCommand(melCommand, resultArray, false, false );
	MString name = nodeName + MString(".") + attrName;
	for(unsigned int k = 0; k < resultArray.length(); ++k)
	{
		if(name == resultArray[k])
			return true;
	}
	return false;
}

//add the specified nodename.attrName to the specified layer
bool atomAnimLayers::addAttrToAnimLayer(const MString &nodeName, const MString &attrName, const MString &layerName)
{
	MStatus status = MS::kFailure;
	MString name = nodeName + MString(".") + attrName;
	MString melCommand = MString("animLayer -edit -attribute ") + MString("\"") + name + MString("\"") + MString(" ")
		+ MString("\"") + layerName + MString("\"");
	status = MGlobal::executeCommand(melCommand, false, false );
	return (status==MS::kSuccess);
}

//remove this attribute from all layers
void atomAnimLayers::removeLayersIfNeeded(bool replaceLayers, const MString &nodeName, const MString &attrName)
{
	//if we are replacing and we have layers in the scene
	if(replaceLayers == true && mOrderedAnimLayerNames.length() > 0)
	{
		MString fullName = nodeName + MString(".") + attrName;
		std::string str(fullName.asChar());
		if(mAttrsRemovedFromAnimLayers.find(std::string(str)) == mAttrsRemovedFromAnimLayers.end())
		{
			mAttrsRemovedFromAnimLayers.insert(str); //don't remove it again
			//mOrderedAnimLayers was caclculated when we read in the animLayer names.
			for(unsigned int z = 0; z < mOrderedAnimLayerNames.length(); ++z)
			{
				MString layerName = mOrderedAnimLayerNames[z];
				if(isAttrInAnimLayer(nodeName, attrName, layerName)) //if the attribute is in this layer remove it.
				{
					MString melCommand = MString("animLayer -edit -removeAttribute ") +  MString("\"") + fullName +  MString("\"") + MString(" ") +  MString("\"")+ 
					layerName +  MString("\"")+ MString(";");
					MStatus status = MGlobal::executeCommand(melCommand, false, false );	
					if(status == MS::kSuccess)
					{
						//removed that attribute so save the layer name out. later after loading we will delete these layers that have
						//no attributes left in them.
						std::string layerStr(layerName.asChar());
						if(mLayersWithRemovedAttrs.find(std::string(layerStr)) == mLayersWithRemovedAttrs.end())
							mLayersWithRemovedAttrs.insert(layerStr);
					}
				}
			}
		}
	}
}

void atomAnimLayers::deleteEmptyLayers(bool replaceLayers)
{
	if(replaceLayers == true && mLayersWithRemovedAttrs.size() > 0)
	{
		for(std::set<std::string>::iterator iter = mLayersWithRemovedAttrs.begin(); iter !=
					mLayersWithRemovedAttrs.end(); ++iter)
		{
			std::string str = *iter;
			MString layerName(str.c_str());
			MStringArray resultArray;
			MString melCommand = MString("animLayer -query -attribute ") + MString("\"") + layerName + MString("\"");
			MGlobal::executeCommand(melCommand, resultArray, false, false );
			if(resultArray.length() ==0) //it has no attributes so delete it..
			{
				melCommand = MString("delete ") + MString("\"") + layerName + MString("\"");
				MGlobal::executeCommand(melCommand,false,false);
			}
		}
	}
}

//this function creates missing animation layers and places it in the correct order
void atomAnimLayers::createMissingAnimLayers(const MStringArray &animLayers)
{
	for(unsigned int k=0; k < animLayers.length(); ++k)
	{
		MSelectionList list;
		list.add(animLayers[k]);
		if(list.length()!=1) //if not in the list then not in the scene so create it, and the previous one WILL be in the scene so attach to it
		{
			MString prevLayerName;
			if(k>0)
				prevLayerName = animLayers[k-1];
			//todo need a better way to handle the root layer
			//this fixes an issue where when you create an animation layer for the base it creates 2 layers the BaseAnimation
			//and a default animLayer1, where if you create the same thing in the UI the second one is named AnimLayer1, with
			//a cap A. So we are getting an extra animLayer1 created, which is a little messy. This stops that
			//but we need to revisit.  Also seems like you can't rename BaseAnimation, but not sure what it is in different
			//languages. It's a node so maybe it never changes?

			if(k > 0 || animLayers[k] != MString("BaseAnimation") || animLayers.length()==0)  //we know we are creating more than 1 so the BaseAnimation will
			{																  //be created next time so dont create it unless only creating a BaseAnimation
				createAnimLayer(animLayers[k],prevLayerName);
			}
		}
	}
}

//find all animation layers in the scene and add them to the selection list
void atomAnimLayers::addAnimLayersToSelection()
{
	getOrderedAnimLayers();
	for(unsigned int z = 0; z < mOrderedAnimLayerNames.length();++z)
	{
		MGlobal::selectByName( mOrderedAnimLayerNames[z], MGlobal::kAddToList );
	}
}

//this function will pass in all anim layer objects found on a particular plug. since we want them to be
//kept in order, but don't want ALL of the anim layers to be specified if they aren't used we
bool atomAnimLayers::addAnimLayers(MObjectArray &layers)
{
	if(mOrderedAnimLayerNames.length() > 0)
	{
		//first time called set up the mAnimLayers object array.
		if(mAnimLayers.length() != mOrderedAnimLayerNames.length())
		{
			mAnimLayers.setLength(mOrderedAnimLayerNames.length());
			MObject nullObject;
			//initialize with nulls
			for(unsigned int k = 0; k < mAnimLayers.length(); ++k)
			{
				mAnimLayers[k] = nullObject;
			}
		}
	}
	//if here we have the ordered name list and the anim layer object list 
	//now we just need to set the objects passed in correctly
	for(unsigned int k = 0; k < layers.length(); ++k)
	{
		if(layers[k].hasFn (MFn::kDependencyNode))
		{
			MFnDependencyNode fnNode (layers[k]);
			MString layerName = fnNode.name();
			for(unsigned int z = 0; z < mOrderedAnimLayerNames.length();++z)
			{
				if(layerName == mOrderedAnimLayerNames[z])
				{
					mAnimLayers[z] = layers[k];
					break;
				}
			}
		}
	}
	
	return true;
}

//this function adds the layer objects in mAnimLayers to the start of the selection list.
//this is used when exporting to make sure the animationlayers come first
void atomAnimLayers::addLayersToStartOfSelectionList(MSelectionList &list)
{
	if(mAnimLayers.length() > 0 )
	{
		MSelectionList layers;
		for(unsigned int i =0; i<mAnimLayers.length();++i)
		{
			layers.add(mAnimLayers[i], true);
		}
		layers.merge(list);
		list = layers;
		
	}

}

//for the nthArray that we hold(that corresponds to the nth item in the export list),
//get the animation layer plugs for it's attributes, e.g. weight, mute
void atomAnimLayers::getPlugs(unsigned int nth, MPlugArray &plugs)
{
	if(nth < mAnimLayers.length() && mAnimLayers[nth].hasFn (MFn::kDependencyNode))
	{
		MFnDependencyNode fnNode (mAnimLayers[nth]);
		MStringArray attributes;
		getRelevantAttributes(attributes);
		collectAnimLayerPlugs(fnNode,attributes,plugs);
	}
}

//get attribute names for an animationlayer, like mute,weight, solo
void atomAnimLayers::getRelevantAttributes(MStringArray &attributes)
{
	MString attr("mute");
	attributes.append(attr);
	attr = MString("lock");
	attributes.append(attr);
	attr = MString("solo");
	attributes.append(attr);
	attr = MString("override");
	attributes.append(attr);
	attr = MString("passthrough");
	attributes.append(attr);
	attr = MString("preferred");
	attributes.append(attr);
	attr = MString("weight");
	attributes.append(attr);
	attr = MString("rotationAccumulationMode");
	attributes.append(attr);
	attr = MString("scaleAccumulationMode");
	attributes.append(attr);
}

//get the attribute plugs for a specific layer
void atomAnimLayers::collectAnimLayerPlugs(MFnDependencyNode &layer, MStringArray &attributes, MPlugArray &plugs)
{
	MStatus status = MS::kSuccess;
	for(unsigned int i = 0; i < attributes.length(); ++i)
	{
		MPlug plug = layer.findPlug(attributes[i],false, &status);
		if(status== MS::kSuccess)
		{
			plugs.append(plug);
		}
	}

}


//for each animation layer we have, write out the name of the layer
//we will later write out the layer like a depend node and write out
//each attribute, but we do this seperately so that on import we
//can quickly see what layers are in the ATOM file so that we can
//create them if they are missing
void atomAnimLayers::writeAnimLayers(ofstream &animFile, atomWriter &writer)
{

	if(mAnimLayers.length() > 0) // at least one Layer
	{
		animFile <<kAnimLayers << " { ";
		for(unsigned int k = 0; k < mAnimLayers.length(); ++k)
		{
				if(mAnimLayers[k].hasFn (MFn::kDependencyNode))
				{
					MFnDependencyNode fnNode (mAnimLayers[k]);
					MString layerName = fnNode.name();
					animFile <<"  " << layerName;
				}
			}
		animFile << " }\n";
	}
}

//read out the animation layers from the animReader
//then create any missing layers and add them to the active selection
bool atomAnimLayers::readAnimLayers(ifstream &readAnim, char *dataType,atomReader &reader)
{

	if (strcmp(dataType, kAnimLayers) == 0)
	{
		dataType = reader.asWord(readAnim);
		if (strcmp(dataType, "{") == 0)
		{
			MStringArray layerNames;
			while((dataType = reader.asWord(readAnim)))
			{
				if(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
				{
					MString name(dataType);
					layerNames.append(name);
				}
				else
					break;
			}
			if(layerNames.length() >0)
			{
				createMissingAnimLayers(layerNames);
				addAnimLayersToSelection();
			}
		}
		return true;
	}
	return false;
}
//
//
///atomNodeWithAnimLayers
//
//

//for this attribute on this node add the following layer nodes and plugs it's associated with
void atomNodeWithAnimLayers::addPlugWithLayer(MPlug &attrPlug, MObjectArray &layers, MPlugArray &plugs)
{
	if(plugs.length() == layers.length())
	{
		MObject attrObj = attrPlug.attribute();
		MFnAttribute fnLeafAttr (attrObj);
		std::string attrStd(std::string(fnLeafAttr.name().asChar()));
		PlugsAndLayers plugsAndLayers;
		for(unsigned int i =0; i < layers.length(); ++i)
		{
			if(plugs[i].isNull()==false) //it's possible to not have a plug for the specified layer
			{
				if(layers[i].hasFn (MFn::kDependencyNode))
				{
					MFnDependencyNode fnNode (layers[i]);
					MString layerName = fnNode.name();
					plugsAndLayers.mLayerNames.append(layerName);
					plugsAndLayers.mPlugs.append(plugs[i]);
					fAttrLayers[attrStd] = plugsAndLayers;
				}
			}
		}
	}
}

//is the specified plug on this node animation layered
//used when exporting
bool atomNodeWithAnimLayers::isPlugLayered(const MString &plugName, MStringArray &layerNames)
{
	AttrLayersMap::iterator p;
	std::string stdAttrName(plugName.asChar());
	p = fAttrLayers.find(stdAttrName);
	if(p != fAttrLayers.end())
	{
		PlugsAndLayers val = p->second;
		layerNames = val.mLayerNames;
		return true;
	}
	return false;
}


//
//
//	atomLayerClipboard
//
//

//get the clipboarditemarray for the specified layername
//if the layer string is empty we return the default clipboard
MAnimCurveClipboardItemArray& atomLayerClipboard::getCBItemArray(const MString &layerName)
{
	if(layerName.length() ==0)
		return fEmptyLayerArray;
	//here the layername is something
	std::string name(layerName.asChar());
	ArrayMap::iterator p;
	p = fArray.find(name);
	if(p != fArray.end())
	{
		return *(p->second);
	}
	MAnimCurveClipboardItemArray *item = new MAnimCurveClipboardItemArray();
	fArray[name] = item;
	return *item;
}

atomLayerClipboard::~atomLayerClipboard()
{
	for(ArrayMap::iterator p = fArray.begin(); p!= fArray.end(); ++p)
	{
		if(p->second)
			delete p->second;
	}
}

//paste the specified clipboard, with the specified attributes
//if the animLayerName is empty we won't use the animLayer flag
//when pasting.
bool atomLayerClipboard::pasteClipboard(MAnimCurveClipboardItemArray &itemArray,
		const MTime& startTime, const MTime& endTime,
		float startUnitless, float endUnitless,const MString &pasteFlags,
		MString &animLayerName)
{
	bool good = false;
	MStatus status = MS::kFailure;
	MAnimCurveClipboard::theAPIClipboard().clear();
	if (MS::kSuccess == MAnimCurveClipboard::theAPIClipboard().set(	itemArray, 
						startTime,endTime,startUnitless, endUnitless,false))
	{
		good = true;
	}


	if (MAnimCurveClipboard::theAPIClipboard().isEmpty()== false)
	{
		MString command("pasteKey -cb api ");
		//we always match by name since we do use our own local algorithm
		//to match up hierarchies. So not matter what option we use
		//the name of the items in the clipboard will match up as we expect them too.
		command += " -mn true ";
		if(animLayerName.length() >0)
		{
			MString onThisLayer = MString(" -al ") + animLayerName + MString(" ");
			command += onThisLayer;
		}
		command += pasteFlags;

		int result;
		if (MS::kSuccess != (status =  
			MGlobal::executeCommand(command, result, false, true)))
		{

			MString msg = MStringResource::getString(kPasteFailed, status);
			MGlobal::displayError(msg);
			return false;
		}
	}
	return good;
}

//paste the keys we have in each stored clipboard using the specified parameters.
//will paste the default layer and each animlayer
MStatus atomLayerClipboard::pasteKeys(const MTime& startTime, const MTime& endTime,
		float startUnitless, float endUnitless,const MString &pasteFlags)
{
	MString animLayerName; //empty first time for the first default empty layer
	bool oneWasGood = pasteClipboard(fEmptyLayerArray, startTime,endTime,
									startUnitless, endUnitless,pasteFlags,animLayerName);
	//now go through and do each one that's in an animation layer
	for(ArrayMap::iterator p = fArray.begin(); p!= fArray.end(); ++p)
	{
		if(p->second) //double check but should not be NULL
		{
			animLayerName= MString(p->first.c_str());
			bool good = pasteClipboard(*(p->second), startTime,endTime,
									startUnitless, endUnitless,pasteFlags,animLayerName);
			if(good == true)
				oneWasGood = true;
		}
	}

	//if not one was good then something failed.
	if(oneWasGood==false)
	{
		MStatus stringStat;
		MString msg = MStringResource::getString(kClipboardFailure,
												 stringStat);
		MGlobal::displayError(msg);
	}
	return (MS::kSuccess);
}


//does this specified node have any layers, and if so return them
//in a std::set of std::strings
bool atomNodeWithAnimLayers::isNodeLayered(std::set<std::string> &layerNames)
{
	AttrLayersMap::iterator p;
	bool isLayered = false;
	for(p = fAttrLayers.begin();p!= fAttrLayers.end(); ++p)
	{
		PlugsAndLayers val = p->second;
		for(unsigned int i =0 ;i< val.mLayerNames.length(); ++i)
		{
			if(val.mLayerNames[i].length() > 0)
			{
				std::string str(val.mLayerNames[i].asChar());
				layerNames.insert(str);
				isLayered = true;
			}
		}
	}
	return isLayered;
}






