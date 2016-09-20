//-*****************************************************************************
//
// Copyright (c) 2009-2012,
//  Sony Pictures Imageworks Inc. and
//  Industrial Light & Magic, a division of Lucasfilm Entertainment Company Ltd.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *	   Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *	   Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *	   Neither the name of Sony Pictures Imageworks, nor
// Industrial Light & Magic, nor the names of their contributors may be used
// to endorse or promote products derived from this software without specific
// prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//-*****************************************************************************

#ifndef _AbcExport_AbcWriteJob_h_
#define _AbcExport_AbcWriteJob_h_

#include "Foundation.h"

#include "MayaTransformWriter.h"
#include "MayaTransformCollectionWriter.h"

#include "MayaUtility.h"

struct AbcWriteJobStatistics
{
	AbcWriteJobStatistics()
	{
		mTransStaticNum = 0;
		mTransAnimNum = 0;
		mTransColNum = 0;
	}

	// for the statistic string
	unsigned int mTransStaticNum;
	unsigned int mTransAnimNum;
	unsigned int mTransColNum;
};

class AbcWriteJob
{
  public:

	AbcWriteJob(const char * iFileName,
		std::set<double> & iTransFrames,
		Alembic::AbcCoreAbstract::TimeSamplingPtr iTransTime,
		const JobArgs & iArgs);

	~AbcWriteJob();

	// returns true if eval has been called on the last frame
	bool eval(double iFrame);

  private:

	void perFrameCallback(double iFrame);
	void postCallback(double iFrame);

	void setup(double iFrame, MayaTransformWriterPtr iParent,
			   util::GetMembersMap& gmMap);

	std::vector< MayaTransformWriterPtr > mTransList;
	std::vector< AttributesWriterPtr > mTransAttrList;

	std::vector< MayaTransformCollectionWriterPtr > mTransColList;
	std::vector< AttributesWriterPtr > mTransColAttrList;

	// helper dag path for recursive calculations
	MDagPath mCurDag;

	// the root world node of the scene
	Alembic::Abc::OArchive mRoot;

	std::string mFileName;

	MSelectionList mSList;

	std::set<double> mTransFrames;
	Alembic::AbcCoreAbstract::TimeSamplingPtr mTransTime;
	Alembic::Util::uint32_t mTransTimeIndex;
	Alembic::Util::uint32_t mTransSamples;

	// when eval is called and the time is the first frame
	// then we run the setup
	double mFirstFrame;

	// when eval is called and the time is the last frame
	// then we also call the post callback
	double mLastFrame;

	Alembic::Abc::OBox3dProperty mBoxProp;
	unsigned int mBoxIndex;

	AbcWriteJobStatistics mStats;
	JobArgs mArgs;
};

typedef Alembic::Util::shared_ptr < AbcWriteJob > AbcWriteJobPtr;

#endif  // _AbcExport_AbcWriteJob_h_
