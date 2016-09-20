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

#include "AbcWriteJob.h"
#include <Alembic/AbcCoreHDF5/All.h>

namespace
{
	void hasDuplicates(const util::ShapeSet & dagPath, unsigned int stripDepth)
	{
		std::map<std::string, MDagPath> roots;
		const util::ShapeSet::const_iterator end = dagPath.end();
		for (util::ShapeSet::const_iterator it = dagPath.begin();
			it != end; it++)
		{
			std::string fullName = it->fullPathName().asChar();
			if (!fullName.empty() && fullName[0] == '|')
			{
				fullName = fullName.substr(1);
			}

			if (stripDepth > 0)
			{
				MString name = fullName.c_str();
				fullName = util::stripNamespaces(name, stripDepth).asChar();
			}

			std::map<std::string, MDagPath>::iterator strIt =
				roots.find(fullName);
			if (strIt != roots.end())
			{
				std::string theError = "Conflicting root node names specified: ";
				theError += it->fullPathName().asChar();
				theError += " ";
				theError += strIt->second.fullPathName().asChar();
				if (stripDepth > 0)
				{
					theError += " with -stripNamespace specified.";
				}
				throw std::runtime_error(theError);
			}
			else
			{
				roots[fullName] = *it;
			}
		}
	}

	void addToString(std::string & str,
		const std::string & name, unsigned int value)
	{
		if (value > 0)
		{
			std::stringstream ss;
			ss << value;
			str += name + std::string(" ") + ss.str() + std::string(" ");
		}
	}

	void processCallback(std::string iCallback, bool isMelCallback,
		double iFrame, const MBoundingBox & iBbox)
	{
		if (iCallback.empty())
			return;

		size_t pos = iCallback.find("#FRAME#");
		if ( pos != std::string::npos )
		{
			std::stringstream sstrm;
			sstrm.precision(std::numeric_limits<double>::digits10);
			sstrm << iFrame;
			std::string str = sstrm.str();

			iCallback.replace(pos, 7, str);
		}

		pos = iCallback.find("#BOUNDS#");
		if ( pos != std::string::npos )
		{
			std::stringstream sstrm;
			sstrm.precision(std::numeric_limits<float>::digits10);
			sstrm << " " << iBbox.min().x << " " << iBbox.min().y << " " <<
				iBbox.min().z << " " << iBbox.max().x << " " <<
				iBbox.max().y << " " <<iBbox.max().z;

			std::string str = sstrm.str();

			iCallback.replace(pos, 8, str);
		}

		pos = iCallback.find("#BOUNDSARRAY#");
		if ( pos != std::string::npos )
		{
			std::stringstream sstrm;
			sstrm.precision(std::numeric_limits<float>::digits10);

			if (isMelCallback)
			{
				sstrm << " {";
			}
			else
			{
				sstrm << " [";
			}

			sstrm << iBbox.min().x << "," << iBbox.min().y << "," <<
				iBbox.min().z << "," << iBbox.max().x << "," <<
				iBbox.max().y << "," << iBbox.max().z;

			if (isMelCallback)
			{
				sstrm << "} ";
			}
			else
			{
				sstrm << "] ";
			}
			std::string str = sstrm.str();
			iCallback.replace(pos, 13, str);
		}

		if (isMelCallback)
			MGlobal::executeCommand(iCallback.c_str(), true);
		else
			MGlobal::executePythonCommand(iCallback.c_str(), true);
	}
}

AbcWriteJob::AbcWriteJob(const char * iFileName,
	std::set<double> & iTransFrames,
	Alembic::AbcCoreAbstract::TimeSamplingPtr iTransTime,
	const JobArgs & iArgs)
{
	MStatus status;
	mFileName = iFileName;
	mBoxIndex = 0;
	mArgs = iArgs;
	mTransSamples = 1;

	if (mArgs.useSelectionList)
	{
		bool emptyDagPaths = mArgs.dagPaths.empty();

		// get the active selection
		MSelectionList activeList;
		MGlobal::getActiveSelectionList(activeList);
		mSList = activeList;
		unsigned int selectionSize = activeList.length();
		for (unsigned int index = 0; index < selectionSize; index ++)
		{
			MDagPath dagPath;
			status = activeList.getDagPath(index, dagPath);
			if (status == MS::kSuccess)
			{
				unsigned int length = dagPath.length();
				while (--length)
				{
					dagPath.pop();
					mSList.add(dagPath, MObject::kNullObj, true);
				}

				if (emptyDagPaths)
				{
					mArgs.dagPaths.insert(dagPath);
				}
			}
		}
	}

	mTransFrames = iTransFrames;

	// only needed during creation of the transforms
	mTransTime = iTransTime;
	mTransTimeIndex = 0;

	// should have at least 1 value
	assert(!mTransFrames.empty());

	mFirstFrame = *(mTransFrames.begin());
	std::set<double>::iterator last = mTransFrames.end();
	last--;
	mLastFrame = *last;
}

void AbcWriteJob::setup(double iFrame, MayaTransformWriterPtr iParent, util::GetMembersMap& gmMap)
{
	MStatus status;

	// short-circuit if selection flag is on but this node isn't actively
	// selected
	if (mArgs.useSelectionList && !mSList.hasItem(mCurDag))
		return;

	MObject ob = mCurDag.node();

	MFnDependencyNode fnDepNode(ob, &status);

	bool bSolvedState = fnDepNode.typeName() == "bulletRigidCollection";

	// skip all intermediate nodes (and their children)
	if (util::isIntermediate(ob))
	{
		if (!bSolvedState)
		{
			return;
		}
	}

	// skip nodes that aren't renderable (and their children)
	if (mArgs.excludeInvisible && !util::isRenderable(ob))
	{
		return;
	}

	if ( bSolvedState )
	{
		// the motionstates are held on the initialstate node at the moment
		if (status != MS::kSuccess)
		{
			MString msg = "Initialize transform collection node ";
			msg += mCurDag.fullPathName();
			msg += " failed, skipping.";
			MGlobal::displayWarning(msg);
			return;
		}

		MayaTransformCollectionWriterPtr transCol;

		// transformcollections always parented to the root case
		Alembic::Abc::OObject obj = mRoot.getTop();
		transCol = MayaTransformCollectionWriterPtr(new MayaTransformCollectionWriter(
			obj, mCurDag, mTransTimeIndex, mArgs));

		mTransColList.push_back(transCol);
		mStats.mTransColNum++;

		AttributesWriterPtr attrs = transCol->getAttrs();
		if (attrs)
		{
			if (mTransTimeIndex != 0 && attrs->isAnimated())
				mTransColAttrList.push_back(attrs);
		}
	}
	else if (ob.hasFn(MFn::kTransform))
	{
		MFnTransform fnTrans(ob, &status);
		if (status != MS::kSuccess)
		{
			MString msg = "Initialize transform node ";
			msg += mCurDag.fullPathName();
			msg += " failed, skipping.";
			MGlobal::displayWarning(msg);
			return;
		}

		MayaTransformWriterPtr trans;

		// parented to the root case
		if (iParent == NULL)
		{
			Alembic::Abc::OObject obj = mRoot.getTop();
			trans = MayaTransformWriterPtr(new MayaTransformWriter(
				obj, mCurDag, mTransTimeIndex, mArgs));
		}
		else
		{
			trans = MayaTransformWriterPtr(new MayaTransformWriter(
				*iParent, mCurDag, mTransTimeIndex, mArgs));
		}

		if (trans->isAnimated() && mTransTimeIndex != 0)
		{
			mTransList.push_back(trans);
			mStats.mTransAnimNum++;
		}
		else
			mStats.mTransStaticNum++;

		AttributesWriterPtr attrs = trans->getAttrs();
		if (mTransTimeIndex != 0 && attrs->isAnimated())
			mTransAttrList.push_back(attrs);

		// loop through the children, making sure to push and pop them
		// from the MDagPath
		unsigned int numChild = mCurDag.childCount();
		for (unsigned int i = 0; i < numChild; ++i)
		{
			mCurDag.push(mCurDag.child(i));
			setup(iFrame, trans, gmMap);
			mCurDag.pop();
		}
	}
	else
	{
		MString warn = mCurDag.fullPathName() + " is an unsupported type of ";
		warn += ob.apiTypeStr();
		MGlobal::displayWarning(warn);
	}
}


AbcWriteJob::~AbcWriteJob()
{
}

bool AbcWriteJob::eval(double iFrame)
{
	if (iFrame == mFirstFrame)
	{
		// check if the shortnames of any two nodes are the same
		// if so, exit here
		hasDuplicates(mArgs.dagPaths, mArgs.stripNamespace);

		std::string appWriter = "Maya ";
		appWriter += MGlobal::mayaVersion().asChar();
		appWriter += " AbcBullet v";
		appWriter += ABCBULLET_VERSION;

		std::string userInfo = "Exported from: ";
		userInfo += MFileIO::currentFile().asChar();

		// these symbols can't be in the meta data
		if (userInfo.find('=') != std::string::npos ||
			userInfo.find(';') != std::string::npos)
		{
			userInfo = "";
		}

		mRoot = CreateArchiveWithInfo(Alembic::AbcCoreHDF5::WriteArchive(),
			mFileName, appWriter, userInfo,
			Alembic::Abc::ErrorHandler::kThrowPolicy);
		mTransTimeIndex = mRoot.addTimeSampling(*mTransTime);

		mBoxProp =  Alembic::AbcGeom::CreateOArchiveBounds(mRoot,
			mTransTimeIndex);

		if (!mRoot.valid())
		{
			std::string theError = "Unable to create abc file";
			throw std::runtime_error(theError);
		}

		util::ShapeSet::const_iterator end = mArgs.dagPaths.end();
		util::GetMembersMap gmMap;
		for (util::ShapeSet::const_iterator it = mArgs.dagPaths.begin();
			it != end; ++it)
		{
			mCurDag = *it;
			setup(iFrame * util::spf(), MayaTransformWriterPtr(), gmMap);
		}
		perFrameCallback(iFrame);
	}
	else
	{
		std::set<double>::iterator checkFrame = mTransFrames.find(iFrame);
		bool foundTransFrame = false;
		if (checkFrame != mTransFrames.end())
		{
			assert(mRoot.valid());
			foundTransFrame = true;
			mTransSamples ++;

			// write out transforms
			{
				std::vector< MayaTransformWriterPtr >::iterator tcur =
					mTransList.begin();

				std::vector< MayaTransformWriterPtr >::iterator tend =
					mTransList.end();

				for (; tcur != tend; tcur++)
				{
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

			// write out transform collections
			{
				std::vector< MayaTransformCollectionWriterPtr >::iterator tcur =
					mTransColList.begin();

				std::vector< MayaTransformCollectionWriterPtr >::iterator tend =
					mTransColList.end();

				for (; tcur != tend; tcur++)
				{
					(*tcur)->write(iFrame);
				}

				std::vector< AttributesWriterPtr >::iterator tattrCur =
					mTransColAttrList.begin();

				std::vector< AttributesWriterPtr >::iterator tattrEnd =
					mTransColAttrList.end();

				for(; tattrCur != tattrEnd; tattrCur++)
				{
					(*tattrCur)->write();
				}
			}
		}
		if (foundTransFrame)
			perFrameCallback(iFrame);
	}

	if (iFrame == mLastFrame)
	{
		postCallback(iFrame);
		return true;
	}

	return false;
}

void AbcWriteJob::perFrameCallback(double iFrame)
{
	MBoundingBox bbox;

	processCallback(mArgs.melPerFrameCallback, true, iFrame, bbox);
	processCallback(mArgs.pythonPerFrameCallback, false, iFrame, bbox);
}


// write the frame ranges and statistic string on the root
// Also call the post callbacks
void AbcWriteJob::postCallback(double iFrame)
{
	std::string statsStr = "";

	addToString(statsStr, "TransStaticNum", mStats.mTransStaticNum);
	addToString(statsStr, "TransAnimNum", mStats.mTransAnimNum);
	addToString(statsStr, "TransColNum", mStats.mTransColNum);

	if (statsStr.length() > 0)
	{
		Alembic::Abc::OStringProperty stats(mRoot.getTop().getProperties(),
			"statistics");
		stats.set(statsStr);
	}

	if (mTransTimeIndex != 0)
	{
		MString propName;
		propName += static_cast<int>(mTransTimeIndex);
		propName += ".samples";
		Alembic::Abc::OUInt32Property samp(mRoot.getTop().getProperties(),
			propName.asChar());
		samp.set(mTransSamples);
	}

	MBoundingBox bbox;

	processCallback(mArgs.melPostCallback, true, iFrame, bbox);
	processCallback(mArgs.pythonPostCallback, false, iFrame, bbox);
}


