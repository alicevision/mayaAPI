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

#ifndef _SLOPESHADERNODE_H_
#define _SLOPESHADERNODE_H_

#include <maya/MPxNode.h>

class slopeShaderNode : public MPxNode
{
public:
                  slopeShaderNode();
  virtual         ~slopeShaderNode();

  virtual MStatus compute( const MPlug&, MDataBlock& );
  virtual void    postConstructor();

  static  void *  creator();
  static  MStatus initialize();

  static  MTypeId id;

protected:
  static MObject aAngle;
  static MObject aColor1;
  static MObject aColor2;
  static MObject aTriangleNormalCamera;
  static MObject aOutColor;
  static MObject aMatrixEyeToWorld;
  static MObject aDirtyShaderAttr;
};

#endif //_SLOPESHADERNODE_H_

