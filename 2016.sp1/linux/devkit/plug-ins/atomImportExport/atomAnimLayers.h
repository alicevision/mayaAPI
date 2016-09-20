/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomAnimLayers.h
//
//
//		Information on animation layers, used by import and export.
#ifndef __ATOM_ANIM_LAYERS_H
#define __ATOM_ANIM_LAYERS_H
 
#include <set>
#include <string>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MString.h>
#include <maya/MTime.h>
#include <maya/MStringArray.h>
#include <maya/MObjectArray.h>
#include <maya/MSelectionList.h>
#include <maya/MAnimCurveClipboardItemArray.h>
#include <maya/MStatus.h>
#include "atomFileUtils.h"

//
//
// class atomAnimLayers
// This class contains the logic to read and write out animation layers
// both at the anim layer node layer and to query if particular attributes
// belong to which layer. 
class atomAnimLayers
{

public:

	atomAnimLayers(){}
	~atomAnimLayers(){}

	bool addAnimLayers(MObjectArray &layers);
	
	void writeAnimLayers(ofstream &animFile, atomWriter &writer);
	bool readAnimLayers(ifstream &readAnim, char *dataType,atomReader &reader);

	void addLayersToStartOfSelectionList(MSelectionList &list);
	unsigned int length(){return mAnimLayers.length();}
	void getPlugs(unsigned int i, MPlugArray &plugs);

	bool getOrderedAnimLayers();
	void createMissingAnimLayers(const MStringArray &animLayers);
	void createAnimLayer(const MString &layerName, const MString &prevLayerName);
	void addAnimLayersToSelection();
	void removeLayersIfNeeded(bool replaceLayers, const MString &nodeName, const MString &leafAttributeName);
	void deleteEmptyLayers(bool replaceLayers);
	//static functions
	static bool isAttrInAnimLayer(const MString &nodeName, const MString &attrName, const MString &layerName);
	static bool addAttrToAnimLayer(const MString &nodeName, const MString &attrName, const MString &layerName);
 
private:
	MObjectArray mAnimLayers;
	MStringArray mOrderedAnimLayerNames;

	//used by removeLayersIfNeeded. Keep track of each layer attribute that we have removed from all layers
	//so that we only do it once
	std::set<std::string> mAttrsRemovedFromAnimLayers;
	//also when removing attributes from layers we also want to delete any animation layers that we removed
	//attributes from if they end up empty with no attributes attached to them.
	std::set<std::string> mLayersWithRemovedAttrs;
	void getRelevantAttributes(MStringArray &attributes);
	void collectAnimLayerPlugs(MFnDependencyNode &layer, MStringArray &attributes, MPlugArray &plugs);
};

//
// 
//	class atomNodeWithAnimLayers
//	This class should be allocated once for item in the depend list.
//  It will contain information as to what attributes/plugs have animation layers
//  and what layers they belong too.
class atomNodeWithAnimLayers
{

public:
	atomNodeWithAnimLayers(){};
	~atomNodeWithAnimLayers(){fAttrLayers.clear();}

	void addPlugWithLayer(MPlug &plug, MObjectArray &layers, MPlugArray &plugs);
	bool isPlugLayered(const MString &plugName, MStringArray &layerNames);
	bool isNodeLayered(std::set<std::string> &layerNames);

private:
	struct PlugsAndLayers
	{
		~PlugsAndLayers() {} //should delete the array's
		MPlugArray mPlugs;
		MStringArray mLayerNames;
		
	};
	typedef std::map< std::string, PlugsAndLayers  > AttrLayersMap;
	AttrLayersMap fAttrLayers;

};


//
// class atomLayerClipboard
//this class holds a list of clipboarditem arrays
//one for each layer that's present in the atom file
// in addition to a default clipboard for when no animtion layer is specified
// 
class atomLayerClipboard
{
public:
	atomLayerClipboard(){};
	~atomLayerClipboard();
	MAnimCurveClipboardItemArray& getCBItemArray(const MString &layerName);
	MStatus pasteKeys(const MTime& startTime, const MTime& endTime,
		float startUnitless, float endUnitless, const MString &pasteFlags);
private:

	bool pasteClipboard(MAnimCurveClipboardItemArray &itemArray,
		const MTime& startTime, const MTime& endTime,
		float startUnitless, float endUnitless,const MString &pasteFlags,
		MString &animLayerName);

	typedef std::map< std::string, MAnimCurveClipboardItemArray*  > ArrayMap;
	ArrayMap fArray;
	MAnimCurveClipboardItemArray  fEmptyLayerArray;
};


#endif

