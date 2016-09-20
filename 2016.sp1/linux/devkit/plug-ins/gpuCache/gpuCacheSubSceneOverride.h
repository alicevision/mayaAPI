#ifndef _gpuCacheSubSceneOverride_h_
#define _gpuCacheSubSceneOverride_h_

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

#include "gpuCacheShapeNode.h"
#include "CacheReader.h"

#include <boost/shared_ptr.hpp>

#define BOOST_DATE_TIME_NO_LIB
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/microsec_time_clock.hpp>

#include <maya/MDagPathArray.h>
#include <maya/MPxSubSceneOverride.h>

#include <maya/MCallbackIdArray.h>
#include <maya/MMessage.h>


namespace GPUCache {

class ShapeNode;

/*==============================================================================
 * CLASS SubSceneOverride
 *============================================================================*/

// Handles the drawing of the cached geometry in the viewport 2.0.
class SubSceneOverride : public MHWRender::MPxSubSceneOverride
{
public:
    // Callback method to create a new MPxSubSceneOverride
    static MHWRender::MPxSubSceneOverride* creator(const MObject& object);

    // Clear all Viewport 2.0 buffers.
    static void clear();

    // Find the Viewport 2.0 index buffer.
    static MHWRender::MIndexBuffer*  lookup(const boost::shared_ptr<const IndexBuffer>& indices);

    // Find the Viewport 2.0 vertex buffer.
    static MHWRender::MVertexBuffer* lookup(const boost::shared_ptr<const VertexBuffer>& vertices);

    // Constructor and Destructor
    SubSceneOverride(const MObject& object);
    virtual ~SubSceneOverride();

    // Overrides
    virtual MHWRender::DrawAPI supportedDrawAPIs() const;

    virtual bool requiresUpdate(const MHWRender::MSubSceneContainer& container,
                                const MHWRender::MFrameContext&      frameContext) const;

    virtual void update(MHWRender::MSubSceneContainer&  container,
                        const MHWRender::MFrameContext& frameContext);

	virtual bool getSelectionPath(const MHWRender::MRenderItem& renderItem, MDagPath& dagPath) const;


    // Dirty methods (called from Callbacks)
    void dirtyEverything();
    void dirtyRenderItems();
    void dirtyVisibility();
    void dirtyWorldMatrix();
    void dirtyStreams();
    void dirtyMaterials();
    void resetDagPaths();

    // Register node dirty callbacks.
    void registerNodeDirtyCallbacks();
    void clearNodeDirtyCallbacks();

    // Current state methods
    const SubNode::Ptr&          getGeometry() const { return fGeometry; }
    const MaterialGraphMap::Ptr& getMaterial() const { return fMaterial;}
    double                       getTime()     const { return fTimeInSeconds; }

    bool castsShadows()   const { return fCastsShadowsPlug.asBool(); }
    bool receiveShadows() const { return fReceiveShadowsPlug.asBool(); }

    // Hardware instancing
    class HardwareInstanceManager;
    boost::shared_ptr<HardwareInstanceManager>& hardwareInstanceManager()
    { return fHardwareInstanceManager; }

    // Update methods
    void updateRenderItems(MHWRender::MSubSceneContainer&  container,
                           const MHWRender::MFrameContext& frameContext);
    void updateVisibility(MHWRender::MSubSceneContainer&  container,
                          const MHWRender::MFrameContext& frameContext);
    void updateWorldMatrix(MHWRender::MSubSceneContainer&  container,
                           const MHWRender::MFrameContext& frameContext);
    void updateStreams(MHWRender::MSubSceneContainer&  container,
                       const MHWRender::MFrameContext& frameContext);
    void updateMaterials(MHWRender::MSubSceneContainer&  container,
                         const MHWRender::MFrameContext& frameContext);

private:
    // Pruning non-animated sub-hierarchy.
    class HierarchyStat;
    class HierarchyStatVisitor;

    const boost::shared_ptr<const HierarchyStat>& getHierarchyStat() const
    { return fHierarchyStat; }


    const MObject    fObject;
    const ShapeNode* fShapeNode;
    MPlug            fCastsShadowsPlug;
    MPlug            fReceiveShadowsPlug;

    CacheFileEntry::BackgroundReadingState fReadingState;
    SubNode::Ptr                           fGeometry;
    MaterialGraphMap::Ptr                  fMaterial;
    double                                 fTimeInSeconds;

    boost::posix_time::ptime fUpdateTime;

    // Callbacks
    MCallbackId fInstanceAddedCallback;
    MCallbackId fInstanceRemovedCallback;
    MCallbackId fWorldMatrixChangedCallback;
    MCallbackIdArray fNodeDirtyCallbacks;

    // Dirty flags
    bool fUpdateRenderItemsRequired;
    bool fUpdateVisibilityRequired;
    bool fUpdateWorldMatrixRequired;
    bool fUpdateStreamsRequired;
    bool fUpdateMaterialsRequired;

    bool fOutOfViewFrustum;
    bool fOutOfViewFrustumUpdated;

    // Wireframe on Shaded mode: Full/Reduced/None
    DisplayPref::WireframeOnShadedMode fWireOnShadedMode;

    // The render items for all instances of this gpuCache node.
    class SubNodeRenderItems;
    class UpdateRenderItemsVisitor;
    template<typename DERIVED> class UpdateVisitorWithPrune;
    class UpdateVisibilityVisitor;
    class UpdateWorldMatrixVisitor;
    class UpdateStreamsVisitor;
    class UpdateDiffuseColorVisitor;
    class InstanceRenderItems;
    typedef std::vector<boost::shared_ptr<InstanceRenderItems> > InstanceRenderItemList;
    typedef std::vector<boost::shared_ptr<SubNodeRenderItems> >  SubNodeRenderItemList;

    MDagPathArray          fInstanceDagPaths;
    InstanceRenderItemList fInstanceRenderItems;

    // The hierarchy status to help pruning.
    boost::shared_ptr<const HierarchyStat> fHierarchyStat;

    // Manages all hardware instances. NULL if hardware instancing is disabled.
    boost::shared_ptr<HardwareInstanceManager> fHardwareInstanceManager;
};

} // namespace GPUCache

#endif
