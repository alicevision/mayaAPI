#ifndef _MColorManagementUtilities
#define _MColorManagementUtilities
//
//-
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// ===========================================================================
//+
//
// ****************************************************************************

#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES

#include <maya/MStatus.h>

// ****************************************************************************
// DECLARATIONS

class MString;

// ****************************************************************************
// CLASS DECLARATION (MColorManagementUtilities)
//! \ingroup OpenMayaRender
//! \brief Utilities for obtaining color management information.
class OPENMAYARENDER_EXPORT MColorManagementUtilities
{
public:
    //! \brief Utility class that retrieves the opaque data containing the color
    //!        transform information needed to render the scene.
    //!
    //!  This class manages internally a buffer that contains data about color 
    //!  transformation used in the scene.   The buffer managed herein 
    //!  whose pointer is returned by the 'getData' method is only valid
    //!  during the lifetime of this object's instance.
    //!  
    class OPENMAYARENDER_EXPORT MColorTransformData 
    {
    public:
      MColorTransformData();
      ~MColorTransformData();

      const unsigned getSize() const;
      const void* getData() const;

    private:
      void *_impl;
    };

	static const char* className();

    static MStatus getColorTransformCacheIdForInputSpace(const MString& inputSpaceName, MString& transformId);

    static MStatus getColorTransformCacheIdForOutputTransform(MString& transformId);

    static bool isColorManagementEnabled();

    static bool isColorManagementAvailable();

private:
    ~MColorManagementUtilities();
#ifdef __GNUC__
	friend class shutUpAboutPrivateDestructors;
#endif
};

// ****************************************************************************

#endif /* __cplusplus */
#endif /* _MColorManagementUtilities */
