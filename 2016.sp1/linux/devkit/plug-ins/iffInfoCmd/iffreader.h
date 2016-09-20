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

#ifndef _iffreader_h
#define _iffreader_h

#include <math.h>
#include <maya/ilib.h>

typedef unsigned char byte;

class MStatus;
class MString;

class IFFimageReader
{
public:
	IFFimageReader ();
	~IFFimageReader ();
	MStatus open (MString filename);
	MStatus close ();
	MStatus readImage ();
	MStatus getSize (int &x, int &y);
	MString errorString ();
	int getBytesPerChannel ();
	bool isRGB ();
	bool isGrayscale ();
	bool hasDepthMap ();
	bool hasAlpha ();
	MStatus getPixel (int x, int y, int *r, int *g, int *b, int *a = NULL);
	MStatus getDepth (int x, int y, float *d);
	const byte *getPixelMap () const;
	const float *getDepthMap () const;

protected:
	ILimage *fImage;
	byte *fBuffer;
	float *fZBuffer;
};

#endif
