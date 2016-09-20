//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include "gpuCacheFrustum.h"

#include <stdio.h>
#include <cassert>


#ifdef DEBUG
#define GPU_CACHE_DEBUG_FRUSTUM 0
#else
#define GPU_CACHE_DEBUG_FRUSTUM 0
#endif
    
namespace GPUCache {

//==============================================================================
// CLASS Frustum
//==============================================================================

//--------------------------------------------------------------------------
//
void Frustum::Plane::print(
    const char* name,
    const MPoint& op1,
    const MPoint& op2,
    const MPoint& op3,
    const MPoint& op4) 
{
    const double d1 = distance(op1);
    const double d2 = distance(op2);
    const double d3 = distance(op3);
    const double d4 = distance(op4);
                
    fprintf(
        stderr,
        "%8s = (%10lf, %10lf, %10lf, %10lf) -- %10lf, %10lf, %10lf, %10lf\n",
        name, a, b, c, d, d1, d2, d3, d4);

    assert(d1 > 0.0f);
    assert(d2 > 0.0f);
    assert(d3 > 0.0f);
    assert(d4 > 0.0f);
}


//--------------------------------------------------------------------------
//
Frustum::Frustum(MMatrix worldViewProjInvMatrix, DrawAPI api /* = kOpenGL */)
{
    // Useful constant for reducing the frustrum size for
    // debugging purposes.
    const float one  = 1.0f;
    const float zNear = (api == kOpenGL) ? -1.0f : 0.0f;
    const float zFar  = 1.0f;

    // Project back the vertices of the view frustem from NDC to
    // object space. This will allows us to efficiently test a
    // whole list of bounding boxes represented in object space.
    //
    // Only 7 of the 8 vertices are necessary!
    MPoint ltn = MPoint(-one, +one, zNear) * worldViewProjInvMatrix;
    MPoint rtn = MPoint(+one, +one, zNear) * worldViewProjInvMatrix;
    MPoint lbn = MPoint(-one, -one, zNear) * worldViewProjInvMatrix;
    MPoint rbn = MPoint(+one, -one, zNear) * worldViewProjInvMatrix;

    MPoint ltf = MPoint(-one, +one, zFar) * worldViewProjInvMatrix;
    MPoint rtf = MPoint(+one, +one, zFar) * worldViewProjInvMatrix;
    MPoint lbf = MPoint(-one, -one, zFar) * worldViewProjInvMatrix;

    ltn.cartesianize();
    rtn.cartesianize();
    lbn.cartesianize();
    rbn.cartesianize();

    ltf.cartesianize();
    rtf.cartesianize();
    lbf.cartesianize();
        

    planes[kLeft].init(   ltf, ltn, lbn,   rbn);
    planes[kRight].init(  rtf, rtn, rbn,   lbn);
        
    planes[kTop].init(    ltf, ltn, rtn,   lbn);
    planes[kBottom].init( lbf, lbn, rbn,   ltn);
        
    planes[kNear].init(   lbn, ltn, rtn,   lbf);
    planes[kFar].init(    lbf, ltf, rtf,   lbn);

#if GPU_CACHE_DEBUG_FRUSTUM
    fprintf(stderr, "ltn = (%lf, %lf, %lf)\n", ltn.x, ltn.y, ltn.z);
    fprintf(stderr, "rtn = (%lf, %lf, %lf)\n", rtn.x, rtn.y, rtn.z);
    fprintf(stderr, "lbn = (%lf, %lf, %lf)\n", lbn.x, lbn.y, lbn.z);
    fprintf(stderr, "rbn = (%lf, %lf, %lf)\n", rbn.x, rbn.y, rbn.z);
                            
    fprintf(stderr, "ltf = (%lf, %lf, %lf)\n", ltf.x, ltf.y, ltf.z);
    fprintf(stderr, "rtf = (%lf, %lf, %lf)\n", rtf.x, rtf.y, rtf.z);
    fprintf(stderr, "lbf = (%lf, %lf, %lf)\n", lbf.x, lbf.y, lbf.z);

    MPoint rbf = MPoint(+one, -one, +oneZ) * worldViewProjInvMatrix;
    rbf.cartesianize();
    fprintf(stderr, "rbf = (%lf, %lf, %lf)\n", rbf.x, rbf.y, rbf.z);

    fprintf(stderr, "\n");

    planes[kLeft].print(  "left",   rtn, rbn, rtf, rbf);
    planes[kRight].print( "right",  ltn, lbn, ltf, lbf);
    planes[kTop].print(   "top",    lbn, lbf, rbn, rbf);
    planes[kBottom].print("bottom", ltn, ltf, rtn, rtf);
    planes[kNear].print(  "near",   ltf, lbf, rtf, rbf);
    planes[kFar].print(   "far",    ltn, lbn, rtn, rbn);
    fprintf(stderr, "\n");
#endif                
}

}
