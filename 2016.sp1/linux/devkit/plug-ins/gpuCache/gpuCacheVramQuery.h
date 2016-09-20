#ifndef _gpuCacheVramQuery_h_
#define _gpuCacheVramQuery_h_

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

#include <maya/MTypes.h>
#include <maya/MString.h>


namespace GPUCache {

/*==============================================================================
 * CLASS VramQuery
 *============================================================================*/

// Helper class used to query the dedicated video memory

class VramQuery
{
public:

    /*----- static member functions -----*/

    static MUint64 queryVram();

    // Returns whether the graphic card of the host computer is an
    // nVidia GeForce Graphic Card.
    static bool    isGeforce();

    // Returns whether the graphic card of the host computer is an
    // nVidia Quadro Graphic Card.
    static bool    isQuadro();

    // Returns the manufacturer of the graphic card
    static const MString& manufacturer();

    // Returns the model of the graphic card
    static const MString& model();

    // Returns the driver version.
    // Currently, this function only works on windows NVIDIA/AMD
    static void    driverVersion(int version[3]);

private:

    /*----- member functions -----*/

    // Forbidden and not implemented.
    VramQuery(const VramQuery&);
    const VramQuery& operator=(const VramQuery&);

private:

    /*----- member functions -----*/

    // Vram query functions for different platforms
#if defined(_WIN32)
    static void queryVramAndDriverWMI(MUint64& vram, int driverVersion[3], MString& manufacturer, MString& model);
    static MUint64 queryVramDXGI();
#elif defined(__APPLE__) || defined(__MACH__)
    static void queryVramAndDriverMAC(MUint64& vram, int driverVersion[3], MString& manufacturer, MString& model);
#else
    static void queryVramAndDriverXORG(MUint64& vram, int driverVersion[3], MString& manufacturer, MString& model);
#endif
    static MUint64 queryVramOGL();

    static bool    isGeforceOGL();
    static bool    isQuadroOGL();

private:

    /*----- static member functions -----*/

    static const VramQuery& getInstance();


    /*----- member functions -----*/

    VramQuery();

    /*----- data members -----*/

    MUint64 fVram;
    int     fDriverVersion[3];
    bool    fIsGeforce;
    bool    fIsQuadro;
    MString fManufacturer;
    MString fModel;
};

}

#endif
