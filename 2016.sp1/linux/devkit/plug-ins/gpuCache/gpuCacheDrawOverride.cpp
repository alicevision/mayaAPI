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

#include "gpuCacheDrawOverride.h"

#include "gpuCacheConfig.h"
#include "gpuCacheShapeNode.h"
#include "gpuCacheDrawTraversal.h"
#include "gpuCacheFrustum.h"
#include "gpuCacheGLFT.h"

#include <maya/MAnimControl.h>
#include <maya/MDrawContext.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MGlobal.h>
#include <maya/MHWGeometryUtilities.h>
#include <maya/MStateManager.h>
#include <maya/MUserData.h>

#include <algorithm>


namespace {

using namespace GPUCache;

//==============================================================================
// LOCAL TYPES
//==============================================================================

enum DepthOffsetType {
    kNoDepthOffset,
    kApplyDepthOffset,
    kNbDepthOffsetType
};

enum ColorType {
    kSubNodeColor,
    kDefaultColor,
    kBlackColor,
    kXrayColor,
    kXrayBlackColor,
    kNbColorType
};

enum NormalsType {
    kFrontNormals,
    kBackNormals,
    kNbNormalsType
};

enum FrontFaceType {
    kFrontClockwise,
    kFrontCounterClockwise,
    kNbFrontFaceType
};

enum TwoSidedLightingType {
    kTwoSidedLighting,
    kOneSidedLighting,
    kNbTwoSidedLightingType
};


//==============================================================================
// LOCAL FUNCTIONS
//==============================================================================

//------------------------------------------------------------------------------
//
void setShadedBlendState(MHWRender::MStateManager* stateMgr)
{
    using namespace MHWRender;

    static const MBlendState* blendState = NULL;
    if (!blendState) {
        MBlendStateDesc desc;

        desc.targetBlends[0].blendEnable = true;
        desc.targetBlends[0].destinationBlend = MBlendState::kInvSourceAlpha;
        desc.targetBlends[0].alphaDestinationBlend = MBlendState::kInvSourceAlpha;

        blendState = stateMgr->acquireBlendState(desc);
    }
    stateMgr->setBlendState(blendState);
}


//------------------------------------------------------------------------------
//
const MHWRender::MRasterizerState* createShadedRasterState(
    MHWRender::MStateManager* stateMgr,
    const MHWRender::MRasterizerState::CullMode cullMode,
    const DepthOffsetType                       depthOffsetType)
{
    using namespace MHWRender;

    MRasterizerStateDesc desc;
    desc.cullMode = cullMode;

    if (depthOffsetType == kApplyDepthOffset) {
        desc.depthBiasIsFloat       = true ;
        desc.depthBias              = 0.0000002384f;
        desc.slopeScaledDepthBias   = 0.95f;
    }

    return stateMgr->acquireRasterizerState(desc);
}

//------------------------------------------------------------------------------
//
void setShadedRasterState(
    MHWRender::MStateManager* stateMgr,
    const MHWRender::MRasterizerState::CullMode cullMode,
    const DepthOffsetType                       depthOffsetType,
    const FrontFaceType                         frontFaceType)
{
    using namespace MHWRender;

    static const MRasterizerState* cullNoneRasterizerState[kNbDepthOffsetType]  =
        {NULL, NULL};
    static const MRasterizerState* cullFrontRasterizerState[kNbDepthOffsetType] =
        {NULL, NULL};
    static const MRasterizerState* cullBackRasterizerState[kNbDepthOffsetType]  =
        {NULL, NULL};

    if (!cullNoneRasterizerState[kNoDepthOffset]) {
        for (int dot = 0; dot < kNbDepthOffsetType; ++dot) {
            cullNoneRasterizerState[dot] = createShadedRasterState(
                stateMgr, MRasterizerState::kCullNone, DepthOffsetType(dot));
            cullFrontRasterizerState[dot] = createShadedRasterState(
                stateMgr, MRasterizerState::kCullFront, DepthOffsetType(dot));
            cullBackRasterizerState[dot] = createShadedRasterState(
                stateMgr, MRasterizerState::kCullBack, DepthOffsetType(dot));
        }
    }

    switch (cullMode) {
        case MRasterizerState::kCullNone:
            stateMgr->setRasterizerState(
                cullNoneRasterizerState[depthOffsetType]);
            break;

        case MRasterizerState::kCullFront:
            stateMgr->setRasterizerState(
                cullFrontRasterizerState[depthOffsetType]);
            break;

        case MRasterizerState::kCullBack:
            stateMgr->setRasterizerState(
                cullBackRasterizerState[depthOffsetType]);
            break;

        default: assert(0);
    }

    // Must use OpenGL directly here since the MStateManager does not
    // allow us to control this OpenGL state. The
    // MRasterizerState::frontCounterClockwise parameter only controls
    // the winding order used for culling purposes, not for lighting
    // purposes.
    gGLFT->glFrontFace(frontFaceType == kFrontClockwise ? MGL_CW : MGL_CCW);
}


//------------------------------------------------------------------------------
//
void setShadedSolidDepthState(
    MHWRender::MStateManager* stateMgr)
{
    using namespace MHWRender;

    static const MDepthStencilState* depthState = NULL;
    if (!depthState) {
        MDepthStencilStateDesc desc;
        depthState = stateMgr->acquireDepthStencilState(desc);
    }

    stateMgr->setDepthStencilState(depthState);
}


//------------------------------------------------------------------------------
//
void setShadedTwoSidedLightingState(
    const TwoSidedLightingType twoSidedLightingType)
{
    // MStateManager does not allow us to set two-sided lighting state.
    gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE,
        twoSidedLightingType == kTwoSidedLighting ? 1 : 0);
}


//------------------------------------------------------------------------------
//
void setShadedAlphaDepthState(
    MHWRender::MStateManager* stateMgr)
{
    using namespace MHWRender;

    static const MDepthStencilState* depthState = NULL;
    if (!depthState) {
        MDepthStencilStateDesc desc;
        desc.depthWriteEnable = false;
        depthState = stateMgr->acquireDepthStencilState(desc);
    }

    stateMgr->setDepthStencilState(depthState);
}


//------------------------------------------------------------------------------
//
void setWireframeState(
    MHWRender::MStateManager* stateMgr)
{
    using namespace MHWRender;

    {
        static const MBlendState* blendState = NULL;
        if (!blendState) {
            MBlendStateDesc desc;
            blendState = stateMgr->acquireBlendState(desc);
        }

        stateMgr->setBlendState(blendState);
    }

    {
        static const MRasterizerState* rasterizerState = NULL;
        if (!rasterizerState) {
            MRasterizerStateDesc desc;
            rasterizerState = stateMgr->acquireRasterizerState(desc);
        }
        stateMgr->setRasterizerState(rasterizerState);
    }

    {
        static const MDepthStencilState* depthState = NULL;
        if (!depthState) {
            MDepthStencilStateDesc desc;
            depthState = stateMgr->acquireDepthStencilState(desc);
        }

        stateMgr->setDepthStencilState(depthState);
    }
}


//==============================================================================
// LOCAL CLASSES
//==============================================================================

//==============================================================================
// CLASS TopLevelCullVisitor
//==============================================================================

class TopLevelCullVisitor : public SubNodeVisitor
{
public:

    TopLevelCullVisitor(const Frustum& frustrum, double seconds)
        : fFrustum(frustrum),
          fSeconds(seconds),
          fIsCulled(true)
    {}

    bool isCulled() const { return fIsCulled; }


    virtual void visit(const XformData&   xform,
                       const SubNode&     subNode)
    {
        const boost::shared_ptr<const XformSample>& sample =
            xform.getSample(fSeconds);
        if (!sample) return;

        fIsCulled =
            fFrustum.test(sample->boundingBox()) == Frustum::kOutside;
    }

    virtual void visit(const ShapeData&   shape,
                       const SubNode&     subNode)
    {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape.getSample(fSeconds);
        if (!sample) return;

        fIsCulled =
            fFrustum.test(sample->boundingBox()) == Frustum::kOutside;
    }


private:

    const Frustum&  fFrustum;
    const double    fSeconds;
    bool            fIsCulled;
};


//==============================================================================
// CLASS DrawShadedTraversal
//==============================================================================

class DrawShadedState : public DrawTraversalState
{
public:
    DrawShadedState(
        const Frustum&                          frustrum,
        const double                            seconds,
        const TransparentPruneType              transparentPrune,
        MHWRender::MStateManager*               stateMgr,
        MHWRender::MRasterizerState::CullMode   cullMode,
        const DepthOffsetType                   depthOffsetType,
        const ColorType                         colorType,
        const MColor&                           defaultDiffuseColor,
        const NormalsType                       normalsType)
        : DrawTraversalState(frustrum, seconds, transparentPrune),
          fStateMgr(stateMgr),
          fCullMode(cullMode),
          fDepthOffsetType(depthOffsetType),
          fColorType(colorType),
          fDefaultDiffuseColor(defaultDiffuseColor),
          fNormalsType(normalsType)
    {}

    MHWRender::MStateManager* stateManager() const         { return fStateMgr; }
    MHWRender::MRasterizerState::CullMode cullMode() const { return fCullMode; }

    DepthOffsetType depthOffsetType() const     { return fDepthOffsetType; }
    ColorType       colorType() const           { return fColorType; }
    const MColor&   defaultDiffuseColor() const { return fDefaultDiffuseColor; }
    NormalsType     normalsType() const         { return fNormalsType; }

private:
    MHWRender::MStateManager* const         fStateMgr;
    MHWRender::MRasterizerState::CullMode   fCullMode;
    const DepthOffsetType                   fDepthOffsetType;
    const ColorType                         fColorType;
    const MColor                            fDefaultDiffuseColor;
    const NormalsType                       fNormalsType;
};

class DrawShadedTraversal
    : public DrawTraversal<DrawShadedTraversal, DrawShadedState>
{
public:

    typedef DrawTraversal<DrawShadedTraversal, DrawShadedState> BaseClass;

    DrawShadedTraversal(
        DrawShadedState&        state,
        const MMatrix&          xform,
        bool                    isReflection,
        Frustum::ClippingResult parentClippingResult)
        : BaseClass(state, xform, isReflection, parentClippingResult)
    {}

    void draw(const boost::shared_ptr<const ShapeSample>& sample)
    {
        if (!sample->visibility()) return;
        gGLFT->glLoadMatrixd(xform().matrix[0]);

        if (sample->isBoundingBoxPlaceHolder()) {
            state().vboProxy().drawBoundingBox(sample, true);
            GlobalReaderCache::theCache().hintShapeReadOrder(subNode());
            return;
        }

        assert(sample->positions());
        assert(sample->normals());

        MColor diffuseColor;
        switch (state().colorType()) {
        case kSubNodeColor:
            diffuseColor = sample->diffuseColor();
            break;
        case kDefaultColor:
            // Use default material
            diffuseColor = state().defaultDiffuseColor();
            break;
        case kBlackColor:
            // No light -> draw black!
            diffuseColor = MColor(0.0f, 0.0f, 0.0f, sample->diffuseColor()[3]);
            break;
        case kXrayColor:
            // X-Ray mode -> alpha *= 0.3f (extraOpacity)
            diffuseColor = MColor(sample->diffuseColor()[0],
                                  sample->diffuseColor()[1],
                                  sample->diffuseColor()[2],
                                  sample->diffuseColor()[3] * 0.3f);
            break;
        case kXrayBlackColor:
            // X-ray mode + No light
            diffuseColor = MColor(0.0f, 0.0f, 0.0f, sample->diffuseColor()[3] * 0.3f);
            break;
        default:
            assert(0);
        }

        if (diffuseColor[3] <= 0.0 ||
            (diffuseColor[3] >= 1.0 &&
                state().transparentPrune() == DrawShadedState::kPruneOpaque) ||
            (diffuseColor[3] <  1.0 &&
                state().transparentPrune() == DrawShadedState::kPruneTransparent)) {
            return;
        }

        // set colour
        gGLFT->glColor4f(diffuseColor[0]*diffuseColor[3],
                         diffuseColor[1]*diffuseColor[3],
                         diffuseColor[2]*diffuseColor[3],
                         diffuseColor[3]);

        setShadedRasterState(state().stateManager(),
                             state().cullMode(),
                             state().depthOffsetType(),
                             isReflection() ?
                             kFrontClockwise : kFrontCounterClockwise);


        // Draw the triangle mesh for all components.
        for (size_t groupId = 0; groupId < sample->numIndexGroups(); ++groupId ) {
            state().vboProxy().drawTriangles(
                sample, groupId,
                state().normalsType() == kFrontNormals ?
                VBOProxy::kFrontNormals : VBOProxy::kBackNormals,
                VBOProxy::kNoUVs);
        }
    }
};


//==============================================================================
// CLASS DrawWireframeTraversal
//==============================================================================

class DrawWireframeState : public DrawTraversalState
{
public:
    DrawWireframeState(
        const Frustum&  frustrum,
        const double    seconds)
        : DrawTraversalState(frustrum, seconds, kPruneNone)
    {}
};

class DrawWireframeTraversal
    : public DrawTraversal<DrawWireframeTraversal, DrawWireframeState>
{
public:

    typedef DrawTraversal<DrawWireframeTraversal, DrawWireframeState> BaseClass;

    DrawWireframeTraversal(
        DrawWireframeState&     state,
        const MMatrix&          xform,
        bool                    isReflection,
        Frustum::ClippingResult parentClippingResult)
        : BaseClass(state, xform, isReflection, parentClippingResult)
    {}

    void draw(const boost::shared_ptr<const ShapeSample>& sample)
    {
        if (!sample->visibility()) return;
        gGLFT->glLoadMatrixd(xform().matrix[0]);

        if (sample->isBoundingBoxPlaceHolder()) {
            state().vboProxy().drawBoundingBox(sample);
            GlobalReaderCache::theCache().hintShapeReadOrder(subNode());
            return;
        }

        assert(sample->positions());
        assert(sample->normals());


        state().vboProxy().drawWireframe(sample);
    }
};

}


namespace GPUCache {

//==============================================================================
// CLASS DrawOverride::UserData
//==============================================================================

class DrawOverride::UserData : public MUserData
{
public:

    /*----- member functions -----*/

    UserData(const ShapeNode* node)
        : MUserData(false),
          fShapeNode(node),
          fSeconds(0.0),
          fIsSelected(false)
    {
        fWireframeColor[0] = 1.0f;
        fWireframeColor[1] = 1.0f;
        fWireframeColor[2] = 1.0f;
    }

    void set(const double& seconds,
             const MColor& wireframeColor,
             bool          isSelected)
    {
        fSeconds = seconds;

        fWireframeColor[0] = wireframeColor[0];
        fWireframeColor[1] = wireframeColor[1];
        fWireframeColor[2] = wireframeColor[2];

        fIsSelected = isSelected;
    }


    void draw(const MHWRender::MDrawContext& context) const;


private:

    /*----- member functions -----*/

    void drawShadedSampleGL(
        const Frustum&                              frustum,
        MHWRender::MStateManager*                   stateMgr,
        MHWRender::MRasterizerState::CullMode       cullMode,
        const DepthOffsetType                       depthOffsetType,
        const ColorType                             colorType,
        const MColor&                               defaultDiffuseColor,
        const NormalsType                           normalsType,
        const DrawShadedState::TransparentPruneType transparentPrune,
        const MMatrix&                              xform,
        const SubNode::Ptr&                         rootNode) const;

    void drawWireframeSampleGL(const Frustum&  frustum,
                               const MMatrix&  xform,
                               const SubNode::Ptr& rootNode) const;

    void drawBoundingBoxSampleGL(const MMatrix&      xform,
                                 const SubNode::Ptr& rootNode) const;

    // Returns true if any lights exists and false otherwise.
    bool setupLightingGL(const MHWRender::MDrawContext& context) const;
    void unsetLightingGL(const MHWRender::MDrawContext& context) const;

    virtual ~UserData()    {}


    /*----- data members -----*/

    const ShapeNode* fShapeNode;
    double           fSeconds;
    GLfloat          fWireframeColor[3];
    bool             fIsSelected;
};


//------------------------------------------------------------------------------
//
void DrawOverride::UserData::drawShadedSampleGL(
    const Frustum&                              frustum,
    MHWRender::MStateManager*                   stateMgr,
    MHWRender::MRasterizerState::CullMode       cullMode,
    const DepthOffsetType                       depthOffsetType,
    const ColorType                             colorType,
    const MColor&                               defaultDiffuseColor,
    const NormalsType                           normalsType,
    const DrawShadedState::TransparentPruneType transparentPrune,
    const MMatrix&                              xform,
    const SubNode::Ptr&                         rootNode) const
{
    DrawShadedState state(
        frustum, fSeconds, transparentPrune, stateMgr,
        cullMode, depthOffsetType, colorType,
        defaultDiffuseColor, normalsType);

    DrawShadedTraversal visitor(
        state, xform, xform.det3x3() < 0.0, Frustum::kUnknown);

    rootNode->accept(visitor);
}


//------------------------------------------------------------------------------
//
void DrawOverride::UserData::drawWireframeSampleGL(
    const Frustum&  frustum,
    const MMatrix&  xform,
    const SubNode::Ptr& rootNode) const
{
    DrawWireframeState state(frustum, fSeconds);
    DrawWireframeTraversal visitor(state, xform, false, Frustum::kUnknown);
    rootNode->accept(visitor);
}

//------------------------------------------------------------------------------
//
void DrawOverride::UserData::drawBoundingBoxSampleGL(
    const MMatrix&      xform,
    const SubNode::Ptr& rootNode) const
{
    // Get the bounding box.
    MBoundingBox boundingBox;

    const SubNodeData::Ptr subNodeData = rootNode->getData();
    if (!subNodeData) return;

    const XformData::Ptr xformData =
        boost::dynamic_pointer_cast<const XformData>(subNodeData);
    if (xformData) {
        const boost::shared_ptr<const XformSample>& sample =
            xformData->getSample(fSeconds);
        if (!sample || !sample->visibility()) return;
        boundingBox = sample->boundingBox();
    }
    else {
        const ShapeData::Ptr shapeData =
            boost::dynamic_pointer_cast<const ShapeData>(subNodeData);
        if (shapeData) {
            const boost::shared_ptr<const ShapeSample>& sample =
                shapeData->getSample(fSeconds);
            if (!sample || !sample->visibility()) return;
            boundingBox = sample->boundingBox();
        }
    }

    // Draw the bounding box.
    gGLFT->glLoadMatrixd(xform.matrix[0]);

    VBOProxy vboProxy;
    vboProxy.drawBoundingBox(boundingBox);
}


//------------------------------------------------------------------------------
//
bool DrawOverride::UserData::setupLightingGL(
    const MHWRender::MDrawContext& context) const
{
    MStatus status;

    // Take into account only the 8 lights supported by the basic
    // OpenGL profile.
    const unsigned int nbLights =
        std::min(context.numberOfActiveLights(&status), 8u);
    if (status != MStatus::kSuccess) return false;

    if (nbLights > 0) {
        // Lights are specified in world space and needs to be
        // converted to view space.
        const MMatrix worldToView =
            context.getMatrix(MHWRender::MFrameContext::kViewMtx, &status);
        if (status != MStatus::kSuccess) return false;
        gGLFT->glLoadMatrixd(worldToView.matrix[0]);

        gGLFT->glEnable(MGL_LIGHTING);
        gGLFT->glColorMaterial(MGL_FRONT_AND_BACK, MGL_AMBIENT_AND_DIFFUSE);
        gGLFT->glEnable(MGL_COLOR_MATERIAL) ;
        gGLFT->glEnable(MGL_NORMALIZE) ;

        {
            const MGLfloat ambient[4]  = { 0.0f, 0.0f, 0.0f, 1.0f };
            const MGLfloat specular[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

            gGLFT->glMaterialfv(MGL_FRONT_AND_BACK, MGL_AMBIENT,  ambient);
            gGLFT->glMaterialfv(MGL_FRONT_AND_BACK, MGL_SPECULAR, specular);

            gGLFT->glLightModelfv(MGL_LIGHT_MODEL_AMBIENT, ambient);

            // Two sided-lighting seems is always enabled in VP2.0.
            if (Config::emulateTwoSidedLighting()) {
                gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE, 0);
            }
            else {
                gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE, 1);
            }
        }

        for (unsigned int i=0; i<nbLights; ++i) {
            MFloatPointArray positions;
            MFloatVector direction;
            float intensity;
            MColor color;
            bool hasDirection;
            bool hasPosition;
            status = context.getLightInformation(
                i, positions, direction, intensity, color,
                hasDirection, hasPosition);
            if (status != MStatus::kSuccess) return false;

            // if (hasPosition)
            //     fprintf(stderr, "   -> Light%d position  = (%lf, %lf, %lf, %lf)\n",
            //             i, position[0], position[1], position[2], position[3]);
            // if (hasDirection)
            //     fprintf(stderr, "   -> Light%d direction = (%lf, %lf, %lf) - %lf\n",
            //             i, direction[0], direction[1], direction[2], direction.length());
            // fprintf(stderr, "   -> Light%d color = %lf x (%lf, %lf, %lf, %lf)\n",
            //         i, intensity, color[0], color[1], color[2], color[3]);

            if (hasDirection) {
                if (hasPosition) {
                    // Assumes a Maya Spot Light!
                    MFloatPoint position;
                    position[0] = positions[0][0];
                    position[1] = positions[0][1];
                    position[2] = positions[0][2];
                    const MGLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                    const MGLfloat diffuse[4] = { intensity * color[0],
                                                  intensity * color[1],
                                                  intensity * color[2],
                                                  1.0f };
                    const MGLfloat pos[4] = { position[0],
                                              position[1],
                                              position[2],
                                              1.0f };
                    const MGLfloat dir[3] = { direction[0],
                                              direction[1],
                                              direction[2]};


                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_AMBIENT,  ambient);
                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_DIFFUSE,  diffuse);
                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_POSITION, pos);
                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_SPOT_DIRECTION, dir);

                    // Maya's default value's for spot lights.
                    gGLFT->glLightf(MGL_LIGHT0+i,  MGL_SPOT_EXPONENT, 0.0);
                    gGLFT->glLightf(MGL_LIGHT0+i,  MGL_SPOT_CUTOFF,  20.0);
                }
                else {
                    // Assumes a Maya Directional Light!
                    const MGLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                    const MGLfloat diffuse[4] = { intensity * color[0],
                                                  intensity * color[1],
                                                  intensity * color[2],
                                                  1.0f };
                    const MGLfloat pos[4] = { -direction[0],
                                              -direction[1],
                                              -direction[2],
                                              0.0f };


                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_AMBIENT,  ambient);
                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_DIFFUSE,  diffuse);
                    gGLFT->glLightfv(MGL_LIGHT0+i, MGL_POSITION, pos);
                    gGLFT->glLightf(MGL_LIGHT0+i, MGL_SPOT_CUTOFF, 180.0);
                }
            }
            else if (hasPosition) {
                // Assumes a Maya Point Light!
                MFloatPoint position;
                position[0] = positions[0][0];
                position[1] = positions[0][1];
                position[2] = positions[0][2];
                const MGLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                const MGLfloat diffuse[4] = { intensity * color[0],
                                              intensity * color[1],
                                              intensity * color[2],
                                              1.0f };
                const MGLfloat pos[4] = { position[0],
                                          position[1],
                                          position[2],
                                          1.0f };


                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_AMBIENT,  ambient);
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_DIFFUSE,  diffuse);
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_POSITION, pos);
                gGLFT->glLightf(MGL_LIGHT0+i, MGL_SPOT_CUTOFF, 180.0);
            }
            else {
                // Assumes a Maya Ambient Light!
                const MGLfloat ambient[4] = { intensity * color[0],
                                              intensity * color[1],
                                              intensity * color[2],
                                              1.0f };
                const MGLfloat diffuse[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                const MGLfloat pos[4] = { 0.0f, 0.0f, 0.0f, 1.0f };


                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_AMBIENT,  ambient);
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_DIFFUSE,  diffuse);
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_POSITION, pos);
                gGLFT->glLightf(MGL_LIGHT0+i, MGL_SPOT_CUTOFF, 180.0);
            }

            gGLFT->glEnable(MGL_LIGHT0+i);
        }
    }

    return nbLights > 0;
}


//------------------------------------------------------------------------------
//
void DrawOverride::UserData::unsetLightingGL(
    const MHWRender::MDrawContext& context) const
{
    MStatus status;

    // Take into account only the 8 lights supported by the basic
    // OpenGL profile.
    const unsigned int nbLights =
        std::min(context.numberOfActiveLights(&status), 8u);
    if (status != MStatus::kSuccess) return;

    // Restore OpenGL default values for anything that we have
    // modified.

    if (nbLights > 0) {
        for (unsigned int i=0; i<nbLights; ++i) {
            gGLFT->glDisable(MGL_LIGHT0+i);

            const MGLfloat ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
            gGLFT->glLightfv(MGL_LIGHT0+i, MGL_AMBIENT,  ambient);

            if (i==0) {
                const MGLfloat diffuse[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_DIFFUSE,  diffuse);

                const MGLfloat spec[4]    = { 1.0f, 1.0f, 1.0f, 1.0f };
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_SPECULAR, spec);
            }
            else {
                const MGLfloat diffuse[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_DIFFUSE,  diffuse);

                const MGLfloat spec[4]    = { 0.0f, 0.0f, 0.0f, 1.0f };
                gGLFT->glLightfv(MGL_LIGHT0+i, MGL_SPECULAR, spec);
            }

            const MGLfloat pos[4]     = { 0.0f, 0.0f, 1.0f, 0.0f };
            gGLFT->glLightfv(MGL_LIGHT0+i, MGL_POSITION, pos);

            const MGLfloat dir[3]     = { 0.0f, 0.0f, -1.0f };
            gGLFT->glLightfv(MGL_LIGHT0+i, MGL_SPOT_DIRECTION, dir);

            gGLFT->glLightf(MGL_LIGHT0+i,  MGL_SPOT_EXPONENT,  0.0);
            gGLFT->glLightf(MGL_LIGHT0+i,  MGL_SPOT_CUTOFF,  180.0);
        }

        gGLFT->glDisable(MGL_LIGHTING);
        gGLFT->glDisable(MGL_COLOR_MATERIAL) ;
        gGLFT->glDisable(MGL_NORMALIZE) ;

        const MGLfloat ambient[4]  = { 0.2f, 0.2f, 0.2f, 1.0f };
        const MGLfloat specular[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

        gGLFT->glMaterialfv(MGL_FRONT_AND_BACK, MGL_AMBIENT,  ambient);
        gGLFT->glMaterialfv(MGL_FRONT_AND_BACK, MGL_SPECULAR, specular);

        gGLFT->glLightModelfv(MGL_LIGHT_MODEL_AMBIENT, ambient);
        gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE, 0);
    }
}


//------------------------------------------------------------------------------
//
void DrawOverride::UserData::draw(
    const MHWRender::MDrawContext& context) const
{
    using namespace MHWRender;
    MStatus status;

    // Extract the cached geometry.
    const SubNode::Ptr& rootNode =
        fShapeNode->getCachedGeometry();
    if (!rootNode) return;

    // get renderer
    MRenderer* theRenderer = MRenderer::theRenderer();
    if (!theRenderer) return;

    MStateManager* stateMgr = context.getStateManager();
    if (!stateMgr) return;

    const unsigned int displayStyle = context.getDisplayStyle();
    if (displayStyle == 0) return;

    // Sample code to debug pass information
    static const bool debugPassInformation = false;
    if (debugPassInformation)
    {
        const MHWRender::MPassContext & passCtx = context.getPassContext();
        const MString & passId = passCtx.passIdentifier();
        const MStringArray & passSem = passCtx.passSemantics();
        printf("gpuCache override in pass[%s], semantic[", passId.asChar());
        for (unsigned int i=0; i<passSem.length(); i++)
            printf(" %s", passSem[i].asChar());
        printf("\n");
    }

    if (displayStyle & MFrameContext::kXray) {
        // Viewport 2.0 will call draw() twice when drawing transparent objects
        // (X-Ray mode). We skip the first draw() call.
        const MRasterizerState* rasterState = stateMgr->getRasterizerState();
        if (rasterState && rasterState->desc().cullMode == MRasterizerState::kCullFront) {
            return;
        }
    }

    // View frustum culling.
    const MMatrix worldViewProjInvMatrix =
        context.getMatrix(MFrameContext::kWorldViewProjInverseMtx, &status);
    if (status != MStatus::kSuccess) return;

    Frustum frustum( worldViewProjInvMatrix);
    {
        TopLevelCullVisitor visitor(frustum, fSeconds);
        rootNode->accept(visitor);
        if (visitor.isCulled()) return;
    }

    // get state data
    const MMatrix xform =
        context.getMatrix(MFrameContext::kWorldViewMtx, &status);
    if (status != MStatus::kSuccess) return;
    const MMatrix projection =
        context.getMatrix(MFrameContext::kProjectionMtx, &status);
    if (status != MStatus::kSuccess) return;

    // Save the current gfx state so that we can restore it later on!
    const MBlendState* savedBlendState           = stateMgr->getBlendState();
    const MRasterizerState* savedRasterizerState = stateMgr->getRasterizerState();
    const MDepthStencilState* savedDepthState    = stateMgr->getDepthStencilState();

    // GL Draw
    if (theRenderer->drawAPIIsOpenGL())
    {
        // set projection matrix
        gGLFT->glMatrixMode(MGL_PROJECTION);
        gGLFT->glPushMatrix();
        gGLFT->glLoadMatrixd(projection.matrix[0]);

        // set world matrix
        gGLFT->glMatrixMode(MGL_MODELVIEW);
        gGLFT->glPushMatrix();

        // Bounding Box
        if (displayStyle & MFrameContext::kBoundingBox)
        {
            setWireframeState(stateMgr);

            // set colour
            gGLFT->glColor3fv(fWireframeColor);

            // set style
            gGLFT->glEnable(MGL_LINE_STIPPLE);
            gGLFT->glLineStipple(1, Config::kLineStippleShortDashed);

            drawBoundingBoxSampleGL(xform, rootNode);

            gGLFT->glDisable(MGL_LINE_STIPPLE);
        }

        const bool needWireframe =
            !(displayStyle & MFrameContext::kBoundingBox) &&
            (displayStyle & MFrameContext::kWireFrame || fIsSelected);
        const bool wireframeOnShaded = needWireframe &&
            (displayStyle & MFrameContext::kGouraudShaded);
        const bool disableWireframeOnShaded = wireframeOnShaded &&
            (DisplayPref::wireframeOnShadedMode() == DisplayPref::kWireframeOnShadedNone);

        // Wireframe can be considerered as being opaque and therefore
        // must be drawn before any transparent object.
        if (needWireframe && !disableWireframeOnShaded)
        {
            setWireframeState(stateMgr);

            // set colour
            gGLFT->glColor3fv(fWireframeColor);

            gGLFT->glEnable(MGL_LINE_STIPPLE);
            if (wireframeOnShaded) {
                // Wireframe on shaded is affected by wireframe on shaded mode
                DisplayPref::WireframeOnShadedMode wireframeOnShadedMode =
                    DisplayPref::wireframeOnShadedMode();

                if (wireframeOnShadedMode == DisplayPref::kWireframeOnShadedReduced) {
                    gGLFT->glLineStipple(1, Config::kLineStippleDotted);
                }
                else {
                    assert(wireframeOnShadedMode != DisplayPref::kWireframeOnShadedNone);
                    gGLFT->glLineStipple(1, Config::kLineStippleShortDashed);
                }
            }
            else {
                gGLFT->glLineStipple(1, Config::kLineStippleShortDashed);
            }

            drawWireframeSampleGL(frustum, xform, rootNode);

            gGLFT->glDisable( MGL_LINE_STIPPLE );
        }

        if (displayStyle & MFrameContext::kGouraudShaded)
        {
            // When we need to draw both the shaded geometry and the
            // wireframe mesh, we need to offset the shaded geometry
            // in depth to avoid Z-fighting against the wireframe
            // mesh.
            //
            // On the hand, we don't want to use depth offset when
            // drawing only the shaded geometry because it leads to
            // some drawing artifacts. The reason is a litle bit
            // subtle. At silouhette edges, both front-facing and
            // back-facing faces are meeting. These faces can have a
            // different slope in Z and this can lead to a different
            // Z-offset being applied. When unlucky, the back-facing
            // face can be drawn in front of the front-facing face. If
            // two-sided lighting is enabled, the back-facing fragment
            // can have a different resultant color. This can lead to
            // a rim of either dark or bright pixels around silouhette
            // edges.
            //
            // When the wireframe mesh is drawn on top (even a dotted
            // one), it masks this effect sufficiently that it is no
            // longer distracting for the user, so it is OK to use
            // depth offset when the wireframe mesh is drawn on top.
            const DepthOffsetType depthOffsetType = needWireframe ?
                kApplyDepthOffset : kNoDepthOffset;

            // Set up OpenGL lights
            const bool anyLights = setupLightingGL(context);

            // Determine the diffuse color
            ColorType colorType = kSubNodeColor;
            MColor    defaultDiffuseColor;

            bool needOpaquePass      =
                rootNode->transparentType() != SubNode::kTransparent;
            bool needTransparentPass =
                rootNode->transparentType() != SubNode::kOpaque;

            DrawShadedState::TransparentPruneType opaquePassPrune =
                DrawShadedState::kPruneTransparent;
            DrawShadedState::TransparentPruneType transparentPassPrune =
                DrawShadedState::kPruneOpaque;

            if (displayStyle & MFrameContext::kDefaultMaterial) {
                // Force drawing as opaque gray if use default material
                colorType           = kDefaultColor;
                needOpaquePass      = true;
                needTransparentPass = false;
                opaquePassPrune     = DrawShadedState::kPruneNone;
                if (anyLights) {
                    defaultDiffuseColor = Config::kDefaultGrayColor;
                }
            }
            else if (displayStyle & MFrameContext::kXray) {
                // Force drawing as transparent in X-Ray mode
                needOpaquePass       = false;
                needTransparentPass  = true;
                transparentPassPrune = DrawShadedState::kPruneNone;
                colorType = anyLights ? kXrayColor : kXrayBlackColor;
            }
            else
            if (!anyLights) {
                // Force drawing as black if no light exists in the scene.
                colorType = kBlackColor;
            }

            setShadedBlendState(stateMgr);

            // Opaque pass
            if (needOpaquePass) {
                setShadedSolidDepthState(stateMgr);

                if (displayStyle & MFrameContext::kTwoSidedLighting) {
                    // Two-sided lighting
                    if (Config::emulateTwoSidedLighting()) {
                        setShadedTwoSidedLightingState(kOneSidedLighting);

                        drawShadedSampleGL(
                            frustum, stateMgr, MRasterizerState::kCullFront,
                            depthOffsetType, colorType, defaultDiffuseColor,
                            kBackNormals, opaquePassPrune, xform, rootNode);

                        drawShadedSampleGL(
                            frustum, stateMgr, MRasterizerState::kCullBack,
                            depthOffsetType, colorType, defaultDiffuseColor,
                            kFrontNormals, opaquePassPrune, xform, rootNode);
                    }
                    else {
                        setShadedTwoSidedLightingState(kTwoSidedLighting);

                        drawShadedSampleGL(
                            frustum, stateMgr, MRasterizerState::kCullNone,
                            depthOffsetType, colorType, defaultDiffuseColor,
                            kFrontNormals, opaquePassPrune, xform, rootNode);
                    }
                }
                else {
                    // One-sided lighting
                    setShadedTwoSidedLightingState(kOneSidedLighting);

                    drawShadedSampleGL(
                        frustum, stateMgr, MRasterizerState::kCullNone,
                        depthOffsetType, colorType, defaultDiffuseColor,
                        kFrontNormals, opaquePassPrune, xform, rootNode);
                }
            }

            // Transparent pass
            if (needTransparentPass) {
                setShadedAlphaDepthState(stateMgr);

                if (displayStyle & MFrameContext::kTwoSidedLighting) {
                    // Two-sided lighting
                    setShadedTwoSidedLightingState(
                           Config::emulateTwoSidedLighting() ?
                           kOneSidedLighting : kTwoSidedLighting );

                    drawShadedSampleGL(
                        frustum, stateMgr, MRasterizerState::kCullFront,
                        depthOffsetType, colorType, defaultDiffuseColor,
                        Config::emulateTwoSidedLighting() ? kBackNormals : kFrontNormals,
                        transparentPassPrune, xform, rootNode);
                    drawShadedSampleGL(
                        frustum, stateMgr, MRasterizerState::kCullBack,
                        depthOffsetType, colorType, defaultDiffuseColor,
                        kFrontNormals,  transparentPassPrune, xform, rootNode);
                }
                else {
                    // One-sided lighting
                    setShadedTwoSidedLightingState(kOneSidedLighting);

                    drawShadedSampleGL(
                        frustum, stateMgr, MRasterizerState::kCullFront,
                        depthOffsetType, colorType, defaultDiffuseColor,
                        kFrontNormals, transparentPassPrune, xform, rootNode);
                    drawShadedSampleGL(
                        frustum, stateMgr, MRasterizerState::kCullBack,
                        depthOffsetType, colorType, defaultDiffuseColor,
                        kFrontNormals, transparentPassPrune, xform, rootNode);
                }
            }

            unsetLightingGL(context);
        }

        // Bring the OpenGL state back to the VP2.0 expected default.

        // Restore the default color.
        gGLFT->glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        // Restore OGS default value
        gGLFT->glFrontFace(GL_CCW);

        // Restore OGS default two-sided lighting state.
        gGLFT->glLightModeli(MGL_LIGHT_MODEL_TWO_SIDE, 1);

        gGLFT->glMatrixMode(MGL_PROJECTION);
        gGLFT->glPopMatrix();
        gGLFT->glMatrixMode(MGL_MODELVIEW);
        gGLFT->glPopMatrix();

    }
    else {
        // TODO: To be implemented!
    }

    stateMgr->setBlendState(savedBlendState);
    stateMgr->setRasterizerState(savedRasterizerState);
    stateMgr->setDepthStencilState(savedDepthState);

	stateMgr->releaseBlendState(savedBlendState);
	stateMgr->releaseRasterizerState(savedRasterizerState);
	stateMgr->releaseDepthStencilState(savedDepthState);
}


//==============================================================================
// CLASS DrawOverride
//==============================================================================


//------------------------------------------------------------------------------
//
void DrawOverride::drawCb(
    const MHWRender::MDrawContext& context,
    const MUserData* userData)

{
    // Make sure that the post render callbacks have been properly
    // initialized. We have to verify at each refresh because there is
    // no easy way to recieve a callback when a new modelEditor is
    // created.
    ShapeNode::init3dViewPostRenderCallbacks();

    InitializeGLFT();
    const UserData* data = dynamic_cast<const UserData*>(userData);
    if (data) data->draw(context);
}


//------------------------------------------------------------------------------
//
MHWRender::MPxDrawOverride*
DrawOverride::creator(const MObject& obj)
{
    return new DrawOverride(obj);
}


//------------------------------------------------------------------------------
//
DrawOverride::DrawOverride(const MObject& obj)
    : MPxDrawOverride(obj, &drawCb)
{}


//------------------------------------------------------------------------------
//
DrawOverride::~DrawOverride()
{}


//------------------------------------------------------------------------------
//
MHWRender::DrawAPI DrawOverride::supportedDrawAPIs() const
{
    // This draw override supports only OpenGL for now.
    return MHWRender::kOpenGL;
}


//------------------------------------------------------------------------------
//
bool DrawOverride::isBounded(const MDagPath& /*objPath*/,
                             const MDagPath& /*cameraPath*/) const
{
    return true;
}


//------------------------------------------------------------------------------
//
MBoundingBox DrawOverride::boundingBox(
    const MDagPath& objPath,
    const MDagPath& cameraPath) const
{
    // Extract the cached geometry.
    MStatus status;
    MFnDependencyNode node(objPath.node(), &status);
    if (!status) return MBoundingBox();

    const ShapeNode* shapeNode = dynamic_cast<ShapeNode*>(node.userNode());
    if (!shapeNode) return MBoundingBox();

    const SubNode::Ptr subNode = shapeNode->getCachedGeometry();
    if (!subNode) return MBoundingBox();

    const SubNodeData::Ptr subNodeData = subNode->getData();
    if (!subNodeData) return MBoundingBox();

    const double seconds = MAnimControl::currentTime().as(MTime::kSeconds);

    // Handle transforms.
    const XformData::Ptr xform =
        boost::dynamic_pointer_cast<const XformData>(subNodeData);
    if (xform) {
        const boost::shared_ptr<const XformSample>& sample =
            xform->getSample(seconds);
        return sample->boundingBox();
    }

    // Handle shapes.
    const ShapeData::Ptr shape =
        boost::dynamic_pointer_cast<const ShapeData>(subNodeData);
    if (shape) {
        const boost::shared_ptr<const ShapeSample>& sample =
            shape->getSample(seconds);
        return sample->boundingBox();
    }

    return MBoundingBox();
}


//------------------------------------------------------------------------------
//
bool DrawOverride::disableInternalBoundingBoxDraw() const
{
    // Always return true since we will perform custom bounding box
    // drawing
    return true;
}


//------------------------------------------------------------------------------
//
MUserData* DrawOverride::prepareForDraw(
    const MDagPath& objPath,
    const MDagPath& cameraPath,
    const MHWRender::MFrameContext& frameContext,
    MUserData* oldData)
{
    using namespace MHWRender;

    MObject object    = objPath.node();
    MObject transform = objPath.transform();

    // Retrieve data cache (create if does not exist)
    UserData* data = dynamic_cast<UserData*>(oldData);
    if (!data)
    {
        // get the real ShapeNode from the MObject
        MStatus status;
        MFnDependencyNode node(object, &status);
        if (status)
        {
            data = new UserData(
                dynamic_cast<ShapeNode*>(node.userNode()));
        }
    }

    if (data) {
        // compute data and cache it

        const MColor wireframeColor =
            MGeometryUtilities::wireframeColor(objPath);

        const DisplayStatus displayStatus =
            MGeometryUtilities::displayStatus(objPath);
        const bool isSelected =
            (displayStatus == kActive) ||
            (displayStatus == kLead)   ||
            (displayStatus == kHilite);

        data->set(
            MAnimControl::currentTime().as(MTime::kSeconds),
            wireframeColor,
            isSelected);
    }

    return data;
}

}
