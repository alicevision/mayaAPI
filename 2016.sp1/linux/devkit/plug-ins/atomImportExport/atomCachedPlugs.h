/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomCachedPlugs.h
//
//
//		When exporting we need to collect all items that we want to cache
//      This holds cached data only
#ifndef __ATOM_CACHED_PLUGS_H
#define __ATOM_CACHED_PLUGS_H
 
#include <vector>
#include <set>
#include <string>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MString.h>
#include <maya/MTime.h>
#include "atomFileUtils.h"

template <class T>
class atomCachedValues
{
public:
	atomCachedValues():mNumItems(0), mStride(1){};
	void resize(unsigned int numItems, unsigned int stride = 1);
	void setValue(T &value,unsigned int item, unsigned int element = 0);
	T getValue(unsigned int item, unsigned int element = 0);
	unsigned int numItems() {return mNumItems;}
	unsigned int stride() {return mStride;}
private:
	std::vector<T> mValues;
	unsigned int mStride;
	unsigned int mNumItems;
};

class atomBasePlugAndValues
{

public:

	virtual ~atomBasePlugAndValues(){};

	virtual void setValue(MDGContext &context,unsigned int index) = 0;
	virtual void writeToAtomFile(ofstream & clip) = 0;
	MPlug& getPlug() {return mPlug;}
protected:
	atomBasePlugAndValues(MPlug &plug):mPlug(plug){}; //virtual class

	MPlug mPlug;


};

class atomShortValues : public atomBasePlugAndValues
{
public:
	atomShortValues(MPlug &plug, unsigned int numItems);
	void setValue(MDGContext &context,unsigned int index);
	void writeToAtomFile(ofstream & clip);
private:
	atomCachedValues<short> mCachedValues;
};

class atomIntValues : public atomBasePlugAndValues
{
public:
	atomIntValues(MPlug &plug, unsigned int numItems);
	void setValue(MDGContext &context,unsigned int index);
	void writeToAtomFile(ofstream & clip);
private:
	atomCachedValues<int> mCachedValues;
};

class atomFloatValues : public atomBasePlugAndValues
{
public:
	atomFloatValues(MPlug &plug, unsigned int numItems,unsigned int stride = 1);
	void setValue(MDGContext &context,unsigned int index);
	void writeToAtomFile(ofstream & clip);
private:
	atomCachedValues<float> mCachedValues;
};

class atomDoubleValues : public atomBasePlugAndValues
{
public:
	atomDoubleValues(MPlug &plug, unsigned int numItems,double scale = 1.0);
	void setValue(MDGContext &context,unsigned int index);
	void writeToAtomFile(ofstream & clip);

private:
	atomCachedValues<double> mCachedValues;
	double mScale;
};



class atomCachedPlugs
{
public:
	atomCachedPlugs(MString &nodeName,MObject &object, const MPlugArray &animatablePlugs,
		bool sdk, bool constraint, bool animLayers, std::set<std::string> &attrStrings,
		atomTemplateReader &templateReader,unsigned int numItems,MAngle::Unit angularUnit,
		MDistance::Unit	linearUnit);
	~atomCachedPlugs();

	bool hasCached(){return (mCachedPlugs.size() >0);}
	unsigned int getNumPlugs(){ return (unsigned int)mCachedPlugs.size();}
	MPlug& getPlug(unsigned int item);	
	void calculateValue(MDGContext &ctx, unsigned int item);
	void writeValues(ofstream &clip, unsigned int item);
	bool isAttrCached(const MString &attrName, const MString &layerName);
private:
	std::vector<atomBasePlugAndValues *> mCachedPlugs;
	void getCachedPlugs(MString &nodeName,const MPlugArray &animatablePlugs,
		bool sdk, bool constraint, bool animLayers,
		std::set<std::string> &attrStrings,atomTemplateReader &templateReader,
		unsigned int numItems,MAngle::Unit angularUnit,
		MDistance::Unit	linearUnit);

};

#endif

