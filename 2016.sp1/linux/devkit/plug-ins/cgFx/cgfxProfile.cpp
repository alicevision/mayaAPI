//-
// ==========================================================================
// Copyright 2011 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+
//

#include <maya/MCommonSystemUtils.h>
#include <maya/MGlobal.h>
#include <maya/MStringArray.h>

#include "cgfxProfile.h"

#include <Cg/cgGL.h>

// List of Cg profiles.
//
// These are tupples of short name, vertex, geometry and fragment
// profiles that we will query to see if they are supported on the
// given platform. They are provided in order of expected rendering
// performance.
//
// We have to provide an explicit list of profiles because there seems
// to be no way to query that list using the Cg runtime API. It is
// missing a way to find out which combination of vertex, geometry and
// fragment profile goes together.
static const char* CgGLProfileList[][4] = {
{"gp5",  "gp5vp",  "gp5gp", "gp5fp"},
{"gp4",  "gp4vp",  "gp4gp", "gp4fp"},
{"glsl", "glslv",  "glslg", "glslf"},
{"NV4X", "vp40",   "",      "fp40"},
{"arb1", "arbvp1", "",      "arbfp1"},
{"NV3X", "vp30",   "",      "fp30"},
{"NV2X", "vp20",   "",      "fp20"}
};


// We use the OpenGL texture coordinate orientation be default.
cgfxProfile::TexCoordOrientation cgfxProfile::sTexCoordOrientation =
    cgfxProfile::TEXCOORD_OPENGL;

cgfxProfile*       cgfxProfile::sProfileList = NULL;
const cgfxProfile* cgfxProfile::sBestProfile = NULL;

void cgfxProfile::initialize()
{
    // Determine the texture coordinate orientation.
    {
        MStatus status;
        MString str =
            MCommonSystemUtils::getEnv("MAYA_TEXCOORD_ORIENTATION", &status);

        if (status) 
        {
            if (str == "OPENGL" || str == "")
                sTexCoordOrientation = TEXCOORD_OPENGL;
            else if (str == "DIRECTX")
                sTexCoordOrientation = TEXCOORD_DIRECTX;
            else {
                MString es = "cgfxShader : The value ";
                es += str;
                es += " of the MAYA_TEXCOORD_ORIENTATION environment variable is unsupported. ";
                es += "Supported values are OPENGL and DIRECTX";
                MGlobal::displayWarning( es );
            }
        }
    }
    
    // Determine the list of available Cg profiles.
    {
        const int nbKnownProfiles = sizeof(CgGLProfileList)/sizeof(const char*[4]);
        cgfxProfile** currentEntry = &sProfileList;
        
        for (int i=0; i<nbKnownProfiles; ++i) {
            const char* shortName       = CgGLProfileList[i][0];
            const char* vtxProfileName  = CgGLProfileList[i][1];
            const char* geomProfileName = CgGLProfileList[i][2];
            const char* fragProfileName = CgGLProfileList[i][3];

            CGprofile vtxProfile  = cgGetProfile(vtxProfileName);
            CGprofile geomProfile = cgGetProfile(geomProfileName);
            CGprofile fragProfile = cgGetProfile(fragProfileName);

            // The geometry profile support is optional.
            if (
                cgGLIsProfileSupported(vtxProfile) &&
                (geomProfileName[0] == '\0' || cgGLIsProfileSupported(geomProfile)) &&
                cgGLIsProfileSupported(fragProfile)
            ) {
                *currentEntry = new cgfxProfile(
                    shortName, vtxProfile, geomProfile, fragProfile);
                currentEntry = &(*currentEntry)->fNext;
            }
        }
    }

    // Set the shader profile strings to the latest supported one.
    if (sProfileList == NULL) {
        sBestProfile = NULL;
    }
    else if (sProfileList->fNext == NULL) {
        // Only one profile is available we must chose it.
        sBestProfile = sProfileList;
    }
    else {
        if (sProfileList->fName == "glsl") {
            // The GLSL profile does not handle properly the semantic
            // annotations on top-level uniform variable
            // declarations. We therefore try to avoid it by default.
            sBestProfile = sProfileList->fNext;
        }
        else {
            sBestProfile = sProfileList;
        }
    }
}

void cgfxProfile::uninitialize()
{
    cgfxProfile* profile = sProfileList;
    while (profile) {
      cgfxProfile* nextProfile = profile->fNext;
      delete profile;
      profile = nextProfile;
    }
    sProfileList = 0;
    sBestProfile = 0;    
}

MStringArray cgfxProfile::getProfileList()
{
    MStringArray result;
    cgfxProfile* profile = sProfileList;
    while (profile) {
        result.append(profile->fName);
        profile = profile->fNext;
    }

    return result;
}

const cgfxProfile* cgfxProfile::getProfile(MString profileName)
{
    if (profileName == "") {
        // Early out for a common case.
        return NULL;
    }
    else {
        // Search for requested profile.
        const cgfxProfile* profile = sProfileList;
        while (profile) {
            if (profile->fName == profileName) {
                return profile;
            }
            profile = profile->fNext;
        }
    }

    return NULL;
}

cgfxProfile::cgfxProfile(
  MString name, CGprofile vtx, CGprofile geom, CGprofile frag
) 
  : fName(name),
    fVtx(vtx),
    fGeom(geom),
    fFrag(frag),
    fNext(NULL)
{}


cgfxProfile::cgfxProfile(MString name, CGpass pass)
  : fName(name),
    fVtx(CG_PROFILE_UNKNOWN),
    fGeom(CG_PROFILE_UNKNOWN),
    fFrag(CG_PROFILE_UNKNOWN),
    fNext(NULL)
{
    CGprogram vp = cgGetPassProgram(pass, CG_VERTEX_DOMAIN);
    if (vp != NULL) {
        fVtx = cgGetProgramProfile(vp);
    }

    CGprogram gp = cgGetPassProgram(pass, CG_GEOMETRY_DOMAIN);
    if (gp != NULL) {
        fGeom = cgGetProgramProfile(gp);
    }

    CGprogram fp = cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN);
    if (fp != NULL) {
        fFrag = cgGetProgramProfile(fp);
    }
}

cgfxProfile::~cgfxProfile()
{}

bool cgfxProfile::isSupported() const
{
    return
        (fVtx == CG_PROFILE_UNKNOWN  || cgGLIsProfileSupported(fVtx)) &&
        (fGeom == CG_PROFILE_UNKNOWN || cgGLIsProfileSupported(fGeom)) &&
        (fFrag == CG_PROFILE_UNKNOWN || cgGLIsProfileSupported(fFrag));
}


