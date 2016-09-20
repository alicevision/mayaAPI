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

#ifndef _AbcExport_MayaTransformCollectionWriter_h_
#define _AbcExport_MayaTransformCollectionWriter_h_

#include "Foundation.h"

#include "AttributesWriter.h"
#include "MayaTransformWriter.h"
#include <maya/MTransformationMatrix.h>

struct MayaTransformCollectionItem : public AnimSampler
{
	MayaTransformCollectionItem(const MString& name, const MMatrix& invMat, int itemIndex, bool verbose=false)
		: mName(name)
		, mItemIndex(itemIndex)
		, mVerbose(verbose)
		, mInvMat(invMat)
	{
	}

	void sample(double iFrame, const MMatrix& wmat);

	virtual const MString& name() const { return mName; } 
	double asDouble(Alembic::AbcGeom::XformOperationType channelOp, Alembic::Util::uint32_t channelNum);
	const MTransformationMatrix& xform() const { return mXform; } 

	MString					mName;
	int						mItemIndex;

	MTransformationMatrix	mXform;
	MMatrix					mInvMat;
	bool					mVerbose;
};

typedef Alembic::Util::shared_ptr < MayaTransformCollectionItem >
	MayaTransformCollectionItemPtr;

// Writes a Collection of Transforms
class MayaTransformCollectionWriter 
{
	public:

	MayaTransformCollectionWriter(Alembic::Abc::OObject & iParent, MDagPath & iDag, 
		Alembic::Util::uint32_t iTimeIndex, const JobArgs & iArgs);

	~MayaTransformCollectionWriter();
	void write(double iFrame);
	bool isAnimated() const;
	Alembic::Abc::OObject getObject() { return Alembic::Abc::OObject(); /* mSchema.getObject(); */ };
	AttributesWriterPtr getAttrs() {return mAttrs;};

  private:
	bool mVerbose;
	MPlug mCollisionObjectsPlug;

	std::vector< MayaTransformCollectionItemPtr > mSamplerList;
	std::vector< MayaTransformWriterPtr > mTransList;
	std::vector< AttributesWriterPtr > mTransAttrList;
	AttributesWriterPtr mAttrs;

};

typedef Alembic::Util::shared_ptr < MayaTransformCollectionWriter >
	MayaTransformCollectionWriterPtr;

#endif  // _AbcExport_MayaTransformCollectionWriter_h_
