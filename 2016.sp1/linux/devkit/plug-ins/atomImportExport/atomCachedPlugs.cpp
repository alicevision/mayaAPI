/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomCachedPlugs.cpp
//
//
//		When exporting we need to collect all items that we want to cache
//      This holds cached only.

#include "atomCachedPlugs.h"
#include <maya/MAnimUtil.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>

template <class T>
void atomCachedValues<T>::resize(unsigned int numItems,unsigned int stride)
{
	mNumItems = numItems;
	mStride = stride;
	int total = numItems *stride;
	mValues.resize(total);
}

template <class T>
void atomCachedValues<T>::setValue(T &value,unsigned int item,unsigned int element)
{
	mValues[(item * mStride) + element] = value;
}

template <class T>
T atomCachedValues<T>::getValue(unsigned int item, unsigned int element)
{
	return mValues[(item * mStride) + element];
}
/*
	atomShortValues
*/
atomShortValues::atomShortValues(MPlug &plug, unsigned int numItems) :atomBasePlugAndValues(plug)
{
	mCachedValues.resize(numItems,1);
}
	
void atomShortValues::setValue(MDGContext &context,unsigned int index)
{
	MObject attribute = mPlug.attribute();

	if ( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnAttrib(attribute);

		switch(fnAttrib.unitType())
		{
			case MFnNumericData::kBoolean:
			{
				bool value;
				mPlug.getValue(value,context);
				short ival = value == true ? 1 : 0;
				mCachedValues.setValue(ival,index);
				break;
			}
			case MFnNumericData::kByte:
			case MFnNumericData::kChar:
			{
				char value;
				mPlug.getValue(value,context);
				short ival = (short) value;
				mCachedValues.setValue(ival,index);
				break;
			}
			case MFnNumericData::kShort:
			{
				short value;
				mPlug.getValue(value,context);
				mCachedValues.setValue(value,index);
				break;
			}
			default:
				break;
		}
	}
	else if( attribute.hasFn( MFn::kEnumAttribute ) )
	{
		short value;
		mPlug.getValue(value,context);
		mCachedValues.setValue(value,index);
	}
}

void atomShortValues::writeToAtomFile(ofstream & clip)
{
	for(unsigned int i=0;i<mCachedValues.numItems();++i)
	{
		clip << mCachedValues.getValue(i);
		clip << " ";
	}
}


/*
	atomIntValues
*/
atomIntValues::atomIntValues(MPlug &plug, unsigned int numItems) :atomBasePlugAndValues(plug)
{
	mCachedValues.resize(numItems,1);
}
	
void atomIntValues::setValue(MDGContext &context,unsigned int index)
{
	MObject attribute = mPlug.attribute();

	if ( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnAttrib(attribute);
		if(fnAttrib.unitType()==MFnNumericData::kLong)
		{
			int value;
			mPlug.getValue(value,context);
			mCachedValues.setValue(value,index);
		}
	}
}

void atomIntValues::writeToAtomFile(ofstream & clip)
{
	for(unsigned int i=0;i<mCachedValues.numItems();++i)
	{
		clip << mCachedValues.getValue(i);
		clip << " ";
	}
}


/*
	atomFloatValues
*/
atomFloatValues::atomFloatValues(MPlug &plug, unsigned int numItems,unsigned int stride) :atomBasePlugAndValues(plug)
{
	mCachedValues.resize(numItems,stride);
}
	
void atomFloatValues::setValue(MDGContext &context,unsigned int index)
{
	MObject attribute = mPlug.attribute();

	if ( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnAttrib(attribute);

		switch(fnAttrib.unitType())
		{
			case MFnNumericData::kFloat:
			{
				float value;
				mPlug.getValue(value,context);
				mCachedValues.setValue(value,index);
				break;
			}
			default:
				break;
		}
	}
}

void atomFloatValues::writeToAtomFile(ofstream & clip)
{
	for(unsigned int i=0;i<mCachedValues.numItems();++i)
	{
		for(unsigned int j=0;j<mCachedValues.stride();++j)
		{
			clip << mCachedValues.getValue(i,j);
			clip << " ";
		}
	}
}


/*
	atomDoubleValues
*/
atomDoubleValues::atomDoubleValues(MPlug &plug, unsigned int numItems,double scale) :atomBasePlugAndValues(plug), mScale(scale)
{
	mCachedValues.resize(numItems,1);
}
	
void atomDoubleValues::setValue(MDGContext &context,unsigned int index)
{
	MObject attribute = mPlug.attribute();

	if ( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnAttrib(attribute);

		switch(fnAttrib.unitType())
		{
			case MFnNumericData::kDouble:
			{
				double value;
				mPlug.getValue(value,context);
				mCachedValues.setValue(value,index);
				break;
			}
			default:
				break;
		}
	}
	else if( attribute.hasFn( MFn::kUnitAttribute ) )
	{
		MFnUnitAttribute fnAttrib(attribute);
		switch(fnAttrib.unitType())
		{
			case MFnUnitAttribute::kAngle:
			{
				double value;
				mPlug.getValue(value,context);
				value *= mScale;
 				mCachedValues.setValue(value,index);
				break;
			}
			case MFnUnitAttribute::kDistance:
			{
				double value;
				mPlug.getValue(value,context);
				value *= mScale;
				mCachedValues.setValue(value,index);
				break;
			}
			case MFnUnitAttribute::kTime:
			{
				double value;
				mPlug.getValue(value,context);
				mCachedValues.setValue(value,index);
				break;
			}
			default:
			{
				break;
			}
		}
	}
}

void atomDoubleValues::writeToAtomFile(ofstream & clip)
{
	for(unsigned int i=0;i<mCachedValues.numItems();++i)
	{
		clip << mCachedValues.getValue(i);
		clip << " ";
	}
}


/*

atom cached plugs

*/

atomCachedPlugs::atomCachedPlugs(MString &nodeName,MObject &object, const MPlugArray& animatablePlugs,
		bool sdk, bool constraint, bool animLayers, std::set<std::string> &attrStrings,
		atomTemplateReader &templateReader,unsigned int numItems,MAngle::Unit angularUnit,
		MDistance::Unit	linearUnit)

{
	getCachedPlugs(nodeName,animatablePlugs, sdk, constraint, animLayers, attrStrings,templateReader,numItems,angularUnit,linearUnit);
}


atomCachedPlugs::~atomCachedPlugs()
{
	for(unsigned int z =0; z< mCachedPlugs.size(); ++z)
	{
		if(mCachedPlugs[z])
			delete mCachedPlugs[z];
	}
}
void atomCachedPlugs::getCachedPlugs(MString &nodeName, const MPlugArray &animatablePlugs,
		bool sdk, bool constraint, bool animLayers,
		std::set<std::string> &attrStrings,atomTemplateReader &templateReader,unsigned int numItems,
		MAngle::Unit angularUnit,
		MDistance::Unit	linearUnit)
{
	std::set<std::string>::const_iterator constIter = attrStrings.end();
	unsigned int numPlugs = animatablePlugs.length();
	MPlugArray cachedPlugs;
	for (unsigned int i = 0; i < numPlugs; i++)
	{
		MPlug plug = animatablePlugs[i];
		MPlugArray destPlugArray;
		//temp test here. If the plug is connected, but not directly to an anim curve, then cache it
		//will get smarter about this later, but good for testing for now.
		bool isConnected = plug.connectedTo (destPlugArray, true,false);
		bool isConnectedButNotToAnimCurve = (isConnected	&& (destPlugArray.length() == 1 && 
			destPlugArray[0].node().hasFn(MFn::kAnimCurve)==false));
		bool animCurveIsConnected = false;
		//now check to see if the anim curve returned is connected and driven by something else
		if (isConnectedButNotToAnimCurve ==false && destPlugArray.length() ==1 &&
			destPlugArray[0].node().hasFn(MFn::kAnimCurve)==true)
		{
			MPlug curve = destPlugArray[0];
			destPlugArray.clear();
			curve.connectedTo(destPlugArray,true,false);
			animCurveIsConnected = destPlugArray.length() > 0; //
		}
		bool shouldBeCached = isConnectedButNotToAnimCurve ||animCurveIsConnected;
		//if not cached yet, see if hooked up to a sdk, don't do the same for a constraint or anim layers
		//since shouldBeCached will be true for them. Only sdk's may still be false here.
		if(sdk==false) //sdk is false when not doing export edits so do caching instead.
		{
			MObjectArray animationNodes;
			MPlugArray drivers;
			if(MAnimUtil::findSetDrivenKeyAnimation(plug,animationNodes,
				drivers)) //it's an sdk
				shouldBeCached = true;
		}

		if (shouldBeCached ) 
		{
			MPlug attrPlug (plug);
			MObject attrObj = attrPlug.attribute();
			MFnAttribute fnLeafAttr (attrObj);
			MString attrName;
			atomBase::getAttrName(plug,attrName);
			//template filter check first
			if(templateReader.findNodeAndAttr(nodeName,attrName) == false)
				continue; 

			//then if we have specified attr strings then don't save it out if it's not found
			//must use shortName since the channelBox command will always return a short name
			//the long name flag there is only used to turn long name (or nice name) display on
			if(attrStrings.size() == 0 || attrStrings.find(std::string(fnLeafAttr.shortName().asChar())) != constIter)
			{
				//okay at this point we may have a contraint which we don't want to cache if we are export editing them
				if(constraint)
				{
					MObject constraint;
					MObjectArray targets;
					if(MAnimUtil::findConstraint(plug,constraint,targets)) //it's a constraint, don't cache
						continue;
				}
				//or it may be hooked up to animLayers, again we don't want to cache it if that option is on
				if(animLayers)
				{
					MObjectArray layers;
					MPlugArray plugs;
					if(MAnimUtil::findAnimationLayers(plug,layers,plugs))
						continue;
				}
				cachedPlugs.append(plug);
			}
		}
	}
	//okay if we have any cached plugs then create the correct value array 
	if(cachedPlugs.length() >0 )
	{
		mCachedPlugs.resize(cachedPlugs.length());
		for(unsigned int i =0 ;i< cachedPlugs.length(); ++i)
		{
			atomBasePlugAndValues * plugAndValue = NULL;
			MPlug &plug = cachedPlugs[i];
			MObject attribute = plug.attribute();
		
			if ( attribute.hasFn( MFn::kNumericAttribute ) )
			{
				MFnNumericAttribute fnAttrib(attribute);

				switch(fnAttrib.unitType())
				{
					case MFnNumericData::kByte:
					case MFnNumericData::kChar:
					case MFnNumericData::kBoolean:
					case MFnNumericData::kShort:
					{
						plugAndValue = new atomShortValues(plug, numItems);
						break;
					}
					case MFnNumericData::kLong:
					{
						plugAndValue = new atomIntValues(plug, numItems);
						break;
					}
					case MFnNumericData::kFloat:
					{
						plugAndValue = new atomFloatValues(plug, numItems,1);
						break;
					}
					case MFnNumericData::kDouble:
					{
						plugAndValue = new atomDoubleValues(plug, numItems,1.0);
						break;
					}
					
					default:
						break;
				}
			}
			else if( attribute.hasFn( MFn::kUnitAttribute ) )
			{
				MFnUnitAttribute fnAttrib(attribute);
				switch(fnAttrib.unitType())
				{
					case MFnUnitAttribute::kAngle:
					{
						MAngle angle(1.0);
						double scale = angle.as(angularUnit);
						plugAndValue = new atomDoubleValues(plug, numItems,scale);
						break;
					}
					case MFnUnitAttribute::kDistance:
					{
						MDistance distance(1.0);
						double scale = distance.as(linearUnit);
						plugAndValue = new atomDoubleValues(plug, numItems,scale);
						break;
					}
					case MFnUnitAttribute::kTime:
					{
						plugAndValue = new atomDoubleValues(plug, numItems,1.0);
						break;
					}
					default:
					{
						break;
					}
				}
			}
			else if( attribute.hasFn( MFn::kEnumAttribute ) )
			{
				plugAndValue = new atomShortValues(plug, numItems);
			}
	
			mCachedPlugs[i] = plugAndValue;
		}
	}
}

MPlug& atomCachedPlugs::getPlug(unsigned int item)
{
	return mCachedPlugs[item]->getPlug();
}
void atomCachedPlugs::calculateValue(MDGContext &ctx,unsigned int item)
{
	for(unsigned int i = 0;i< mCachedPlugs.size(); ++i)
	{
		if(mCachedPlugs[i])
			mCachedPlugs[i]->setValue(ctx,item);
	}
}

void atomCachedPlugs::writeValues(ofstream &clip,unsigned int item)
{
	if(item< mCachedPlugs.size())
	{
		if(mCachedPlugs[item])
			mCachedPlugs[item]->writeToAtomFile(clip);
	}
}

//todo need to add layerName support
bool atomCachedPlugs::isAttrCached(const MString &attrName,const  MString &layerName)
{
	if(hasCached())
	{
		//currently since the plugs are in a vector we iterate one by one,
		//we may change this implementation to use a map instead to speed this up though it won't be too
		//bad we don't think.
		for(unsigned int i = 0;i< mCachedPlugs.size(); ++i)
		{
			if(mCachedPlugs[i])
			{
				MPlug plug = mCachedPlugs[i]->getPlug();
				MString name;
				atomBase::getAttrName(plug,name);
				if(name == attrName)
					return true;
			}
		}
	}
	return false;
}


