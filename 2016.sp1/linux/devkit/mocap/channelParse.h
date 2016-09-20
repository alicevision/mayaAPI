/*
//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
*/

#ifndef _WIN32
#include <maya/mocapserver.h>
#else
#include <maya/mocapserver.h>
#endif

#define kScaleStride 3
enum channelInfoOpCode {
	kInvalid    = -999,
	kFactors    = -5,
	kStartParse = -4,
	kEndParse   = -3,
	kUnit       = -2,
	kRotOrder   = -1,
	
};

enum channelInfoAxis {
	kInval = -1,
	kXAxis = 0,
	kYAxis = 1,
	kZAxis = 2
};

enum channelScaleOffsetType {

	kOffset = 0,
	kMult   = 1
};
enum channelUsageType {
	kPosQuat  = -3,
	kPosRot   = -2,
	kRotation = 0,
	kPosition = 1,
	kScale    = 2,
};

enum channelInfoUnitType {
	kUnitRot,
	kUnitPos
};

typedef struct {
	const char *shortName;
	const char *longName;
	CapChannelUsage use;
	union {
		int dim;
		int count;
	} p1;

	union {
		int  extra;
		int  opCode;
	} p2;

	union {
		int axisOffset;
		int factorType;
	} p3;

	int typeOffset;

} channelParseInfo;

typedef struct channelParseUnits {
	const char  *shortName;
	const char  *longName;
	int	   parm;
	float value;
} channelParseUnits;

typedef struct {
	char  *shortName;
	char  *longName;
	CapRotationOrder   value;
} channelParseRotOrder;

typedef struct channelInfo {
	CapChannel       chan;
	channelParseInfo *info;
	int	             startingCol;
	float            factors[2][3][3];
	CapRotationOrder rotOrder;
	float			 *data; /* numColumn entries */
	char			 *name; /* string of exactly strlen(name) +1 */
	struct channelInfo *next;
} channelInfo;

void channelInfoSetData(channelInfo *chan, int follow, float *rawData);
channelInfo *channelInfoCreate(FILE *configFile, 
							   int lookForBegin,
							   channelInfo *head);
