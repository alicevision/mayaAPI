#ifndef _engine
#define _engine

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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#ifdef WIN32
#	include <io.h>
#pragma warning (disable : 4244)
#else
#	include <unistd.h>
#endif
#include <string.h>
#include <math.h>
#include <values.h>

/* Constants ============================================================== */
#define kFileMaxWordSize 256	/* maximum size of a word					*/

/* Common types =========================================================== */
typedef int EtInt;			/* natural int representation					*/
typedef float EtFloat;		/* natural float representation					*/
#define kEngineFloatMax (FLT_MAX)	/* Opposite of 0						*/
typedef void EtVoid;		/* void											*/
#define kEngineNULL (0)		/* NULL											*/
typedef char EtBoolean;		/* boolean										*/
#define kEngineTRUE ((EtBoolean)1)
#define kEngineFALSE ((EtBoolean)0)
typedef unsigned char EtByte;		/* 1 byte								*/

/* File Engine ============================================================ */
typedef int EtFileHandle;	/* type used for referencing a file				*/
#define kFileBadParam -2	/* a bad parameter was passed					*/
#define kFileNotOpened -1	/* file could not be opened						*/

typedef const char * EtFileName;	/* type used for a file name			*/

/* Anim Engine ============================================================ */
typedef float EtTime;		/* type used for key times (in seconds)			*/
typedef float EtValue;		/* type used for key values (in internal units)	*/

struct EtKey {
	EtTime	time;			/* key time (in seconds)						*/
	EtValue	value;			/* key value (in internal units)				*/
	EtFloat	inTanX;			/* key in-tangent x value						*/
	EtFloat	inTanY;			/* key in-tangent y value						*/
	EtFloat	outTanX;		/* key out-tangent x value						*/
	EtFloat	outTanY;		/* key out-tangent y value						*/
};
typedef struct EtKey EtKey;

enum EtInfinityType {
	kInfinityConstant,
	kInfinityLinear,
	kInfinityCycle,
	kInfinityCycleRelative,
	kInfinityOscillate
};
typedef enum EtInfinityType EtInfinityType;

struct EtCurve {
	EtInt		numKeys;	/* The number of keys in the anim curve			*/
	EtBoolean	isWeighted;	/* whether or not this curve has weighted tangents */
	EtBoolean	isStatic;	/* whether or not all the keys have the same value */
	EtInfinityType preInfinity;		/* how to evaluate pre-infinity			*/
	EtInfinityType postInfinity;	/* how to evaluate post-infinity		*/

	/* evaluate cache */
	EtKey *		lastKey;	/* lastKey evaluated							*/
	EtInt		lastIndex;	/* last index evaluated							*/
	EtInt		lastInterval;	/* last interval evaluated					*/
	EtBoolean	isStep;		/* whether or not this interval is a step interval */
	EtBoolean	isStepNext;		/* whether or not this interval is a step interval */
	EtBoolean	isLinear;	/* whether or not this interval is linear		*/
	EtValue		fX1;		/* start x of the segment						*/
	EtValue		fX4;		/* end x of the segment							*/
	EtValue		fCoeff[4];	/* bezier x parameters (only used for weighted curves */
	EtValue		fPolyY[4];	/* bezier y parameters							*/

	EtKey		keyList[1];	/* This must be the last element and contains	*/
							/* an array of keys sorted in ascending order	*/
							/* by time										*/
};
typedef struct EtCurve EtCurve;

struct EtChannel {
	EtByte *			channel;	/* the name of the channel				*/
	EtCurve *			curve;		/* the animation curve					*/
	struct EtChannel *	next;		/* the next animation curve in a linked list */
};
typedef struct EtChannel EtChannel;

#endif
