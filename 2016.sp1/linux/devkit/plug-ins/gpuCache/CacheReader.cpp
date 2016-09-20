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

#include "CacheReader.h"
#include "gpuCacheShapeNode.h"
#include "gpuCacheUtil.h"

#include <list>
#include <vector>

#include <boost/bimap.hpp>
#include <boost/bimap/list_of.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/foreach.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/hashed_index.hpp>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>

#include <tbb/compat/condition_variable>
#include <tbb/mutex.h>
#include <tbb/task.h>

#define DURATION_SECONDS(x) (tbb::tick_count::interval_t(double(x)))
namespace tbb {
    using tbb::interface5::unique_lock;
    using tbb::interface5::condition_variable;
};

#include <iostream>
#ifdef _WIN32
    #include <stdio.h>
#else
    #include <sys/resource.h>
    #include <limits.h>
#endif

#include <maya/MFnDagNode.h>
#include <maya/MGlobal.h>
#include <maya/MViewport2Renderer.h>

using namespace GPUCache;


class GlobalReaderCache::Impl {
public:
    Impl(int initNumFileHandles)
        :  fMaxNumFileHandles(initNumFileHandles), fHitCount(0), fGetCount(0)
    {
        assert(fMaxNumFileHandles > 10);
    }

    ~Impl()
    {
    }

    boost::shared_ptr<CacheReader> getCacheReader(const MFileObject& file)
    {
        // Bimap: fileName -> ownershipCount with CacheReader
        MString resolvedFullName = file.resolvedFullName();
        std::string key = resolvedFullName.asChar();
        boost::shared_ptr<CacheReader> value;

        tbb::unique_lock<tbb::mutex> lock(fMutex);
        while (!value)
        {
            // look up the bimap
            const LeftIterator iter = fData.left.find(key);
            if (iter == fData.left.end()) {
                // miss
                // if the cache has reached its capacity, we try to
                // close the least-recent-used reader
                // the reader should not be in use
                if (fData.size() == (size_t)fMaxNumFileHandles) {
                    // try to close one reader
                    RightIterator leastUsed;
                    for (leastUsed = fData.right.begin();
                            leastUsed != fData.right.end(); ++leastUsed) {
                        if ((*leastUsed).get_right() == 0) {
                            // we have found a reader not in use
                            break;
                        }
                    }

                    if (leastUsed != fData.right.end()) {
                        // got one reader to close
                        fData.right.erase(leastUsed);
                    }
                }

                if (fData.size() < (size_t)fMaxNumFileHandles) {
                    // safe to insert a new reader
                    value = createReader(file);
                    fData.left[key] = 1;
                    fData.right.back().info = value;
                }
            }
            else {
                // hit cache, we move the reader to the end of
                // the LRU list
                fData.right.relocate(fData.right.end(),
                    fData.project_right(iter));
                value = (*iter).info;
                ++(*iter).get_right();
                ++fHitCount;
            }
            ++fGetCount;

            // failed to create a reader because all readers are currently
            // in use and the cache has reached its capacity
            // wait for some time and try again
            if (!value) {
                fCond.wait(lock);
            }
        }
        
        return value;
    }

    void increaseFileRef(const MFileObject& file)
    {
        MString resolvedFullName = file.resolvedFullName();
        std::string key = resolvedFullName.asChar();
        tbb::unique_lock<tbb::mutex> lock(fMutex);

        // look up the file ref count
        FileRefCountIterator fileRefCountIter = fFileRefCount.find(key);
        if (fileRefCountIter != fFileRefCount.end()) {
            // increase the file ref count
            ++(*fileRefCountIter).second;
        }
        else {
            // insert a new entry for file ref count
            fFileRefCount[key] = 1;
        }
    }

    void decreaseFileRef(const MFileObject& file)
    {
        MString resolvedFullName = file.resolvedFullName();
        std::string key = resolvedFullName.asChar();
        tbb::unique_lock<tbb::mutex> lock(fMutex);

        // look up the file ref count
        FileRefCountIterator fileRefCountIter = fFileRefCount.find(key);
        if (fileRefCountIter != fFileRefCount.end()) {
            // decrease the file ref count
            if (--(*fileRefCountIter).second == 0) {
                // file ref count reaches 0
                // purge this reader from cache since the reader won't
                // be referenced any more
                // the reader may already be closed because of the capacity
                fFileRefCount.erase(fileRefCountIter);
                LeftIterator iter = fData.left.find(key);
                if (iter != fData.left.end()) {
                    fData.left.erase(iter);
                }
            }
        }
    }

    boost::shared_ptr<CacheReader> acquireOwnership(const MFileObject& file)
    {
        // make sure the reader is cached
        return getCacheReader(file);
    }

    void releaseOwnership(const MFileObject& file)
    {
        MString resolvedFullName = file.resolvedFullName();
        std::string key = resolvedFullName.asChar();
        tbb::unique_lock<tbb::mutex> lock(fMutex);

        // look up the cache
        const LeftIterator iter = fData.left.find(key);
        if (iter != fData.left.end()) {
            // decrease the ownership count
            if (--(*iter).get_right() == 0) {
                // there is one reader that is able to be closed
                fCond.notify_one();
            }
        }
        else {
            // acquire/release mismatch!
            assert(iter != fData.left.end());
        }
    }

    void print()
    {
        tbb::unique_lock<tbb::mutex> lock(fMutex);

        // dump the cache
        std::cout << "File Reader Cache" << std::endl
            << "    Get Count: " << fGetCount << std::endl
            << "    Hit Count: " << fHitCount << std::endl
            << "    Hit Ratio: " << (1.0f * fHitCount / fGetCount) << std::endl;
        std::cout << "LRU list: " << fData.size() << std::endl;
        for (RightIterator iter = fData.right.begin(); iter != fData.right.end(); iter++) {
            std::cout << "    " << (*iter).get_left() << std::endl;
        }
        std::cout << std::endl;
    }

private:
    // Prohibited and not implemented.
    Impl(const Impl&);
    const Impl& operator= (const Impl&);

    boost::shared_ptr<CacheReader> createReader(const MFileObject& file)
    {
        return CacheReader::create("Alembic", file);
    }

    typedef boost::bimaps::unordered_set_of<std::string>   LeftViewType;
    typedef boost::bimaps::list_of<int>                    RightViewType;
    typedef boost::bimaps::with_info<boost::shared_ptr<CacheReader> > InfoViewType;
    typedef boost::bimap<LeftViewType, RightViewType, InfoViewType>   BimapType;
    typedef BimapType::value_type      BimapValueType;
    typedef BimapType::left_iterator   LeftIterator;
    typedef BimapType::right_iterator  RightIterator;
    typedef std::map<std::string,int>  FileRefCountType;
    typedef FileRefCountType::iterator FileRefCountIterator;

    int              fMaxNumFileHandles;
    BimapType        fData;
    FileRefCountType fFileRefCount;

    tbb::mutex                  fMutex;
    tbb::condition_variable     fCond;
    int        fHitCount;
    int        fGetCount;
};


#if 0
#define DEBUG_SCHEDULER 1
#endif

//
// This is the scheduler for background reading of cache files.
// This class maintains a queue for the scheduled read tasks and
// execute them one by one.
// Once the task is finished, it will notify the shape node to 
// update its internal state.
//
class GlobalReaderCache::Scheduler
{
public:
    // The root task for reading hierarchy.
    class BGReadHierarchyTask : public tbb::task
    {
    public:
        BGReadHierarchyTask(Scheduler*             scheduler,
                            const CacheFileEntry*  entry,
                            CacheReaderProxy::Ptr& proxy,
                            const MString&         geometryPath)
            : fScheduler(scheduler),
              fCacheFileEntry(entry),
              fProxy(proxy), 
              fGeometryPath(geometryPath)
        {}

        virtual ~BGReadHierarchyTask()
        {}

        virtual task* execute()
        {
            // Read the cache file
            SubNode::Ptr          geometry;
            MString               validatedGeometryPath = fGeometryPath;
            MaterialGraphMap::Ptr materials;

            try {
                CacheReaderHolder holder(fProxy);
                const boost::shared_ptr<CacheReader> cacheReader = holder.getCacheReader();


                if (cacheReader && cacheReader->valid()) {
                    // Validate the input geometry path
                    cacheReader->validateGeomPath(fGeometryPath, validatedGeometryPath);

                    // Read the hierarchy
                    geometry = cacheReader->readHierarchy(
                        validatedGeometryPath, !Config::isIgnoringUVs());

                    // Read the materials
                    materials = cacheReader->readMaterials();
                }
            }
            catch (CacheReaderInterruptException&) {
#ifdef DEBUG_SCHEDULER
                std::cout << "[gpuCache] Background reading is interrupted" << std::endl;
#endif
            }
            catch (std::exception&) {
#ifdef DEBUG_SCHEDULER
                std::cout << "[gpuCache] Background reading is interrupted for unknown reason" << std::endl;
#endif
            }

            // Callback to scheduler that this task is finished.
            fScheduler->hierarchyTaskFinished(fCacheFileEntry, 
                geometry, validatedGeometryPath, materials, fProxy);
            fProxy.reset();

            return 0;
        }

    private:
        Scheduler*            fScheduler;
        const CacheFileEntry* fCacheFileEntry;
        CacheReaderProxy::Ptr fProxy;
        MString               fGeometryPath;
    };

    // The root task for reading shape data.
    class BGReadShapeTask : public tbb::task
    {
    public:
        BGReadShapeTask(Scheduler*             scheduler,
                        const CacheFileEntry*  entry,
                        CacheReaderProxy::Ptr& proxy,
                        const MString&         prefix,
                        const MString&         geometryPath)
            : fScheduler(scheduler),
              fCacheFileEntry(entry),
              fProxy(proxy), 
              fPrefix(prefix),
              fGeometryPath(geometryPath)
        {}

        virtual ~BGReadShapeTask()
        {}

        virtual task* execute()
        {
            // Read the cache file for the specified geometry path
            SubNode::Ptr geometry;

            try {
                CacheReaderHolder holder(fProxy);
                const boost::shared_ptr<CacheReader> cacheReader = holder.getCacheReader();

                if (cacheReader && cacheReader->valid()) {
                    // Read the specified shape
                    geometry = cacheReader->readShape(fPrefix + fGeometryPath, !Config::isIgnoringUVs());
                }
            }
            catch (CacheReaderInterruptException&) {
#ifdef DEBUG_SCHEDULER
                std::cout << "[gpuCache] Background reading is interrupted" << std::endl;
#endif
            }
            catch (std::exception&) {
#ifdef DEBUG_SCHEDULER
                std::cout << "[gpuCache] Background reading is interrupted for unknown reason" << std::endl;
#endif
            }

            // Must release the reader proxy here so the CacheReader can be destroyed early.
            fProxy.reset();

            // Callback to scheduler that this task is finished.
            fScheduler->shapeTaskFinished(fCacheFileEntry, geometry, fGeometryPath);

            return 0;
        }

    private:
        Scheduler*            fScheduler;
        const CacheFileEntry* fCacheFileEntry;
        CacheReaderProxy::Ptr fProxy;
        MString               fPrefix;
        MString               fGeometryPath;
    };

    class WorkItem
    {
    public:
        typedef boost::shared_ptr<WorkItem> Ptr;

        enum WorkItemType {
            kHierarchyWorkItem,
            kShapeWorkItem,
        };

        WorkItem(Scheduler*             scheduler,
                 const CacheFileEntry*  entry, 
                 const MString&         geometryPath,
                 CacheReaderProxy::Ptr& proxy)
            : fCacheFileEntry(entry), 
              fSubNode(NULL),
              fValidatedGeometryPath(geometryPath), 
              fCancelled(false),
              fType(kHierarchyWorkItem)
        {
            // Create task for reading hierarchy
            fTask = new (tbb::task::allocate_root())
                BGReadHierarchyTask(scheduler, entry, proxy, geometryPath);
        }

        WorkItem(Scheduler*             scheduler,
                 const CacheFileEntry*  entry, 
                 const SubNode*         subNode,
                 const MString&         prefix,
                 const MString&         geometryPath,
                 CacheReaderProxy::Ptr& proxy)
            : fCacheFileEntry(entry), 
              fSubNode(subNode),
              fValidatedGeometryPath(geometryPath), 
              fCancelled(false),
              fType(kShapeWorkItem)
        {
            // Create task for reading shape
            fTask = new (tbb::task::allocate_root())
                BGReadShapeTask(scheduler, entry, proxy, prefix, geometryPath);
        }

        ~WorkItem()
        { 
            // The task has not been scheduled to run. Delete it.
            if (fTask) {
                assert(fTask->state() != tbb::task::executing);
                tbb::task::destroy(*fTask);
            }
        }

        void startTask()
        {
            assert(fTask);
            if (fTask) {
                tbb::task::enqueue(*fTask);
            }
        }

        void cancelTask()
        {
            assert(fTask);
            fCancelled = true;
        }

        void finishTask(SubNode::Ptr& geometry, 
                        const MString& validatedGeometryPath, 
                        MaterialGraphMap::Ptr& materials)
        {
            // fTask will be automatically destroyed by TBB after execute()
            fTask = NULL;
            fGeometry = geometry;
            fMaterials = materials;
            fValidatedGeometryPath = validatedGeometryPath;
        }

        const CacheFileEntry*       cacheFileEntry() const  { return fCacheFileEntry; }
        const SubNode*              subNode()   const  { return fSubNode;  }
        SubNode::Ptr&               geometry()         { return fGeometry; }
        const MString&              validatedGeometryPath() const { return fValidatedGeometryPath; }
        MaterialGraphMap::Ptr&      materials()     { return fMaterials; }

        bool          isCancelled() const { return fCancelled; }
        WorkItemType  type() const        { return fType; }

    private:
        const CacheFileEntry* fCacheFileEntry;
        const SubNode*        fSubNode;
        tbb::task*            fTask;
        SubNode::Ptr          fGeometry;
        MString               fValidatedGeometryPath;
        MaterialGraphMap::Ptr fMaterials;
        bool                  fCancelled;
        WorkItemType          fType;
    };


    Scheduler()
    {
        fInterrupted = false;
        fPaused      = false;
        fRefreshTime = boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();
    }

    ~Scheduler() {}

    // Schedule an async read. This function will return immediately.
    bool scheduleRead(const CacheFileEntry*  entry, 
                      const MString&         geometryPath,
                      CacheReaderProxy::Ptr& proxy
    )
    {
        // Assumption: Called from the main thread.
        assert(entry && proxy);

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        if (!entry) return false;

        // Create a new work item for reading hierarchy
        WorkItem::Ptr item(new WorkItem(this, entry, geometryPath, proxy));

#ifdef DEBUG_SCHEDULER
        MString fileName = entry->fCacheFileName;
        std::cout << "[gpuCache] Schedule background reading of " << fileName.asChar() << std::endl;
#endif

        if (fTaskRunning) {
            // We have a running task, push to pending queue
            fHierarchyTaskQueue.push_back(item);
        }
        else {
            // No task running, start this task
            fTaskRunning = item;
            fTaskRunning->startTask();
        }

        return true;
    }

    // Pull the hierarchy data.
    bool pullHierarchy(const CacheFileEntry*  entry, 
                       SubNode::Ptr&          geometry,
                       MString&               validatedGeometryPath,
                       MaterialGraphMap::Ptr& materials
    )
    {
        // Assumption: Called from the main thread.

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        if (!entry) return false;

        // Find the hierarchy task. We have exactly 1 task for reading hierarchy.
        WorkItem::Ptr resultItem;
        HierarchyItemPtrListHashIterator iter = fHierarchyTaskDone.get<1>().find(entry);
        if (iter != fHierarchyTaskDone.get<1>().end()) {
            resultItem = *iter;
            fHierarchyTaskDone.get<1>().erase(iter);
        }

        // Background read complete
        if (resultItem) {
            assert(resultItem->type() == WorkItem::kHierarchyWorkItem);

#ifdef DEBUG_SCHEDULER
            MString fileName = entry->fCacheFileName;
            std::cout << "[gpuCache] Background reading (hierarchy) of " << name.asChar() << " finished." << std::endl;
#endif

            // Return the sub-node hierarchy and validated geometry path.
            geometry              = resultItem->geometry();
            validatedGeometryPath = resultItem->validatedGeometryPath();
            materials             = resultItem->materials();
            return true;
        }

#ifdef DEBUG
        // Make sure that the read is really in progress
        bool inProgress = false;
        if (fTaskRunning && fTaskRunning->cacheFileEntry() == entry) {
            inProgress = true;
        }
        if (fHierarchyTaskQueue.get<1>().find(entry) != fHierarchyTaskQueue.get<1>().end()) {
            inProgress = true;
        }
        assert(inProgress);
#endif

        // Background read in progress
        return false;
    }

    // Pull the shape data.
    bool pullShape(const CacheFileEntry*   entry, 
                   GPUCache::SubNode::Ptr& geometry
    )
    {
        // Assumption: Called from the main thread.
        assert(geometry);
        if (!geometry) return false;

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        if (!entry) return false;

        // A list of finished tasks
        std::vector<WorkItem::Ptr> resultItems;

        // Find the entry tasks, we have one or more tasks for reading shapes
        std::pair<ShapeItemPtrListHashIterator,ShapeItemPtrListHashIterator> range =
                fShapeTaskDone.get<1>().equal_range(entry);
        for (ShapeItemPtrListHashIterator iter = range.first; iter != range.second; iter++) {
            resultItems.push_back(*iter);
        }
        fShapeTaskDone.get<1>().erase(range.first, range.second);

        // Background read complete
        BOOST_FOREACH (const WorkItem::Ptr& item, resultItems) {
            assert(item->type() == WorkItem::kShapeWorkItem);

#ifdef DEBUG_SCHEDULER
            MString fileName = entry->fCacheFileName;
            std::cout << "[gpuCache] Background reading (shape) of " << fileName.asChar() << " finished." << std::endl;
#endif

            // Replace the shape node data.
            SubNode::Ptr shape = item->geometry();
            MString      path  = item->validatedGeometryPath();
            if (shape && path.length() > 0) {
                ReplaceSubNodeData(geometry, shape, path);
            }
        }

        // If we replace shape node data, update the transparent type.
        if (!resultItems.empty()) {
            SubNodeTransparentTypeVisitor transparentTypeVisitor;
            geometry->accept(transparentTypeVisitor);
        }

        // Check if we still have task in progress or queued
        bool inProgress = false;
        if (fTaskRunning && fTaskRunning->cacheFileEntry() == entry) {
            inProgress = true;
        }
        if (fShapeTaskQueue.get<1>().find(entry) != fShapeTaskQueue.get<1>().end()) {
            inProgress = true;
        }

        // Return false if we still have shape tasks in progress or queued
        return !inProgress;
    }

    void hintShapeReadOrder(const SubNode& subNode)
    {
        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        // Note that the (const SubNode*) is ONLY used for comparison.
        SubNodePtrList::nth_index<1>::type::iterator iter = 
            fShapeTaskOrder.get<1>().find(&subNode);
        if (iter == fShapeTaskOrder.get<1>().end()) {
            // This subNode does not exist in the order list.
            // Insert it to the front.
            fShapeTaskOrder.push_front(&subNode);
        }
        else {
            // This subNode exists. Move it to the front.
            fShapeTaskOrder.get<1>().erase(iter);
            fShapeTaskOrder.push_front(&subNode);
        }
    }

    // Cancel the async read.
    void cancelRead(const CacheFileEntry* entry)
    {
        // Assumption: Called from the main thread.
        assert(entry);

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        if (!entry) return;

#ifdef DEBUG_SCHEDULER
        MString fileName = entry->fCacheFileName;
        std::cout << "[gpuCache] Background reading of " << fileName.asChar() << " canceled" << std::endl;
#endif

        // Remove the queued hierarchy task
        fHierarchyTaskQueue.get<1>().erase(entry);

        // Remove the finished hierarchy task
        fHierarchyTaskDone.get<1>().erase(entry);

        // Remove the queued shape tasks
        fShapeTaskQueue.get<1>().erase(entry);

        // Remove the finished shape tasks
        fShapeTaskDone.get<1>().erase(entry);

        // Check the current running task
        if (fTaskRunning && fTaskRunning->cacheFileEntry() == entry) {
            fTaskRunning->cancelTask();
            fInterrupted = true;
        }

        // Notify there are task cancelled
        fCondition.notify_all();
    }

    void waitForRead(const CacheFileEntry* entry)
    {
        // Assumption: Called from the main thread.
        assert(entry);

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        if (!entry) return;

#ifdef DEBUG_SCHEDULER
        MString fileName = entry->fCacheFileName;
        std::cout << "[gpuCache] Waiting for background reading of " << fileName.asChar() << std::endl;
#endif

        while (true) {
            // Find the task
            bool inProgress = false;
            if (fTaskRunning && fTaskRunning->cacheFileEntry() == entry) {
                inProgress = true;
            }
            if (fHierarchyTaskQueue.get<1>().find(entry) != fHierarchyTaskQueue.get<1>().end()) {
                inProgress = true;
            }
            if (fShapeTaskQueue.get<1>().find(entry) != fShapeTaskQueue.get<1>().end()) {
                inProgress = true;
            }

            // Return if the task is done.
            if (!inProgress) break;

            // Wait until the in progress task finishes
            fCondition.wait_for(lock, DURATION_SECONDS(3));
        }
    }

    bool isInterrupted()
    {
        return fInterrupted;
    }

    void pauseRead()
    {
        tbb::unique_lock<tbb::mutex> lock(fPauseMutex);

        // Set the "Paused" flag to true.
        // If the reader supports pause/resume, 
        // it should check this flag by calling isPaused() and pauseUntilNotified().
        assert(!fPaused);
        fPaused = true;
    }

    void resumeRead()
    {
        tbb::unique_lock<tbb::mutex> lock(fPauseMutex);

        // Clear the "Paused" flag.
        assert(fPaused);
        fPaused = false;

        // Resume the worker thread...
        fPauseCond.notify_all();
    }

    bool isPaused()
    {
        return fPaused;
    }

    void pauseUntilNotified()
    {
        tbb::unique_lock<tbb::mutex> lock(fPauseMutex);

        // If the "Paused" flag is true, pause the worker thread by waiting on the mutex.
        if (fPaused) {
            // Pause this worker thread.
            fPauseCond.wait(lock);
        }
    }

private:
    friend class WorkItem;

    void hierarchyTaskFinished(const CacheFileEntry*  entry, 
                               SubNode::Ptr&          geometry, 
                               const MString&         validatedGeometryPath,
                               MaterialGraphMap::Ptr& materials,
                               CacheReaderProxy::Ptr& proxy)
    {
        // Assumption: Called from worker thread.

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        // The task must be the running task
        assert(fTaskRunning);
        assert(fTaskRunning->cacheFileEntry() == entry);
        assert(fTaskRunning->type() == WorkItem::kHierarchyWorkItem);

        // The hierarchy task is finished
        fTaskRunning->finishTask(geometry, validatedGeometryPath, materials);

        // Move the task to done queue
        bool isCancelled = fTaskRunning->isCancelled();
        if (!isCancelled) {
            fHierarchyTaskDone.push_back(fTaskRunning);

            // Extract the shape paths
            ShapePathVisitor::ShapePathAndSubNodeList shapeGeomPaths;
            if (geometry) {
                ShapePathVisitor shapePathVisitor(shapeGeomPaths);
                geometry->accept(shapePathVisitor);
            }

            // The absolute shape path in the archive is prefix+shapePath
            MString prefix;
            int lastStep = validatedGeometryPath.rindexW('|');
            if (lastStep > 0) {
                prefix = validatedGeometryPath.substringW(0, lastStep - 1);
            }

            // Create shape tasks
            BOOST_FOREACH (const ShapePathVisitor::ShapePathAndSubNode& pair, shapeGeomPaths) {
                WorkItem::Ptr item(new WorkItem(
                    this, 
                    entry, 
                    pair.second,   // The SubNode pointer. Hint the shape read order
                    prefix,        // prefix+pair.first is the absolute path in the archive
                    pair.first,    // The relative path from root sub node
                    proxy
                ));
                fShapeTaskQueue.push_back(item);
            }
        }
        fTaskRunning.reset();
        fInterrupted = false;

        // Start the next task
        startNextTask();

		// Dirty VP2 geometry
		ShapeNode::dirtyVP2Geometry( entry->fResolvedCacheFileName );

        // Notify a task has just finished
        fCondition.notify_all();

        // Schedule a refresh
        postRefresh();
    }

    void shapeTaskFinished(const CacheFileEntry* entry, 
                           SubNode::Ptr&         geometry, 
                           const MString&        geometryPath)
    {
        // Assumption: Called from worker thread.

        // Lock the scheduler
        tbb::unique_lock<tbb::mutex> lock(fBigMutex);

        // The task must be the running task
        assert(fTaskRunning);
        assert(fTaskRunning->cacheFileEntry() == entry);
        assert(fTaskRunning->type() == WorkItem::kShapeWorkItem);

        // The hierarchy task is finished
        MaterialGraphMap::Ptr noMaterials;
        fTaskRunning->finishTask(geometry, geometryPath, noMaterials);

        // Move the task to done queue
        bool isCancelled = fTaskRunning->isCancelled();
        if (!isCancelled) {
            fShapeTaskDone.push_back(fTaskRunning);
        }
        fTaskRunning.reset();
        fInterrupted = false;

        // Start the next task
        startNextTask();

        // Notify a task has just finished
        fCondition.notify_all();

        // Schedule a refresh
        postRefresh();
    }

    void startNextTask()
    {
        // Hierarchy task take the precedence over shape tasks
        if (!fHierarchyTaskQueue.empty()) {
            fTaskRunning = fHierarchyTaskQueue.front();
            fTaskRunning->startTask();
            fHierarchyTaskQueue.pop_front();
            return;
        }

        // Pick up a shape task in the order list.
        while (!fShapeTaskOrder.empty()) {
            const SubNode* subNode = fShapeTaskOrder.front();
            assert(subNode);
            fShapeTaskOrder.pop_front();

            // Search the shape task list for the shape
            ShapeItemPtrListSubNodeHashIterator iter = fShapeTaskQueue.get<2>().find(subNode);
            if (iter != fShapeTaskQueue.get<2>().end()) {
                fTaskRunning = *iter;
                fTaskRunning->startTask();
                fShapeTaskQueue.get<2>().erase(iter);
                return;
            }
        }

        // Check if we have shape task
        if (!fShapeTaskQueue.empty()) {
            fTaskRunning = fShapeTaskQueue.front();
            fTaskRunning->startTask();
            fShapeTaskQueue.pop_front();
            return;
        }
    }

    void postRefresh()
    {
        // Get current time
        boost::posix_time::ptime currentTime = 
            boost::date_time::microsec_clock<boost::posix_time::ptime>::local_time();

        // Last hierarchy or shape task, force a refresh
        if (!fTaskRunning) {
            fRefreshTime = currentTime;
            MGlobal::executeCommandOnIdle("refresh -f;");
        }

        // Check refresh interval
        boost::posix_time::time_duration interval = currentTime - fRefreshTime;
        if (interval.total_milliseconds() >= (int)Config::backgroundReadingRefresh()) {
            fRefreshTime = currentTime;
            MGlobal::executeCommandOnIdle("refresh -f;");
        }
    }

private:
    tbb::mutex                   fBigMutex;
    tbb::condition_variable      fCondition;

    // A list of work items with (const CacheFileEntry*) as hash key
    typedef boost::multi_index_container<
        WorkItem::Ptr,
        boost::multi_index::indexed_by<
            boost::multi_index::sequenced<>,
            boost::multi_index::hashed_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(WorkItem,const CacheFileEntry*,cacheFileEntry)>
        >
    > HierarchyItemPtrList;
    typedef HierarchyItemPtrList::nth_index<1>::type::iterator HierarchyItemPtrListHashIterator;

    // A list of work items with (const CacheFileEntry*) as hash key (multi)
    typedef boost::multi_index_container<
        WorkItem::Ptr,
        boost::multi_index::indexed_by<
            boost::multi_index::sequenced<>,
            boost::multi_index::hashed_non_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(WorkItem,const CacheFileEntry*,cacheFileEntry)>,
            boost::multi_index::hashed_unique<BOOST_MULTI_INDEX_CONST_MEM_FUN(WorkItem,const SubNode*,subNode)>
        >
    > ShapeItemPtrList;
    typedef ShapeItemPtrList::nth_index<1>::type::iterator ShapeItemPtrListHashIterator;
    typedef ShapeItemPtrList::nth_index<2>::type::iterator ShapeItemPtrListSubNodeHashIterator;

    WorkItem::Ptr        fTaskRunning;
    HierarchyItemPtrList fHierarchyTaskQueue;
    HierarchyItemPtrList fHierarchyTaskDone;
    ShapeItemPtrList     fShapeTaskQueue;
    ShapeItemPtrList     fShapeTaskDone;

    typedef boost::multi_index_container<
        const SubNode*,
        boost::multi_index::indexed_by<
            boost::multi_index::sequenced<>,
            boost::multi_index::hashed_unique<boost::multi_index::identity<const SubNode*> >
        >
    > SubNodePtrList;
    SubNodePtrList fShapeTaskOrder;

    tbb::atomic<bool>        fInterrupted;
    boost::posix_time::ptime fRefreshTime;
    
    // Pause and resume the worker thread.
    tbb::atomic<bool>           fPaused;
    tbb::mutex                  fPauseMutex;
    tbb::condition_variable     fPauseCond;
};


//==============================================================================
// CLASS CacheFileEntry
//==============================================================================

CacheFileEntry::CacheFileEntry()
:	fCachedGeometry()
,	fCachedMaterial()
,	fCacheReaderProxy()
,	fReadState(CacheFileEntry::kReadingDone)
{}

CacheFileEntry::CacheFileEntry( const CacheFileEntry& rhs )
{
	(*this) = rhs;
}

CacheFileEntry::CacheFileEntry( const MString& fileName )
:	fCacheFileName(fileName)
,	fCachedGeometry()
,	fCachedMaterial()
,	fCacheReaderProxy()
,	fReadState(CacheFileEntry::kReadingDone)
{
	// Create the CacheReaderProxy
	if( fileName.length() > 0 )
	{
		MFileObject cacheFile;
		cacheFile.setRawFullName(fileName);
		cacheFile.setResolveMethod(MFileObject::kInputFile);
		fResolvedCacheFileName = cacheFile.resolvedFullName();
		fCacheReaderProxy = GlobalReaderCache::theCache().getCacheReaderProxy(cacheFile);
	}
}

CacheFileEntry::~CacheFileEntry()
{}

CacheFileEntry& CacheFileEntry::operator=( const CacheFileEntry& rhs )
{
	fCacheFileName = rhs.fCacheFileName;
	fCachedGeometry = rhs.fCachedGeometry;
	fCachedMaterial = rhs.fCachedMaterial;
	fCacheReaderProxy = rhs.fCacheReaderProxy;
	fReadState = rhs.fReadState;
	return (*this);
}


//==============================================================================
// CLASS CacheFileRegistry
//==============================================================================


CacheFileRegistry CacheFileRegistry::fsSingleton;
CacheFileRegistry::Map CacheFileRegistry::fMap;

CacheFileRegistry::CacheFileRegistry()
{}

CacheFileRegistry::~CacheFileRegistry()
{}

CacheFileRegistry& CacheFileRegistry::theCache()
{
	return fsSingleton;
}

void CacheFileRegistry::getAll( std::vector<CacheFileEntry::MPtr>& entries ) const
{
	entries.clear();

	Map::iterator it = fMap.begin();
	for( ; it != fMap.end(); it++ )
	{
		entries.push_back(it->second);
	}
}

CacheFileEntry::MPtr CacheFileRegistry::find( const MString& key )
{
	Map::iterator it = fMap.find(key);
	if( it != fMap.end() )
	{
		return it->second;
	}
	return CacheFileEntry::MPtr();
}

bool CacheFileRegistry::insert( const MString& key, const CacheFileEntry::MPtr& entry )
{
	// Search for an existing entry before insertion
	Map::iterator it = fMap.find(key);
	if( it != fMap.end() )
	{
		// Update the existing entry.
		it->second = entry;
		return true;
	}

	// Not found, insert new item.
	Map::value_type item( key, entry );
	return fMap.insert( item ).second;
}

bool CacheFileRegistry::remove( const MString& key )
{
	Map::iterator it = fMap.find(key);
	if( it != fMap.end() )
	{
		fMap.erase(it);
		return true;
	}
	return false;
}

size_t CacheFileRegistry::size() const
{
	return fMap.size();
}

bool CacheFileRegistry::cleanUp( const MString& key )
{
	Map::iterator it = fMap.find(key);
	if( it != fMap.end() )
	{
		CacheFileEntry::MPtr& entry = it->second;
		if( entry.use_count() == 1 )
		{
			// This entry now only has one reference to it: this map.
			// Cancel any pending read and remove the entry.
			if( entry->fReadState != CacheFileEntry::kReadingDone )
			{
				GlobalReaderCache::theCache().cancelRead(entry.get());
				entry->fReadState = CacheFileEntry::kReadingDone;
			}
			fMap.erase(it);
			return true;
		}
	}
	return false;
}

void CacheFileRegistry::clear()
{
	fMap.clear();
}


//==============================================================================
// CLASS GlobalReaderCache
//==============================================================================

GlobalReaderCache& GlobalReaderCache::theCache()
{
    static GlobalReaderCache gsReaderCache;
    return gsReaderCache;
}

int GlobalReaderCache::maxNumOpenFiles()
{
    // an estimate on the max number of open files when gpuCache command is executed
    const int mayaOpenFiles = 100;

    // query the current soft limit and raise the soft limit if possible
    int softLimit;
#ifdef _WIN32
    // MSVC limits the max open files to 2048
    _setmaxstdio(2048);
    softLimit = _getmaxstdio();
#else
    // query the soft limit and hard limit on MAC/Linux
    rlimit rlp;
    getrlimit(RLIMIT_NOFILE, &rlp);
    // try to raise the soft limit to hard limit
    rlp.rlim_cur = rlp.rlim_max;
    setrlimit(RLIMIT_NOFILE, &rlp);
    getrlimit(RLIMIT_NOFILE, &rlp);
    if (rlp.rlim_cur < rlp.rlim_max) {
        // raise to hard limit failed, try 8000
        rlp.rlim_cur = (rlp.rlim_max > 0 && rlp.rlim_max <= 8000) ? rlp.rlim_max : 8000;
        setrlimit(RLIMIT_NOFILE, &rlp);
    }
    // query the new soft limit
    getrlimit(RLIMIT_NOFILE, &rlp);
    softLimit = (rlp.rlim_cur > INT_MAX) ? INT_MAX : (int)rlp.rlim_cur;
#endif
    softLimit -= (mayaOpenFiles + 3);
    return softLimit;
}

//==============================================================================
// CLASS GlobalReaderCache::CacheReaderProxy
//==============================================================================

GlobalReaderCache::CacheReaderProxy::CacheReaderProxy(const MFileObject& file)
    : fFile(file)
{
    GlobalReaderCache::theCache().increaseFileRef(fFile);
}

GlobalReaderCache::CacheReaderProxy::~CacheReaderProxy()
{
    GlobalReaderCache::theCache().decreaseFileRef(fFile);
}


//==============================================================================
// CLASS GlobalReaderCache::CacheReaderHolder
//==============================================================================

GlobalReaderCache::CacheReaderHolder::CacheReaderHolder(boost::shared_ptr<CacheReaderProxy> proxy)
    : fProxy(proxy)
{
    fReader = GlobalReaderCache::theCache().acquireOwnership(fProxy->file());
}

GlobalReaderCache::CacheReaderHolder::~CacheReaderHolder()
{
    GlobalReaderCache::theCache().releaseOwnership(fProxy->file());
}

boost::shared_ptr<CacheReader> GlobalReaderCache::CacheReaderHolder::getCacheReader()
{
    return fReader;
}


//==============================================================================
// CLASS GlobalReaderCache
//==============================================================================

GlobalReaderCache::GlobalReaderCache()
    : fImpl(new Impl(maxNumOpenFiles())), fScheduler(new Scheduler())
{}

GlobalReaderCache::~GlobalReaderCache()
{}

boost::shared_ptr<GlobalReaderCache::CacheReaderProxy>
    GlobalReaderCache::getCacheReaderProxy(const MFileObject& file)
{
    return boost::make_shared<CacheReaderProxy>(file);
}

bool GlobalReaderCache::scheduleRead(
    const CacheFileEntry*  entry, 
    const MString&         geometryPath,
    CacheReaderProxy::Ptr& proxy
)
{
    return fScheduler->scheduleRead(entry, geometryPath, proxy);
}

bool GlobalReaderCache::pullHierarchy(
    const CacheFileEntry*  entry,
    SubNode::Ptr&          geometry,
    MString&               validatedGeometryPath,
    MaterialGraphMap::Ptr& materials
)
{
    return fScheduler->pullHierarchy(entry, geometry, validatedGeometryPath, materials);
}

bool GlobalReaderCache::pullShape(
    const CacheFileEntry*   entry,
    GPUCache::SubNode::Ptr& geometry
)
{
    return fScheduler->pullShape(entry, geometry);
}

void GlobalReaderCache::hintShapeReadOrder(const GPUCache::SubNode& subNode)
{
    return fScheduler->hintShapeReadOrder(subNode);
}

void GlobalReaderCache::cancelRead(const CacheFileEntry* entry)
{
    fScheduler->cancelRead(entry);
}

void GlobalReaderCache::waitForRead(const CacheFileEntry* entry)
{
    fScheduler->waitForRead(entry);
}

bool GlobalReaderCache::isInterrupted()
{
    return fScheduler->isInterrupted();
}

void GlobalReaderCache::pauseRead()
{
    fScheduler->pauseRead();
}

void GlobalReaderCache::resumeRead()
{
    fScheduler->resumeRead();
}

bool GlobalReaderCache::isPaused()
{
    return fScheduler->isPaused();
}

void GlobalReaderCache::pauseUntilNotified()
{
    fScheduler->pauseUntilNotified();
}

void GlobalReaderCache::increaseFileRef(const MFileObject& file)
{
    fImpl->increaseFileRef(file);
}

void GlobalReaderCache::decreaseFileRef(const MFileObject& file)
{
    fImpl->decreaseFileRef(file);
}

boost::shared_ptr<CacheReader> GlobalReaderCache::acquireOwnership(const MFileObject& file)
{
    return fImpl->acquireOwnership(file);
}

void GlobalReaderCache::releaseOwnership(const MFileObject& file)
{
    fImpl->releaseOwnership(file);
}


//==============================================================================
// CLASS CacheReader
//==============================================================================

std::map<std::string,CacheReader::CreateFunction*> CacheReader::fsRegistry;

boost::shared_ptr<CacheReader> CacheReader::create(const MString& impl,
    const MFileObject& file)
{
    std::string key = impl.asChar();
    std::map<std::string,CreateFunction*>::iterator iter = fsRegistry.find(key);
    if (iter != fsRegistry.end()) {
        return ((*iter).second)(file);
    }

    assert("not implemented");
    return boost::shared_ptr<CacheReader>();
}

void CacheReader::registerReader(const MString& impl, CreateFunction* func)
{
    std::string key = impl.asChar();
    fsRegistry[key] = func;
}



