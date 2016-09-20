#ifndef _assemblyDefinitionFileCache_h_
#define _assemblyDefinitionFileCache_h_

/*==============================================================================
 * EXTERNAL DECLARATIONS
 *============================================================================*/

#include <maya/MString.h>

#include <ciso646>
#if defined( _WIN32 ) || defined( _LIBCPP_VERSION )
#include <memory>
#include <unordered_map>
#define ADSTD std
#else
#include <tr1/memory>
// Note:
// We found tr1::unordered_map::begin() is O(n) on Linux. 
// The C++11 standard clearly states that begin() must be O(1) 
// for all std containers This means that the platform 
// implementation of tr1::unordered_map is not C++11 compliant yet.
// If meet performance issue with tr1::unordered_map, suggest to 
// use boost library, which is cross-platform and boost::unordered_map
// is C++11 compliant.
#include <tr1/unordered_map>
#define ADSTD std::tr1
#endif

#include <string>
#include <vector>

#include <time.h>               // For time_t.


/*==============================================================================
 * CLASS AssemblyDefinitionFileCache
 *============================================================================*/

// Cache the content of definition files.
//
// This is useful when several assemblyReference nodes are referring
// to the same assembly definition file. In turns out that the process
// of reading the definition file is costly (MEL interpretation
// overhead, Maya scene file common information, post scene read
// callbacks, etc.). By caching the content of the definition file, we
// can avoid paying that cost over and over again.
class AssemblyDefinitionFileCache 
{
private:

    /*----- types -----*/
    
   	// Information used to determine if a file has changed since it
   	// was last read or accessed. We are currently using the
   	// combination of the file size and the last modification
   	// time. Alternatively, a cryptographic checksum (MD5, Murmur3,
   	// etc.) could also have been used.
    class Timestamp 
    {
    public:
        Timestamp(const MString& path);

        MInt64   fFileSize;  // File size.
        time_t   fMTime;     // Last modification time.
    };

    friend inline bool operator==(const Timestamp& lhs,
                                  const Timestamp& rhs);

public:

    /*----- types -----*/

    // Information necessary to create a given representation.
    class RepresentationCreationArgs 
    {
    public:

        /*----- member functions -----*/

        RepresentationCreationArgs(const MString& name,
                                   const MString& type,
                                   const MString& label,
                                   const MString& data)
            : fName(name), fType(type), fLabel(label), fData(data)
        {}

        const MString& getName()  const { return fName;  }
        const MString& getType()  const { return fType;  }
        const MString& getLabel() const { return fLabel; }
        const MString& getData()  const { return fData;  }
    
    private:

        /*----- data members -----*/
    
        MString fName;
        MString fType;
        MString fLabel;
        MString fData;
    };

    typedef std::vector<RepresentationCreationArgs> RepCreationArgsList;

    class Entry 
    {
    public:
        
        /*----- member functions -----*/

        const RepCreationArgsList& getRepCreationArgsList() const
        { return fRepCreationArgsList; }


    private:
        friend class AssemblyDefinitionFileCache;
        
        /*----- types -----*/

        struct Deleter { void operator()(Entry* entry); };
        

        /*----- member functions -----*/

        // Note that it would have been nice to be able to use C++11
        // move semantic to avoid copying the repCreationArgsList
        // here.
        Entry(const std::string&         defnFile,
              const Timestamp&           timestamp,
              const RepCreationArgsList& repCreationArgsList)
            : fDefnFile(            defnFile),
              fTimestamp(           timestamp),
              fRepCreationArgsList( repCreationArgsList)
        {}

        const Timestamp& getTimestamp() const
        { return fTimestamp; }

        
        /*----- data members -----*/

        std::string         fDefnFile;
        Timestamp           fTimestamp;
        RepCreationArgsList fRepCreationArgsList;
    };

    
    typedef ADSTD::shared_ptr<Entry> EntryPtr;


    /*----- static member functions -----*/

    static AssemblyDefinitionFileCache& getInstance();


    /*----- member functions -----*/

    // Look-up the cache for an entry matching the corresponding
    // definition file. Returns a null pointer if no matching entries
    // are found.
    EntryPtr get(const MString& defnFile);

    // Insert a new entry into the cache. The entry is for the given
    // definition file containing the given list of
    // representations. Returns pointer to the newly created
    // entry. The caller is responsible for first calling get() to
    // ensure that no matching entry exists for the given definition
    // file before attempting to insert the entry into the cache.
    EntryPtr insert(const MString&              defnFile,
                    const RepCreationArgsList&  repCreationArgsList);

private:

    /*----- types -----*/
    
    typedef ADSTD::unordered_map<std::string, ADSTD::weak_ptr<Entry> > Entries;

    /*----- data members -----*/
    
    Entries fEntries;
};

#endif

//-
//*****************************************************************************
// Copyright 2013 Autodesk, Inc.
// All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
