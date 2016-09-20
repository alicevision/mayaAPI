#ifndef _adskRepresentationCmd_h_
#define _adskRepresentationCmd_h_

#include <maya/MPxCommand.h>
#include <maya/MArgDatabase.h>

#include <cassert>

/*==============================================================================
 * CLASS AdskRepresentationCmd
 *============================================================================*/

class AdskRepresentationCmd : public MPxCommand {

public: 

   /*----- static member functions -----*/

	static MSyntax cmdSyntax();

    /// \return Name string for command plugin registry.
    static const char* name();
	static void* creator();

   /*----- member functions -----*/
      
	AdskRepresentationCmd();
	virtual ~AdskRepresentationCmd();

	virtual MStatus doIt( const MArgList& );

private:

    /*----- types and enumerations ----*/

    enum Mode {      
        kEdit   = 1 << 0,
        kQuery  = 1 << 1
    };

    /*----- classes -----*/

    // Helper class for holding command-line flags with one flag.
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

    // Specialization for flags with no argument.
    template <Mode ValidModes>
    class OptFlag<void, ValidModes>
    {
    public:
        OptFlag() : fIsSet(false) {}

        void parse(const MArgDatabase& argDb, const char* name)
        {
            MStatus status;
            fIsSet = argDb.isFlagSet(name, &status);
            assert(status == MS::kSuccess);
        }

        bool isModeValid(const Mode currentMode)
        {
            return !fIsSet || ((currentMode & ValidModes) != 0);
        }
        
        bool isSet()        const { return fIsSet; }
        
    private:
        bool        fIsSet;
    };
	
	
    /*----- member functions -----*/
  
    MStatus doEdit(const MString&);
    MStatus doQuery(const MString&);
	
	bool needObjectArg() const;

    /*----- data members -----*/

    // Command line arguments
    Mode										fMode;	
    OptFlag<MString, Mode(kEdit|kQuery)>        fTypeLabelFlag;
	OptFlag<MString, Mode(kEdit|kQuery)>        fAERepresentationProcFlag;
	OptFlag<MString, Mode(kQuery)>              fListRepTypesFlag;

};


#endif

//-
//*****************************************************************************
// Copyright 2013 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
