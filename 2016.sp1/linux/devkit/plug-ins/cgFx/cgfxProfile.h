#ifndef _cgfxProfile_h_
#define _cgfxProfile_h_

//
// File: cgfxProfile.h
//
// Class representing the valid Cg profiles on a given platform.
//
//-
// ==========================================================================
// Copyright 2010 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include <maya/MString.h>
#include <Cg/cg.h>

class cgfxProfile 
{
public:

    // Initialize the list of supported profiles on the current
    // platform.
    static void initialize();
    static void uninitialize();

    // Initializes a profile with the default profile of a pass.
    cgfxProfile(MString name, CGpass pass);

    // Destructor
    ~cgfxProfile();

    // Texture coordinate orientation
    enum TexCoordOrientation {
        TEXCOORD_OPENGL,
        TEXCOORD_DIRECTX,
    };
    static TexCoordOrientation getTexCoordOrientation() {
        return sTexCoordOrientation;
    }
    
    // Get the list of the names of the supported profiles on the
    // current platform.
	static MStringArray getProfileList();

    // Get the profile matching a given name, or NULL if not found.
    static const cgfxProfile* getProfile(MString name);

    // Get the profile having the best performance on this platform.
    static const cgfxProfile* getBestProfile() { return sBestProfile; }

    // Get the name of the profile.
    MString getName() const { return fName; }

    // Get the Cg profiles.
    CGprofile getVertexProfile() const { return fVtx; }
    CGprofile getGeometryProfile() const { return fGeom; }
    CGprofile getFragmentProfile() const { return fFrag; }

    bool isSupported() const;
    
private:
    cgfxProfile(MString name, CGprofile vtx, CGprofile geom, CGprofile frag);

    // Prohibited and not implemented.
    cgfxProfile(const cgfxProfile&);
    const cgfxProfile& operator=(const cgfxProfile&);
    
    static TexCoordOrientation sTexCoordOrientation;

    static cgfxProfile*       sProfileList;
	static const cgfxProfile* sBestProfile;

    MString         fName;
    CGprofile       fVtx;
    CGprofile       fGeom;
    CGprofile       fFrag;
    cgfxProfile*    fNext;
};

#endif /* _cgfxProfile_h_ */
