//-
//*****************************************************************************
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+

#include "AbcBulletStringTable.h"
#include "MayaTransformCollectionWriter.h"
#include "MayaUtility.h"
#include <maya/MDataHandle.h>
#include <maya/MFnPluginData.h>
#include <maya/MPxData.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MQuaternion.h>
#include <maya/MGlobal.h>
#include <maya/MStringResource.h>
#include <maya/MMatrix.h>
#include <maya/MPoint.h>

void MayaTransformCollectionItem::sample(double iFrame, const MMatrix& wmat)
{
	mXform = wmat * mInvMat;
#if _DEBUG
	if (mVerbose)
	{
		MString strItem, strInfo;
		MStringArray args(10,"");
		unsigned int i = 0;

		// translation
		MVector pos = xform().translation(MSpace::kTransform);
		MEulerRotation euler = xform().eulerRotation();
		
		i=0;
		args[i++] = name();
		args[i++].set(iFrame);

		strItem.format("sample=^1s, node=^2s",args);
		strInfo += strItem;

		i=0;
		args[i++].set(pos[0]);
		args[i++].set(pos[1]);
		args[i++].set(pos[2]);

		args[i++].set(euler[0]);
		args[i++].set(euler[1]);
		args[i++].set(euler[2]);

		args[i++].set(Alembic::AbcGeom::RadiansToDegrees(euler[0]));
		args[i++].set(Alembic::AbcGeom::RadiansToDegrees(euler[1]));
		args[i++].set(Alembic::AbcGeom::RadiansToDegrees(euler[2]));

		strItem.format("translation=[^1s, ^2s, ^3s], eulerRot=[^4s, ^5s, ^6s] degreesRot=[^7s, ^8s, ^9s]", args);
		strInfo += ", ";
		strInfo += strItem;

		MGlobal::displayInfo( strInfo );
	}
#endif
}

double MayaTransformCollectionItem::asDouble(Alembic::AbcGeom::XformOperationType channelOp, Alembic::Util::uint32_t channelNum)
{
	double result = 0.0;

	if (isTranslationChannel(channelOp,channelNum))
	{
		result = mXform.translation(MSpace::kTransform)[channelNum];
	}
	else if (isRotationChannel(channelOp,channelNum)) 
	{
		int index = (channelOp==Alembic::AbcGeom::kRotateXOperation) ? 0 : (channelOp==Alembic::AbcGeom::kRotateYOperation) ? 1 : 2;

		result = mXform.eulerRotation()[index];
	}

	return result;
}


MayaTransformCollectionWriter::MayaTransformCollectionWriter
	(
	Alembic::AbcGeom::OObject & iParent,
	MDagPath & iDag, 
	Alembic::Util::uint32_t iTimeIndex, 
	const JobArgs & iArgs
	)
	: mVerbose(iArgs.verbose)
{
	MStatus stat;

	// get SolvedState state from iDag 
	MFnDependencyNode depSolvedState(iDag.node(&stat));

	// get the outCollisionObjects plug from the solvedState
	// we'll use this to ensure the solver data is upto date each time we
	// change frames, even though we don't need to the output mesh
	mCollisionObjectsPlug = depSolvedState.findPlug("outCollisionObjects", &stat);
	assert(stat == MS::kSuccess);

	// add node to transform write list
	MString fullpath, name;
	MDagPath dagPath;

	// call evaluateNumElements to pull the latest solution
	for (unsigned i=0, m = mCollisionObjectsPlug.evaluateNumElements(); i < m && mCollisionObjectsPlug.isArray(); i++)
	{
		// add leaf node
		MPlug elementPlug = mCollisionObjectsPlug[i];
		MPlug dagPathPlug = elementPlug.child(0);

		stat = dagPathPlug.getValue(fullpath);

		name = fullpath.substringW(fullpath.rindex('|') + 1, fullpath.length()-1);

		MSelectionList sList;
		stat = sList.add(fullpath);
		assert(stat == MS::kSuccess);

		sList.getDagPath(0, dagPath);
		assert(stat == MS::kSuccess);

		MMatrix invMat;	// by default its the identity;

		// we have atleast 1 ancestor
		if (dagPath.length()>1)
		{
			MDagPath parentDagPath(dagPath);
			parentDagPath.pop();

			invMat = parentDagPath.inclusiveMatrixInverse();
		}

		MayaTransformCollectionItemPtr sampler = 
			MayaTransformCollectionItemPtr(new MayaTransformCollectionItem(name, invMat, i, iArgs.verbose));
		mSamplerList.push_back(sampler);

		// sample first frame
		{
			MPlug wmatPlug = elementPlug.child(1);

			MDataHandle dhMatrix;
			stat = wmatPlug.getValue(dhMatrix);
			assert(stat == MS::kSuccess);

			sampler->sample(iTimeIndex, dhMatrix.asMatrix());
		}

		MayaTransformWriterPtr trans;
		
		// Copy the dag path because we'll be popping from it
		//
		MDagPath dag(dagPath);
	
		// precondition: iParent will already be at the root of the tree
		//	
		Alembic::Abc::OObject iRoot = iParent;
		Alembic::Abc::OObject iCurrent = iRoot;

		// Create the non-animation-based transforms (if any)
		//
		int j;
		int numPaths = dag.length();
		if (numPaths>1)
		{
			std::vector< MDagPath > dagList;

			for (j = numPaths - 1; j > -1; j--, dag.pop())
			{
				dagList.push_back(dag);
			}

			std::vector< MDagPath >::iterator iStart = dagList.begin();
			std::vector< MDagPath >::iterator iCur = dagList.end();
			iCur--;

			// now loop backwards over our DAG path list so we push ancestor nodes
			// first, all the way down to the current node
			//
			// This essentially reads the DAG paths left to right (or top down)
			// and checks to see if their components already exist in Alembic;
			// if a DAG path component is missing, it'll be added.
			//
			for (; iCur != iStart; iCur--)
			{
				MString currentDagPathName = (*iCur).fullPathName();
				MStringArray pathArray;
				currentDagPathName.split('|', pathArray);
				iCurrent = iRoot;
				for (unsigned int i = 0; i < pathArray.length(); i++) {
					// make sure you strip the namespace off the path before you
					// try to look up the alembic node.
					MString step = util::stripNamespaces(pathArray[i], iArgs.stripNamespace);
					Alembic::Abc::OObject iPriorCurrent = iCurrent;
					iCurrent = iCurrent.getChild(step.asChar());
					if (!iCurrent.valid()) {
						iCurrent = iPriorCurrent;
						trans = MayaTransformWriterPtr(new MayaTransformWriter(iCurrent, *iCur, iTimeIndex, iArgs));
						iCurrent = iCurrent.getChild(step.asChar());
					}
				}
			}
		}
		assert(iCurrent.valid());

		// Remember to continue the hierarchy - to include the animation-based transform
		// It is assumed that a leaf entry won't exist in Alembic due to the uniqueness of a full transform path
		//

		// Create the animation-based transform
		//
		trans = MayaTransformWriterPtr(new MayaTransformWriter(
			iCurrent, *sampler, iTimeIndex, iArgs));

		//TODO: Remove
		mTransList.push_back(trans);

		AttributesWriterPtr attrs = trans->getAttrs();
		if (attrs)
		{
			if (iTimeIndex != 0 && attrs->isAnimated())
				mTransAttrList.push_back(attrs);
		}

	}
}

MayaTransformCollectionWriter::~MayaTransformCollectionWriter()
{
}

void MayaTransformCollectionWriter::write(double iFrame)
{
	MStatus stat;

	// write out transforms 
	std::vector< MayaTransformWriterPtr >::iterator tcur =
		mTransList.begin();

	std::vector< MayaTransformCollectionItemPtr >::iterator scur =
		mSamplerList.begin();

	std::vector< MayaTransformWriterPtr >::iterator tend =
		mTransList.end();

	std::vector< MayaTransformCollectionItemPtr >::iterator send =
		mSamplerList.end();

	assert( mSamplerList.size() == mTransList.size() );

	for (unsigned int i=0, m = mCollisionObjectsPlug.evaluateNumElements(); tcur != tend && scur != send && i < m; tcur++, scur++, i++)
	{
		MPlug elementPlug = mCollisionObjectsPlug[i];
		MPlug wmatPlug = elementPlug.child(1);

		MDataHandle dhMatrix;
		stat = wmatPlug.getValue(dhMatrix);
		assert(stat == MS::kSuccess);

		// sample result
		(*scur)->sample(iFrame, dhMatrix.asMatrix());

		(*tcur)->write();

	}

	std::vector< AttributesWriterPtr >::iterator tattrCur =
		mTransAttrList.begin();

	std::vector< AttributesWriterPtr >::iterator tattrEnd =
		mTransAttrList.end();

	for(; tattrCur != tattrEnd; tattrCur++)
	{
		(*tattrCur)->write();
	}
}

bool MayaTransformCollectionWriter::isAnimated() const
{
	return true;
}



