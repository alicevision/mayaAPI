#ifndef _gpuCacheCmd_h_
#define _gpuCacheCmd_h_

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

#include <maya/MPxCommand.h>

#include <maya/MArgDatabase.h>
#include <maya/MDagModifier.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MTime.h>

#include <cassert>
#include <string>
#include <set>
#include <vector>
#include <map>

namespace GPUCache {

///////////////////////////////////////////////////////////////////////////////
//
// gpuCache MEL command
//
// Creates one or more cache files on disk to store attribute data for
// a span of frames. 
//
// FIXME: What is the proper way to document this command so that the
// documentation appears in the Maya On-line Help Documentation ?
//
//
////////////////////////////////////////////////////////////////////////////////


class Command : public MPxCommand
{
public:

    /*----- member functions -----*/

    static MSyntax cmdSyntax();
    static void* creator();

    /*----- member functions -----*/

    Command();
    virtual ~Command();

    virtual MStatus doIt( const MArgList& );
    virtual bool isUndoable() const;
    virtual bool hasSyntax() const;

private:

    /*----- types and enumerations ----*/

    enum Mode {
        kCreate = 1,
        kEdit   = 2,
        kQuery  = 4
    };


    /*----- classes -----*/

    // Helper class for holding command-line flags with one flag.
    template <class T, Mode ValidModes>
    class OptFlag
    {
    public:
        OptFlag() : fIsSet(false), fArg() {}

        unsigned int parse(const MArgDatabase& argDb, const char* name)
		//
		// Return Value:
		//		Used to count number of flags set.
		//
		//		1, if flag was set
		//		0, otherwise
		//
        {
            MStatus status;
            fIsSet = argDb.isFlagSet(name, &status);
            assert(status == MS::kSuccess);
            
            status = argDb.getFlagArgument(name, 0, fArg);
            fIsArgValid = status == MS::kSuccess;

			return fIsSet ? 1 : 0;
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

        unsigned int parse(const MArgDatabase& argDb, const char* name)
		//
		// Return Value:
		//		Used to count number of flags set.
		//
		//		1, if flag was set
		//		0, otherwise
		//
        {
            MStatus status;
            fIsSet = argDb.isFlagSet(name, &status);
            assert(status == MS::kSuccess);

			return fIsSet ? 1 : 0;
        }

        bool isModeValid(const Mode currentMode)
        {
            return !fIsSet || ((currentMode & ValidModes) != 0);
        }
        
        bool isSet()        const { return fIsSet; }
        
    private:
        bool        fIsSet;
    };

    // Structure for keeping xform bakers
    class SourceAndXformNode;
    typedef std::vector<SourceAndXformNode> SourceAndXformMapping;

    
    /*----- member functions -----*/

    void AddHierarchy(const MDagPath&                      dagPath,
                      std::map<std::string, int>*          idMap,
                      std::vector<MObject>*                sourceNodes,
                      std::vector<std::vector<MDagPath> >* sourcePaths,
                      std::vector<MObject>*                gpuCacheNodes);

    bool AddSelected(const MSelectionList&                objects,
                     std::vector<MObject>*                sourceNodes,
                     std::vector<std::vector<MDagPath> >* sourcePaths,
                     std::vector<MObject>*                gpuCacheNodes);

    MStatus doCreate(const std::vector<MObject>&                sourceNodes,
                     const std::vector<std::vector<MDagPath> >& sourcePaths,
                     const MSelectionList& objects);

    MStatus doQuery( const std::vector<MObject>& gpuCacheNodes) const;

    MStatus doEdit(  const std::vector<MObject>& gpuCacheNodes);

    MStatus doBaking(const std::vector<MObject>&                sourceNodes,
                     const std::vector<std::vector<MDagPath> >& sourcePaths,
                     MTime                       startTime,
                     MTime                       endTime,
                     MTime                       simulationRate,
                     int                         samplingRate);

    void showStats(const std::vector<MObject>& gpuCacheNodes,
                   MStringArray& result) const;
    void showGlobalStats(MStringArray& result) const;
    void dumpHierarchy(const std::vector<MObject>& gpuCacheNodes,
                       MStringArray& result) const;
    MStatus dumpHierarchyToFile(const std::vector<MObject>& gpuCacheNodes,
                                const MFileObject& file) const;
    void showAnimTimeRange(const std::vector<MObject>& gpuCacheNodes,
                           MDoubleArray& result) const;
    void refresh(const std::vector<MObject>& gpuCacheNodes);
	void refreshAll();
    void refreshSettings();

	void listFileEntries();
	void listShapeEntries();


    /*----- data members -----*/

    // Command line arguments
    Mode                                    fMode;
    OptFlag<MString, Mode(kCreate)>         fDirectoryFlag;
    OptFlag<MString, Mode(kCreate)>         fFileNameFlag;
    OptFlag<bool,    Mode(kCreate)>         fSaveMultipleFilesFlag;
    OptFlag<MString, Mode(kCreate)>         fFilePrefixFlag;
    OptFlag<MString, Mode(kCreate)>         fClashOptionFlag;
    OptFlag<void,    Mode(kCreate)>         fOptimizeFlag;
    OptFlag<unsigned,Mode(kCreate)>         fOptimizationThresholdFlag;
    OptFlag<MTime,   Mode(kCreate)>         fStartTimeFlag;
    OptFlag<MTime,   Mode(kCreate)>         fEndTimeFlag;
    OptFlag<MTime,   Mode(kCreate)>         fSimulationRateFlag;
    OptFlag<int,     Mode(kCreate)>         fSampleMultiplierFlag;
    OptFlag<int,     Mode(kCreate)>         fCompressLevelFlag;
    OptFlag<MString, Mode(kCreate)>         fDataFormatFlag;
    OptFlag<MString, Mode(kCreate|kQuery)>  fShowFailedFlag;
    OptFlag<void,    Mode(kQuery)>          fShowStats;
    OptFlag<void,    Mode(kQuery)>          fShowGlobalStats;
    OptFlag<MString, Mode(kQuery)>          fDumpHierarchy;
    OptFlag<void,    Mode(kQuery)>          fAnimTimeRangeFlag;
    OptFlag<void,    Mode(kQuery)>          fGpuManufacturerFlag;
    OptFlag<void,    Mode(kQuery)>          fGpuModelFlag;
    OptFlag<void,    Mode(kQuery)>          fGpuDriverVersion;
    OptFlag<void,    Mode(kQuery)>          fGpuMemorySize;
    OptFlag<void,    Mode(kCreate)>         fAllDagObjectsFlag;
    OptFlag<void,    Mode(kEdit)>           fRefreshFlag;
    OptFlag<void,    Mode(kCreate)>         fRefreshAllFlag;
	OptFlag<void,    Mode(kCreate)>         fListFileEntriesFlag;
	OptFlag<void,    Mode(kCreate)>         fListShapeEntriesFlag;
    OptFlag<void,    Mode(kEdit)>           fRefreshSettingsFlag;
    OptFlag<void,    Mode(kQuery)>          fWaitForBackgroundReadingFlag;
    OptFlag<void,    Mode(kCreate)>         fWriteMaterials;
    OptFlag<void,    Mode(kCreate)>			fUVsFlag;
    OptFlag<void,    Mode(kCreate)>         fOptimizeAnimationsForMotionBlurFlag;
    OptFlag<void,    Mode(kCreate)>         fUseBaseTessellationFlag;
    OptFlag<void,    Mode(kCreate|kEdit)>   fPromptFlag;
};

} // namespace GPUCache

#endif
