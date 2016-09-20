#ifndef _gpuCacheFrustum_h_
#define _gpuCacheFrustum_h_

//-
//**************************************************************************/
// Copyright 2012 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include <maya/MMatrix.h>
#include <maya/MBoundingBox.h>

#include <cassert>

namespace GPUCache {
    
//==============================================================================
// CLASS Frustum
//==============================================================================

// Inspired from "Optimized View Frustum Culling Algorithms", Ulf
// Assarsson and Tomas Moller, Chalmers University, March 1999
class Frustum
{
public:
    
    /*----- typedefs and enums -----*/
    
    enum ClippingResult {
        kIntersectsLeft     = (1),
        kIntersectsRight    = (1 << 1),
        kIntersectsBottom   = (1 << 2),
        kIntersectsTop      = (1 << 3),
        kIntersectsNear     = (1 << 4),
        kIntersectsFar      = (1 << 5),

        kIntersectsMask     = kIntersectsLeft | kIntersectsRight |
                              kIntersectsBottom | kIntersectsTop |
                              kIntersectsNear | kIntersectsFar,

        kOutside            = (1 << 6),
        kInside             = (1 << 7),

        kIntersects         = kIntersectsMask,
        kUnknown            = kIntersectsMask,
    };

    enum DrawAPI {
        kOpenGL,
        kDirectX,
    };

    /*----- member functions -----*/
    
    Frustum(MMatrix worldViewProjInvMatrix, DrawAPI api = kOpenGL);

    ClippingResult test(const MBoundingBox& bbox, 
        const ClippingResult& parentResult = kUnknown) const
    {
        // kOutside and kInside should be processed before calling test()
        assert(!(parentResult & (kOutside | kInside)));

        const MPoint pmin = bbox.min();
        const MPoint pmax = bbox.max();
            
        int result    = 0;
        int intersect = 1;

        for (int i=kFirstPlane; i<=kLastPlane; ++i,intersect=intersect<<1) {
            // Check the parent bounding box against the clipping plane.
            // We don't need to check the clipping plane if
            // the parent bounding box is fully inside the clipping plane,
            if (parentResult & intersect) {
                // Parent bounding box intersects with the clipping plane
                // Test this bounding box against the clipping plane
                ClippingResult res = planes[i].test(pmin, pmax);

                // Outside the clipping plane, return
                if (res == kOutside) {
                    return kOutside;
                }

                // Intersect the clipping plane, set the bit
                if (res == kIntersects) {
                    result = result | intersect;
                }
            }
        }

        // The bounding box does not intersect with any of the clipping plane.
        if (!(result & kIntersectsMask)) {
            return kInside;
        }

        // The bounding box intersect with one of the clipping plane.
        return (ClippingResult)result;
    }        
        
private:
        
    /*----- typedefs and enums -----*/
    
    enum PlaneId {
        kLeft,
        kRight,
        kBottom,
        kTop,
        kNear,
        kFar,

        kFirstPlane = kLeft,
        kLastPlane = kFar
    };

    /*----- classes -----*/
    
    class Plane 
    {
    public:
            
        // Setup plane given three non-colinear points.
        //
        // We also pass in a vertex that lies on the possitive
        // side of the plane. We can't rely on the winding
        // ordering of the vertices because the
        // worldViewProjInvMatrix might contain reflections along
        // arbitray axis.
        //
        // Note that we could check the orientation of only half
        // the planes, but it seem simpler to simply check each
        // planes.
        void init(const MPoint& p0,
                  const MPoint& p1,
                  const MPoint& p2,
                  const MPoint& opp)
        {
            MVector u = p2 - p1;
            MVector v = p0 - p1;
            MVector n = (u ^ v).normal();
            a = n.x;
            b = n.y;
            c = n.z;
            d = -(n.x*p1.x + n.y*p1.y + n.z*p1.z);

            if (distance(opp) < 0.0f) {
                a = -a; b = -b; c = -c; d = -d;
            }
        }

        double distance(double x, double y, double z) const
        {
            return a*x + b*y + c*z + d;
        }
            
        double distance(const MPoint& p) const
        {
            return distance(p.x, p.y, p.z);
        }

        ClippingResult test(const MPoint& pmin,
                            const MPoint& pmax) const
        {
            const bool sa = a > 0.0;
            const bool sb = b > 0.0;
            const bool sc = c > 0.0;

            // Test the p-vertex
            if (distance(sa ? pmax.x : pmin.x,
                         sb ? pmax.y : pmin.y,
                         sc ? pmax.z : pmin.z) < 0.0)
                return kOutside;

            // Test the n-vertex
            if (distance(sa ? pmin.x : pmax.x,
                         sb ? pmin.y : pmax.y,
                         sc ? pmin.z : pmax.z) > 0.0)
                return kInside;

            return kIntersects;
        }

        void print(const char* name,
                   const MPoint& op1,
                   const MPoint& op2,
                   const MPoint& op3,
                   const MPoint& op4);
                    
    private:
            
        // Plane equation a*x + b*y + c*z + d = 0
        // ||a,b,c|| == 1.0
        double a, b, c, d;
    };
    
    /*----- data members -----*/
    
    Plane planes[6];
};

    
} // namespace GPUCache

#endif
