#ifndef _cmdUtils_h_
#define _cmdUtils_h_

#include <cassert>
#include <maya/MStatus.h>
#include <maya/MArgDatabase.h>

//======================================================================
//
// Some useful error handling macros.
//
#define MStatError(status,msg)                              \
    if ( MS::kSuccess != (status) ) {                       \
        MPxCommand::displayError(                           \
            (msg) + MString(":") + (status).errorString()); \
        return (status);                                    \
    }

#define MStatErrorNullObj(status,msg)                       \
    if ( MS::kSuccess != (status) ) {                       \
        MPxCommand::displayError(                           \
            (msg) + MString(":") + (status).errorString()); \
        return MObject::kNullObj;                           \
    }

#define MCheckReturn(expression)                \
    {                                           \
        MStatus status = (expression);          \
        if ( MS::kSuccess != (status) ) {       \
            return (status);                    \
        }                                       \
    }

//======================================================================
// Valid modes for a command to execute. They form a bitfield so that legal
// modes can be stored in a single integer but really only one mode can be
// active at a time.
enum CommandMode
{
	kCreate = 0x01,
	kEdit   = 0x02,
	kQuery  = 0x04
};

//======================================================================
//
// Helper class for packaging up command options into a simple flag object.
//
template <class T, CommandMode ValidModes>
class OptFlag
{
public:
	OptFlag() : fIsSet(false), fArg() {}

	void parse(const MArgDatabase& argDb, const char* name)
	{
		MStatus status;
		fIsSet = argDb.isFlagSet(name, &status);
		assert(status == MS::kSuccess);
		
		status = argDb.getFlagArgument(name, 0, fArg);
		fIsArgValid = status == MS::kSuccess;
	}

	bool isModeValid(const CommandMode currentMode)
	{
		return !fIsSet || ((currentMode & ValidModes) != 0);
	}
	
	bool isSet		()		const { return fIsSet; }
	bool isArgValid	()		const { return fIsArgValid; }
	const T& arg	()		const { return fArg; }

	const T& arg(const T& defValue) const
	{
		if (isSet())
		{
			assert(isArgValid());
			return fArg;
		}
		else
		{
			return defValue;
		}
	}
	
private:
	bool	fIsSet;
	bool	fIsArgValid;
	T		fArg;
};

//======================================================================
//
// Specialization for flags with no argument.
//
template <CommandMode ValidModes>
class OptFlag<void, ValidModes>
{
public:
	OptFlag() : fIsSet(false) {}

	void parse(const MArgDatabase& argDb, const char* name)
	{
		MStatus status;
		fIsSet = argDb.isFlagSet(name, &status);
		assert(status == MS::kSuccess);
	}

	bool isModeValid(const CommandMode currentMode)
	{
		return !fIsSet || ((currentMode & ValidModes) != 0);
	}
	
	bool	isSet()	const { return fIsSet; }
	
private:
	bool	fIsSet;
};

//-
//**************************************************************************/
// Copyright (c) 2012 Autodesk, Inc.
// All rights reserved.
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
//+

#endif // _cmdUtils_h_
