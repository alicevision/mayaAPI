#ifndef _adskSceneMetadata_h_
#define _adskSceneMetadata_h_

#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>

#include <cassert>
#include <memory>

// Forward declarations
namespace adsk
{
    namespace Data
    {
        class Structure;
        class Accessor;
        class Associations;
    }
}

/*==============================================================================
 * CLASS AdskSceneMetadataCmd
 *============================================================================*/

class AdskSceneMetadataCmd : public MPxCommand {

public: 

   /*----- static member functions -----*/

    static MSyntax cmdSyntax();

    /// \return Name string for command plug-in registry.
    static const char* name();
	static void* creator();

   /*----- member functions -----*/
      
	AdskSceneMetadataCmd();
	virtual ~AdskSceneMetadataCmd();

	virtual MStatus doIt( const MArgList& );

private:

    /*----- types and enumerations ----*/

    enum Mode {      
        kEdit   = 1 << 0,
        kQuery  = 1 << 1
    };

    /*----- classes -----*/

    // Helper class for holding command-line flags with one argument.
    template <class T, Mode ValidModes>
    class OptFlag
    {
    public:
        OptFlag() : fIsSet(false), fArg() {}

        void parse(const MArgDatabase& argDb, const char* name)
        {
            MStatus status;
            fIsSet = argDb.isFlagSet(name, &status);
            assert(status == MS::kSuccess);
            
            status = argDb.getFlagArgument(name, 0, fArg);
            fIsArgValid = status == MS::kSuccess;
        }

        bool isModeValid(const Mode currentMode)
        {
            return !fIsSet || ((currentMode & ValidModes) != 0);
        }
        
        bool isSet()        const { return fIsSet; }
        bool isArgValid()   const { return fIsArgValid; }
        const T& arg()      const { return fArg; }

        const T& arg(const T& defValue) const {
            if (isSet()) {
                assert(isArgValid());
                return fArg;
            }
            else {
                return defValue;
            }
        }
        
    private:
        bool        fIsSet;
        bool        fIsArgValid;
        T           fArg;
    };

    /*----- member functions -----*/
  
    MStatus doEdit ( MString fileName );
    MStatus doQuery( MString fileName );

    MStatus getMetadata( MString fileName );
    MStatus setMetadata( MString fileName );

    std::auto_ptr< adsk::Data::Accessor > getAccessorForScene( MString scenePath );
    MStatus getSceneAssociations( adsk::Data::Accessor &accessor, adsk::Data::Associations &out_associations );

    /*----- data members -----*/

    // Command line arguments
    Mode						            fMode;	        // The command mode (edit or query)
    OptFlag<MString, Mode(kEdit|kQuery)>    fChannelName;   // The channel name in which the metadata is written
	OptFlag<MString, Mode(kEdit)>           fData;          // The metadata to write (edit mode only)
};


#endif // _adskSceneMetadata_h_

//-
//*****************************************************************************
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
