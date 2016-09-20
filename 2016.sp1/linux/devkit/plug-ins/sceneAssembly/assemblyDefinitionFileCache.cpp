//==============================================================================
// EXTERNAL DECLARATIONS
//==============================================================================

#include "assemblyDefinitionFileCache.h"

#include <sys/types.h>          // For stat()
#include <sys/stat.h>

#include <cassert>
#include <utility>


namespace {

//==============================================================================
// LOCAL DECLARATIONS
//==============================================================================

/*----- variables -----*/

AssemblyDefinitionFileCache fsTheInstance;

}

//==============================================================================
// PUBLIC FUNCTIONS
//==============================================================================

//==============================================================================
// CLASS AssemblyDefinitionFileCache::Timestamp
//==============================================================================

//------------------------------------------------------------------------------
//
AssemblyDefinitionFileCache::Timestamp::Timestamp(const MString& path)
    : fFileSize(-1)
{
    struct stat stat_buf;

    if (stat(path.asChar(), &stat_buf) != 0)
        return;

    fFileSize      = stat_buf.st_size;
    fMTime         = stat_buf.st_mtime;
}
    

//------------------------------------------------------------------------------
//
inline bool operator==(const AssemblyDefinitionFileCache::Timestamp& lhs,
                       const AssemblyDefinitionFileCache::Timestamp& rhs)
{
    return (lhs.fFileSize == rhs.fFileSize &&
            lhs.fMTime    == rhs.fMTime    );
}


//==============================================================================
// CLASS AssemblyDefinitionFileCache::Entry
//==============================================================================

//------------------------------------------------------------------------------
//
void AssemblyDefinitionFileCache::Entry::Deleter::operator()(Entry* entry)
{
    AssemblyDefinitionFileCache& cache =
        AssemblyDefinitionFileCache::getInstance();
    Entries::iterator it = cache.fEntries.find(entry->fDefnFile);

    // The deleter might be invoked after AssemblyDefinitionFileCache::get()
    // has removed the entry from the cache if the file timestamp ended-up
    // being different. We therefore have to explicitly test for that
    // possibility.
    if (it != cache.fEntries.end()) {
        // No assemblyReference node is referring to that entry anymore. We
        // should therefore remove the now expired pointer to this entry from
        // the cache. But, we have to be careful. It is possible that the cache
        // entry has been replaced with one with a newer timestamp. This case
        // is detected by double-checking that the pointer has expired before
        // attempting to erase the entry from the cache.
        if (it->second.expired()) {
            cache.fEntries.erase(it);
        }
    }
    
    delete entry;
}


//==============================================================================
// CLASS AssemblyDefinitionFileCache
//==============================================================================

//------------------------------------------------------------------------------
//
AssemblyDefinitionFileCache& AssemblyDefinitionFileCache::getInstance()
{
    return fsTheInstance;
}


//------------------------------------------------------------------------------
//
AssemblyDefinitionFileCache::EntryPtr
AssemblyDefinitionFileCache::get(const MString& defnFile)
{
    using namespace std::rel_ops;

    Entries::iterator it = fEntries.find(std::string(defnFile.asChar()));
    if (it == fEntries.end()) return EntryPtr();

    EntryPtr entry(it->second);
    if (!entry) return EntryPtr();

    const Timestamp timestamp(defnFile);
    if (entry->getTimestamp() != timestamp) {
        // The file has changed since it was last read and cached. Get
        // rid of the stale cache entry and read it again.
        fEntries.erase(it);
        return EntryPtr();
    }

    return entry;
}
    

//------------------------------------------------------------------------------
//
AssemblyDefinitionFileCache::EntryPtr
AssemblyDefinitionFileCache::insert(
    const MString&              defnFile,
    const RepCreationArgsList&  repCreationArgsList)
{
    const std::string defnFileString(defnFile.asChar());
    EntryPtr entry(
        new Entry(defnFileString,
                  Timestamp(defnFile),
                  repCreationArgsList),
        Entry::Deleter());
        
#ifndef NDEBUG
    std::pair<Entries::iterator, bool> result =
#endif            
        fEntries.insert(std::make_pair(defnFileString, entry));
    assert(result.second);

    return entry;
}
    

//-
// Copyright 2015 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
//+

