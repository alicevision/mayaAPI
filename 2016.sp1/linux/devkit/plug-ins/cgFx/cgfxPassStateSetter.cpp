//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include <maya/MDrawContext.h>
#include <maya/MGlobal.h>
#include "cgfxPassStateSetter.h"
#include "cgfxShaderNode.h"

#include <string>
#include <map>
#include <set>

using namespace MHWRender;

// The GL enumerants for stencil wrap are not defined on every
// platforms.
#ifndef GL_INCR_WRAP
#define GL_INCR_WRAP                      0x8507
#endif

#ifndef GL_DECR_WRAP
#define GL_DECR_WRAP                      0x8508
#endif

namespace
{

    std::string lowerCaseString(const char* str) {
        std::string lower;
        lower.reserve(strlen(str));
        for (const char* p=str; *p != 0; ++p) {
            lower += char(tolower(*p));
        }
        return lower;
    }

    //==============================================================================
    // CLASS StateAssignment
    //==============================================================================

    class StateAssignment
    {
    public:
        typedef void (*HandlerFn)(
            const CGstateassignment&    sa,
            MBlendStateDesc*            blendStateDesc,
            MRasterizerStateDesc*       rasterizerStateDesc,
            MDepthStencilStateDesc*     depthStencilStateDesc);

        // Returns whether the state assignement is unsupported by the
        // VP2.0 viewport and therefore a glPushAttrib()/glPopAttrib()
        // pair is required to ensure correct rendering.
        typedef bool (*UnsupportedCheckFn)(const CGstateassignment& sa);

        static void registerHandler(const char* stateName, HandlerFn handler) {
            CGstate cgState = cgGetNamedState(cgfxShaderNode::sCgContext, stateName);
            if (cgState) {
                std::string lstateName = lowerCaseString(stateName);
                const bool inserted =
                    map.insert(
                        std::make_pair(lstateName,StateAssignment(cgState, handler))
                    ).second;
                if (!inserted) {
                    MString s;
                    s += "cgfxPassStateSetter is trying to register multiple "
                        "handlers for the same CgFX state \"";
                    s += stateName;
                    s += "\".";
                    MGlobal::displayError(s);
                }
            }
            else {
                MString s;
                s += "The CgFX state assignment \"";
                s += stateName;
                s += "\" is unknown to the Cg library.";
                MGlobal::displayError(s);
            }
        }

        static void registerUnsupportedCheck(const char* stateName, UnsupportedCheckFn check) {
            CGstate cgState = cgGetNamedState(cgfxShaderNode::sCgContext, stateName);
            if (cgState) {
                std::string lstateName = lowerCaseString(stateName);
                const bool inserted =
                    unsupportedChecks.insert(
                        std::make_pair(lstateName,check)
                    ).second;
                if (!inserted) {
                    MString s;
                    s += "cgfxPassStateSetter is trying to register multiple "
                        "unsupported checks for the same CgFX state \"";
                    s += stateName;
                    s += "\".";
                    MGlobal::displayError(s);
                }
            }
            else {
                MString s;
                s += "The CgFX state assignment \"";
                s += stateName;
                s += "\" is unknown to the Cg library.";
                MGlobal::displayError(s);
            }
        }

        static void registerIgnoredState(const char* stateName) {
            CGstate cgState = cgGetNamedState(cgfxShaderNode::sCgContext, stateName);
            if (cgState) {
                const bool inserted =
                    ignoredSet.insert(lowerCaseString(stateName)).second;
                if (!inserted) {
                    MString s;
                    s += "cgfxPassStateSetter is trying to register multiple "
                        "ignore handlers for the same CgFX state \"";
                    s += stateName;
                    s += "\".";
                    MGlobal::displayError(s);
                }
            }
            else {
                MString s;
                s += "The CgFX state assignment \"";
                s += stateName;
                s += "\" is unknown to the Cg library.";
                MGlobal::displayError(s);
            }
        }

        static void registerDefaultCallBacks()
        {
            // The default viewport uses immediate parameter setting.
            cgSetParameterSettingMode(cgfxShaderNode::sCgContext,
                                      CG_IMMEDIATE_PARAMETER_SETTING);

            const Map::const_iterator itEnd = map.end();
            for ( Map::const_iterator it    = map.begin();
                  it != itEnd; ++it) {
                it->second.setDefaultCallbacks();
            }
        }

        static void registerVP20CallBacks()
        {
            // The VP2.0 viewport uses deferred parameter setting.
            cgSetParameterSettingMode(cgfxShaderNode::sCgContext,
                                      CG_DEFERRED_PARAMETER_SETTING);

            const Map::const_iterator itEnd = map.end();
            for ( Map::const_iterator it    = map.begin();
                  it != itEnd; ++it) {
                it->second.setVP20Callbacks();
            }
        }

        // Invokes the proper handler for given state assignment. The
        // state objects will be modified accordingly if such a
        // handler exists.
        //
        // Returns whether a glPushAttib()/glPopAttrib() is required
        // due to an unsupported state assignement.
        static bool callHandler(
            const CGstateassignment&            sa,
            MBlendStateDesc*         blendStateDesc,
            MRasterizerStateDesc*    rasterizerStateDesc,
            MDepthStencilStateDesc*  depthStencilStateDesc)
        {
            // We modify the state objects according to the sa
            CGstate state = cgGetStateAssignmentState(sa);
            const char *stateName = cgGetStateName(state);
            std::string lstateName = lowerCaseString(stateName);

            {
                const Map::const_iterator handler = map.find(lstateName);
                if (handler != map.end()) {
                    handler->second.fHandler(sa,
                                             blendStateDesc,
                                             rasterizerStateDesc,
                                             depthStencilStateDesc);
                    return false;
                }
            }

            {
                const UnsupportedChecks::const_iterator unsupportedCheck =
                    unsupportedChecks.find(lstateName);
                if (unsupportedCheck != unsupportedChecks.end()) {
                    return unsupportedCheck->second(sa);
                }
            }

            if (ignoredSet.find(lstateName) != ignoredSet.end()) {
                // This state assignement requires no specific
                // handling is therefore ignored by the
                // cgfxPassStateSetter. I.e. we let Cg perform its
                // default behavior in that case.
                return false;
            }

#ifdef DEBUG
            // Unsupported state. We must use a
            // glPushAttrib()/glPopAttrib() pair to guarantee correct
            // rendering.
            MString s;
            s += "cgfxShader: The CgFX state assignment \"";
            s += stateName;
            s += "\" contained in the shader file is not accelarated by the "
                "cgfxShader plugin in a VP2.0 viewport and will results in lower "
                "performance.";
            MGlobal::displayWarning(s);
#endif

            return true;
        }

    private:

        typedef std::map<std::string, StateAssignment> Map;
        typedef std::map<std::string, UnsupportedCheckFn> UnsupportedChecks;
        typedef std::set<std::string> IgnoredSet;

        friend class std::map<std::string, StateAssignment>;

        static CGbool noopStateAssignment(CGstateassignment sa) {
            return CG_TRUE;
        }

        StateAssignment(CGstate cgState, HandlerFn handler) {
            fCgState           = cgState;
            fDefaultSetCB      = cgGetStateSetCallback(cgState);
            fDefaultResetCB    = cgGetStateResetCallback(cgState);
            fDefaultValidateCB = cgGetStateValidateCallback(cgState);
            fHandler           = handler;
        }

        // Need a default ctor to please std::map...
        StateAssignment()
            : fCgState(NULL),
              fDefaultSetCB(NULL),
              fDefaultResetCB(NULL),
              fDefaultValidateCB(NULL),
              fHandler(NULL)
        {};

        void setDefaultCallbacks() const
        {
            cgSetStateCallbacks(fCgState,
                                fDefaultSetCB,
                                fDefaultResetCB,
                                fDefaultValidateCB);
        }

        void setVP20Callbacks() const
        {
            cgSetStateCallbacks(fCgState,
                                noopStateAssignment,
                                noopStateAssignment,
                                NULL);
        }

        static Map                  map;
        static UnsupportedChecks    unsupportedChecks;
        static IgnoredSet           ignoredSet;

        CGstate         fCgState;
        CGstatecallback fDefaultSetCB;
        CGstatecallback fDefaultResetCB;
        CGstatecallback fDefaultValidateCB;
        HandlerFn       fHandler;
    };

    StateAssignment::Map                StateAssignment::map;
    StateAssignment::UnsupportedChecks  StateAssignment::unsupportedChecks;
    StateAssignment::IgnoredSet         StateAssignment::ignoredSet;


    //==============================================================================
    //  OpenGL mappings
    //==============================================================================

    MBlendState::BlendOption mapBlendOption(int blendOption)
    {
        switch(blendOption) {
            case GL_ZERO:                     return MBlendState::kZero;
            case GL_ONE:                      return MBlendState::kOne;
            case GL_SRC_COLOR:                return MBlendState::kSourceColor;
            case GL_ONE_MINUS_SRC_COLOR:      return MBlendState::kInvSourceColor;
            case GL_SRC_ALPHA:                return MBlendState::kSourceAlpha;
            case GL_ONE_MINUS_SRC_ALPHA:      return MBlendState::kInvSourceAlpha;
            case GL_SRC_ALPHA_SATURATE:       return MBlendState::kSourceAlphaSat;
            case GL_DST_COLOR:                return MBlendState::kDestinationColor;
            case GL_ONE_MINUS_DST_COLOR:      return MBlendState::kInvDestinationColor;
            case GL_DST_ALPHA:                return MBlendState::kDestinationAlpha;
            case GL_ONE_MINUS_DST_ALPHA:      return MBlendState::kInvDestinationAlpha;
            case GL_CONSTANT_COLOR:           return MBlendState::kBlendFactor;
            case GL_ONE_MINUS_CONSTANT_COLOR: return MBlendState::kInvBlendFactor;

            //These two cases are duplicated in OGL
            // case GL_SRC_ALPHA:             return MBlendState::kBothSourceAlpha;
            // case GL_ONE_MINUS_SRC_ALPHA:   return MBlendState::kBothInvSourceAlpha;

            // Error, unknown GL enum.
            default:                          return MBlendState::kOne;
        }
    }

    MBlendState::BlendOperation mapBlendOperation(int blendOperation)
    {
        switch(blendOperation) {
            case GL_FUNC_ADD:              return MBlendState::kAdd;
            case GL_FUNC_SUBTRACT:         return MBlendState::kSubtract;
            case GL_FUNC_REVERSE_SUBTRACT: return MBlendState::kReverseSubtract;
            case GL_MIN:                   return MBlendState::kMin;
            case GL_MAX:                   return MBlendState::kMax;

            // Error, unknown GL enum.
            default:                       return MBlendState::kAdd;
        }
    }

    MStateManager::CompareMode mapCompareMode(int compareMode)
    {
        switch(compareMode)
        {
            case GL_NEVER:    return MStateManager::kCompareNever;
            case GL_LESS:     return MStateManager::kCompareLess;
            case GL_EQUAL:    return MStateManager::kCompareEqual;
            case GL_LEQUAL:	  return MStateManager::kCompareLessEqual;
            case GL_GREATER:  return MStateManager::kCompareGreater;
            case GL_NOTEQUAL: return MStateManager::kCompareNotEqual;
            case GL_GEQUAL:   return MStateManager::kCompareGreaterEqual;
            case GL_ALWAYS:	  return MStateManager::kCompareAlways;

            // Error, unknown GL enum.
            default:          return MStateManager::kCompareAlways;
        }
    }

    MDepthStencilState::StencilOperation mapStencilOperation(int stencilOp)
    {
        switch(stencilOp)
        {
            case GL_KEEP:      return MDepthStencilState::kKeepStencil;
            case GL_ZERO:      return MDepthStencilState::kZeroStencil;
            case GL_REPLACE:   return MDepthStencilState::kReplaceStencil;
            case GL_INCR:	   return MDepthStencilState::kIncrementStencilSat;
            case GL_DECR:      return MDepthStencilState::kDecrementStencilSat;
            case GL_INVERT:    return MDepthStencilState::kInvertStencil;
            case GL_INCR_WRAP: return MDepthStencilState::kIncrementStencil;
            case GL_DECR_WRAP: return MDepthStencilState::kDecrementStencil;

            // Error, unknown GL enum.
            default:           return MDepthStencilState::kKeepStencil;
        }
    }

    //==============================================================================
    //  STATE ASSIGNMENT HANDLERS
    //==============================================================================

    //  We currently support all of the states that the OpenMaya VP2.0
    //  API supports. The state assignments supported by CgFX but not
    //  by the OpenMaya VP2.0 API are not handled by the
    //  cgfxPassStateSetter and will be handled the the regular
    //  cgSetPassState mechanism. A warning is emitted when we
    //  encounter such a state because it is a potential performance
    //  bottleneck.

    MString WrongNumValues(
        "cgfxPassStateSetter : Incoherent number of state assignment values");

    //******************************************************************************
    // Blend states
    //******************************************************************************

    void assignHdlr_BlendColor(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const float* values = cgGetFloatStateAssignmentValues(sa, &numValues);
        if(numValues == 4) {
            blendStateDesc->blendFactor[0] = values[0];
            blendStateDesc->blendFactor[1] = values[1];
            blendStateDesc->blendFactor[2] = values[2];
            blendStateDesc->blendFactor[3] = values[3];
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_BlendEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            const bool value = (values[0] == CG_TRUE) ? true : false;
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].blendEnable = value;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_SrcBlend(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].sourceBlend =
                mapBlendOption(values[0]);
            blendStateDesc->targetBlends[0].alphaSourceBlend =
                mapBlendOption(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_DestBlend(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].destinationBlend =
                mapBlendOption(values[0]);
            blendStateDesc->targetBlends[0].alphaDestinationBlend =
                mapBlendOption(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_BlendFunc(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 2) {
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].sourceBlend =
                mapBlendOption(values[0]);
            blendStateDesc->targetBlends[0].destinationBlend =
                mapBlendOption(values[1]);
            blendStateDesc->targetBlends[0].alphaSourceBlend =
                mapBlendOption(values[0]);
            blendStateDesc->targetBlends[0].alphaDestinationBlend =
                mapBlendOption(values[1]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_BlendOp(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].blendOperation =
                mapBlendOperation(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_BlendFuncSeparate(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 4) {
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].sourceBlend =
                mapBlendOption(values[0]);
            blendStateDesc->targetBlends[0].destinationBlend =
                mapBlendOption(values[1]);
            blendStateDesc->targetBlends[0].alphaSourceBlend =
                mapBlendOption(values[2]);
            blendStateDesc->targetBlends[0].alphaDestinationBlend =
                mapBlendOption(values[3]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_BlendEquationSeparate(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 2) {
            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].blendOperation =
                mapBlendOperation(values[0]);
            blendStateDesc->targetBlends[1].alphaBlendOperation =
                mapBlendOperation(values[1]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_ColorWriteEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         blendStateDesc,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool* values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 4) {
            int mask = MBlendState::kNoChannels;

            mask |= (values[0]!=0) ? MBlendState::kRedChannel   : 0;
            mask |= (values[1]!=0) ? MBlendState::kGreenChannel : 0;
            mask |= (values[2]!=0) ? MBlendState::kBlueChannel  : 0;
            mask |= (values[3]!=0) ? MBlendState::kAlphaChannel : 0;

            // We always use the target 0 since the CgFX plugin only
            // renders to a single framebuffer.
            blendStateDesc->targetBlends[0].targetWriteMask =
                MBlendState::ChannelMask(mask);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }


    //******************************************************************************
    // Rasterizer states
    //******************************************************************************

    void assignHdlr_FillMode(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues >= 1) {
            switch(values[0])
            {
                case GL_FILL:  rasterizerStateDesc->fillMode = MRasterizerState::kFillSolid; break;
                case GL_LINE:  rasterizerStateDesc->fillMode = MRasterizerState::kFillWireFrame; break;
                case GL_POINT: //we don't have such enum in MRasteriszerState, so pass
                default:; //error
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_CullFace(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            switch(values[0])
            {
                case GL_FRONT: rasterizerStateDesc->cullMode = MRasterizerState::kCullFront; break;
                case GL_BACK:  rasterizerStateDesc->cullMode = MRasterizerState::kCullBack;  break;
                case GL_FRONT_AND_BACK:   //we don't have such enum in MRasteriszerState, so pass
                default:; //error
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_CullFaceEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            if(values[0] == CG_TRUE) {
                //use default cull mode if cull is enabled
                rasterizerStateDesc->cullMode = MRasterizerState::kCullBack;

            }
            else {
                rasterizerStateDesc->cullMode = MRasterizerState::kCullNone;
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_FrontFace(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            switch(values[0])
            {
                case GL_CW:  rasterizerStateDesc->frontCounterClockwise = false; break;
                case GL_CCW: rasterizerStateDesc->frontCounterClockwise = true; break;
                default:; //error
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_PolygonMode(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 2) {
            switch(values[0])
            {
                case GL_FRONT: rasterizerStateDesc->cullMode = MRasterizerState::kCullFront; break;
                case GL_BACK:  rasterizerStateDesc->cullMode = MRasterizerState::kCullBack;  break;
                case GL_FRONT_AND_BACK:   //we don't have such enum in MRasteriszerState, so pass
                default:; //error
            }
            switch(values[1])
            {
                case GL_FILL:  rasterizerStateDesc->fillMode = MRasterizerState::kFillSolid; break;
                case GL_LINE:  rasterizerStateDesc->fillMode = MRasterizerState::kFillWireFrame; break;
                case GL_POINT: //we don't have such enum in MRasteriszerState, so pass
                default:; //error
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_PolygonOffset(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const float *values = cgGetFloatStateAssignmentValues(sa, &numValues);
        if (numValues == 2) {
            rasterizerStateDesc->slopeScaledDepthBias = values[0];
            rasterizerStateDesc->depthBias = values[1];
            rasterizerStateDesc->depthBiasIsFloat = true;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_DepthBias(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const float *values = cgGetFloatStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            rasterizerStateDesc->depthBias = values[0];
            rasterizerStateDesc->depthBiasIsFloat = true;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_SlopScaleDepthBias(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const float *values = cgGetFloatStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            rasterizerStateDesc->slopeScaledDepthBias = values[0];
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_PolygonOffsetFillEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            rasterizerStateDesc->depthClipEnable = (values[0] == CG_TRUE) ? true : false;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_ScissorTestEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            rasterizerStateDesc->scissorEnable = (values[0] == CG_TRUE) ? true : false;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_MultisampleEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            rasterizerStateDesc->multiSampleEnable = (values[0] == CG_TRUE) ? true : false;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_LineSmoothEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    rasterizerStateDesc,
        MDepthStencilStateDesc*  /* depthStencilStateDesc */)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            rasterizerStateDesc->antialiasedLineEnable = (values[0] == CG_TRUE) ? true : false;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    //******************************************************************************
    // Depth and stencil states
    //******************************************************************************

    void assignHdlr_DepthTestEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            const bool value = (values[0] == CG_TRUE) ? true : false;
            depthStencilStateDesc->depthEnable = value;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_DepthMask(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            const bool value = (values[0] == CG_TRUE) ? true : false;
            depthStencilStateDesc->depthWriteEnable = value;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_DepthFunc(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if(numValues == 1) {
            depthStencilStateDesc->depthFunc = mapCompareMode(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilEnable(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            const bool value = (values[0] == CG_TRUE) ? true : false;
            depthStencilStateDesc->stencilEnable = value;
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilWriteMask(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1)
        {
            depthStencilStateDesc->stencilWriteMask = (unsigned char)values[0];
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilFunc(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            depthStencilStateDesc->frontFace.stencilFunc =
                mapCompareMode(values[0]);
            depthStencilStateDesc->backFace.stencilFunc =
                mapCompareMode(values[0]);
        }
        else if (numValues == 3)
        {
            depthStencilStateDesc->frontFace.stencilFunc =
                mapCompareMode(values[0]);
            depthStencilStateDesc->backFace.stencilFunc =
                mapCompareMode(values[0]);
            depthStencilStateDesc->stencilReferenceVal = values[1];
            depthStencilStateDesc->stencilReadMask     = (unsigned char)values[2];
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilFuncSeparate(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 4) {
            switch (values[0]) {
                case GL_FRONT:
                    depthStencilStateDesc->frontFace.stencilFunc =
                        mapCompareMode(values[1]);
                    break;

                case GL_BACK:
                    depthStencilStateDesc->backFace.stencilFunc =
                        mapCompareMode(values[1]);
                    break;

                case GL_FRONT_AND_BACK:
                default:
                    depthStencilStateDesc->frontFace.stencilFunc =
                        mapCompareMode(values[1]);
                    depthStencilStateDesc->backFace.stencilFunc =
                        mapCompareMode(values[1]);
                    break;
            }

            // Note: The OpenMaya API does not support seperate
            // reference and read mask for front and back faces.
            depthStencilStateDesc->stencilReferenceVal = values[2];
            depthStencilStateDesc->stencilReadMask     = (unsigned char)values[3];
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilOp(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 3) {
            depthStencilStateDesc->frontFace.stencilFailOp =
                mapStencilOperation(values[0]);
            depthStencilStateDesc->frontFace.stencilDepthFailOp =
                mapStencilOperation(values[1]);
            depthStencilStateDesc->frontFace.stencilPassOp =
                mapStencilOperation(values[2]);

            depthStencilStateDesc->backFace.stencilFailOp =
                mapStencilOperation(values[0]);
            depthStencilStateDesc->backFace.stencilDepthFailOp =
                mapStencilOperation(values[1]);
            depthStencilStateDesc->backFace.stencilPassOp =
                mapStencilOperation(values[2]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilOpSeparate(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 4) {
            if (values[0] == GL_FRONT || values[0] == GL_FRONT_AND_BACK) {
                depthStencilStateDesc->frontFace.stencilFailOp =
                    mapStencilOperation(values[1]);
                depthStencilStateDesc->frontFace.stencilDepthFailOp =
                    mapStencilOperation(values[2]);
                depthStencilStateDesc->frontFace.stencilPassOp =
                    mapStencilOperation(values[3]);
            }

            if (values[0] == GL_BACK || values[0] == GL_FRONT_AND_BACK) {
                depthStencilStateDesc->backFace.stencilFailOp =
                    mapStencilOperation(values[1]);
                depthStencilStateDesc->backFace.stencilDepthFailOp =
                    mapStencilOperation(values[2]);
                depthStencilStateDesc->backFace.stencilPassOp =
                    mapStencilOperation(values[3]);
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilRef(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            depthStencilStateDesc->stencilReferenceVal = values[0];
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilFail(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            depthStencilStateDesc->frontFace.stencilFailOp =
                mapStencilOperation(values[0]);
            depthStencilStateDesc->backFace.stencilFailOp =
                mapStencilOperation(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilZFail(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            depthStencilStateDesc->frontFace.stencilDepthFailOp =
                mapStencilOperation(values[0]);
            depthStencilStateDesc->backFace.stencilDepthFailOp =
                mapStencilOperation(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }

    void assignHdlr_StencilPass(
        const CGstateassignment& sa,
        MBlendStateDesc*         /* blendStateDesc */,
        MRasterizerStateDesc*    /* rasterizerStateDesc */,
        MDepthStencilStateDesc*  depthStencilStateDesc)
    {
        int numValues = 0;
        const int *values = cgGetIntStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            depthStencilStateDesc->frontFace.stencilPassOp =
                mapStencilOperation(values[0]);
            depthStencilStateDesc->backFace.stencilPassOp =
                mapStencilOperation(values[0]);
        }
        else {
            MGlobal::displayError(WrongNumValues);
        }
    }


    //******************************************************************************
    // Error handlers for unsupported states
    //******************************************************************************

    bool unsupportedIfTrue(const CGstateassignment& sa)
    {
        int numValues = 0;
        const CGbool *values = cgGetBoolStateAssignmentValues(sa, &numValues);
        if (numValues == 1) {
            const bool value = (values[0] == CG_TRUE) ? true : false;
            if (value) {

#ifdef DEBUG
                CGstate state = cgGetStateAssignmentState(sa);
                const char *stateName = cgGetStateName(state);

                // Unsupported state. We must use a
                // glPushAttrib()/glPopAttrib() pair to guarantee correct
                // rendering.
                MString s;
                s += "cgfxShader: The CgFX state assignment \"";
                s += stateName;
                s += " = true;\" contained in the shader file is not accelarated by the "
                    "cgfxShader plugin in a VP2.0 viewport and will results in lower "
                    "performance.";
                MGlobal::displayWarning(s);
#endif

                return true;
            }
            else {
                // The state assignment value is false. We assume that
                // this state corresponds to the default Cg, VP2.0
                // and OpenGL state. We can therefore safely assume
                // that this state assignment is supported.
                return false;
            }
        }
        else {
            MGlobal::displayError(WrongNumValues);
            return true;
        }
    }
}


//==============================================================================
// CLASS cgfxPassStateSetter
//==============================================================================

cgfxPassStateSetter::ViewportMode cgfxPassStateSetter::sActiveViewportMode =
    cgfxPassStateSetter::kUnknown;

bool cgfxPassStateSetter::registerCgStateCallBacks(
    cgfxPassStateSetter::ViewportMode mode
) {
    static bool initialized = false;
    if(!initialized)
    {
        ////////////////////////////////////////
        // Ignored states
        StateAssignment::registerIgnoredState("VertexProgram");
        StateAssignment::registerIgnoredState("VertexShader");
        StateAssignment::registerIgnoredState("GeometryProgram");
        StateAssignment::registerIgnoredState("GeometryShader");
        StateAssignment::registerIgnoredState("FragmentProgram");
        StateAssignment::registerIgnoredState("PixelShader");

        ////////////////////////////////////////
        // Blend states
        StateAssignment::registerHandler("BlendColor",        assignHdlr_BlendColor      );
        StateAssignment::registerHandler("BlendEnable",       assignHdlr_BlendEnable     );
        StateAssignment::registerHandler("AlphaBlendEnable",  assignHdlr_BlendEnable     );

        StateAssignment::registerHandler("SrcBlend",          assignHdlr_SrcBlend        );
        StateAssignment::registerHandler("DestBlend",         assignHdlr_DestBlend       );
        StateAssignment::registerHandler("BlendFunc",         assignHdlr_BlendFunc       );

        StateAssignment::registerHandler("BlendOp",           assignHdlr_BlendOp         );
        StateAssignment::registerHandler("BlendEquation",     assignHdlr_BlendOp         );
        StateAssignment::registerHandler("BlendFuncSeparate", assignHdlr_BlendFuncSeparate );
        StateAssignment::registerHandler("BlendEquationSeparate", assignHdlr_BlendEquationSeparate );

        StateAssignment::registerHandler("ColorWriteEnable",  assignHdlr_ColorWriteEnable);

        // FXIME: I am not sure how to handle the
        // MBlendStateDesc::alphaToCoverageEnable and
        // MBlendStateDesc::multiSampleMask fields of the
        // MBlendStateDesc. It seems to me that Maya should initialize
        // them to the proper values depending on whether MSAA is
        // globaly enabled. They also seems DirectX specific.

        // Note: The MBlendStateDesc::independentBlendEnable field is
        // always set to false since the CgFX plugin only renders to a
        // single framebuffer.


        ////////////////////////////////////////
        // Rasterizer states
        //
        StateAssignment::registerHandler("FillMode",           	    assignHdlr_FillMode                );
        StateAssignment::registerHandler("CullMode",           	    assignHdlr_CullFace                );
        StateAssignment::registerHandler("CullFace",           	    assignHdlr_CullFace                );
        StateAssignment::registerHandler("CullFaceEnable",     	    assignHdlr_CullFaceEnable          );
        StateAssignment::registerHandler("FrontFace",     	        assignHdlr_FrontFace               );
        StateAssignment::registerHandler("PolygonMode",     	    assignHdlr_PolygonMode             );

        StateAssignment::registerHandler("PolygonOffset",     	    assignHdlr_PolygonOffset           );
        StateAssignment::registerHandler("DepthBias",               assignHdlr_DepthBias               );
        StateAssignment::registerHandler("SlopScaleDepthBias",      assignHdlr_SlopScaleDepthBias      );
        StateAssignment::registerHandler("PolygonOffsetFillEnable", assignHdlr_PolygonOffsetFillEnable );

        StateAssignment::registerHandler("ScissorTestEnable",       assignHdlr_ScissorTestEnable       );
        StateAssignment::registerHandler("MultisampleEnable",       assignHdlr_MultisampleEnable       );
        StateAssignment::registerHandler("MultiSampleAntialias",    assignHdlr_MultisampleEnable       );

        // The OpenMaya API combines the Line and Point smoothing into
        // a single control, mainly because there are no such separate
        // controls on DX.
        StateAssignment::registerHandler("LineSmoothEnable",        assignHdlr_LineSmoothEnable       );
        StateAssignment::registerHandler("PointSmoothEnable",       assignHdlr_LineSmoothEnable       );


        ////////////////////////////////////////
        // Depth and stencil states
        StateAssignment::registerHandler("DepthTestEnable",    	    assignHdlr_DepthTestEnable         );
        StateAssignment::registerHandler("ZEnable",    	            assignHdlr_DepthTestEnable         );
        StateAssignment::registerHandler("DepthMask",          	    assignHdlr_DepthMask               );
        StateAssignment::registerHandler("ZWriteEnable",          	assignHdlr_DepthMask               );
        StateAssignment::registerHandler("DepthFunc",          	    assignHdlr_DepthFunc               );

        StateAssignment::registerHandler("StencilEnable",      	    assignHdlr_StencilEnable           );
        StateAssignment::registerHandler("StencilTestEnable",      	assignHdlr_StencilEnable           );
        StateAssignment::registerHandler("StencilMask",        	    assignHdlr_StencilWriteMask        );
        StateAssignment::registerHandler("StencilWriteMask",   	    assignHdlr_StencilWriteMask        );
        StateAssignment::registerHandler("StencilFunc",             assignHdlr_StencilFunc             );
        StateAssignment::registerHandler("StencilFuncSeparate",     assignHdlr_StencilFuncSeparate     );
        StateAssignment::registerHandler("StencilOp",               assignHdlr_StencilOp               );
        StateAssignment::registerHandler("StencilOpSeparate",       assignHdlr_StencilOpSeparate       );

        StateAssignment::registerHandler("StencilRef",              assignHdlr_StencilRef              );
        StateAssignment::registerHandler("StencilFail",             assignHdlr_StencilFail             );
        StateAssignment::registerHandler("StencilZFail",            assignHdlr_StencilZFail            );
        StateAssignment::registerHandler("StencilPass",             assignHdlr_StencilPass             );

        ////////////////////////////////////////
        // Unsupported states
        StateAssignment::registerUnsupportedCheck("AutoNormalEnable",               unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("AlphaTestEnable",                unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("ClipPlaneEnable",                unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("ColorLogicOpEnable",             unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("ColorVertex",                    unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("DepthBoundsEnable",              unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("DepthClampEnable",               unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("DitherEnable",                   unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("FogEnable",                      unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LightEnable",                    unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LightModelLocalViewerEnable",    unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LightModelTwoSideEnable",        unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LightingEnable",                 unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LineStippleEnable",              unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LocalViewer",                    unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("LogicOpEnable",                  unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("NormalizeEnable",                unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PointScaleEnable",               unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PointSmoothEnable",              unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PointSpriteCoordReplace",        unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PointSpriteEnable",              unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PolygonOffsetLineEnable",        unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PolygonOffsetPointEnable",       unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("PolygonStippleEnable",           unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("RescaleNormalEnable",            unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("SampleAlphaToCoverageEnable",    unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("SampleAlphaToOneEnable",         unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("SampleCoverageEnable",           unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("ScissorTestEnable",              unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("StencilTestTwoSideEnable",       unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("TexGenQEnable",                  unsupportedIfTrue);
        StateAssignment::registerUnsupportedCheck("TexGenREnable",                  unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("TexGenSEnable",                  unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("TexGenTEnable",                  unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("Texture1DEnable",                unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("Texture2DEnable",                unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("Texture3DEnable",                unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("TextureCubeMapEnable",           unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("TextureRectangleEnable",         unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("VertexProgramPointSizeEnable",   unsupportedIfTrue );
        StateAssignment::registerUnsupportedCheck("VertexProgramTwoSideEnable",     unsupportedIfTrue );

        initialized = true;
    }

    if(mode == cgfxPassStateSetter::kUnknown)
        return false;

    if(mode == cgfxPassStateSetter::sActiveViewportMode)
        return true;

    if(mode == cgfxPassStateSetter::kDefaultViewport)
    {
        StateAssignment::registerDefaultCallBacks();
        sActiveViewportMode = cgfxPassStateSetter::kDefaultViewport;
    }
    else
    {
        StateAssignment::registerVP20CallBacks();
        sActiveViewportMode = cgfxPassStateSetter::kVP20Viewport;
    }

    return true;
}

cgfxPassStateSetter::cgfxPassStateSetter()
    : fBlendState(NULL),
      fRasterizerState(NULL),
      fDepthStencilState(NULL),
      fIsPushPopAttribsRequired(false)
{}

void cgfxPassStateSetter::init(
    MStateManager* stateMgr,
    CGpass pass
)
{
    // Start with the default state!
    MBlendStateDesc blendStateDesc;
    MRasterizerStateDesc rasterizerStateDesc;
    MDepthStencilStateDesc depthStencilStateDesc;

    CGstateassignment sa = cgGetFirstStateAssignment(pass);
    while (sa) {
        fIsPushPopAttribsRequired |=
            StateAssignment::callHandler(sa,
                                         &blendStateDesc,
                                         &rasterizerStateDesc,
                                         &depthStencilStateDesc);
        sa = cgGetNextStateAssignment(sa);
    }

    fBlendState = stateMgr->acquireBlendState(blendStateDesc);
    fDepthStencilState = stateMgr->acquireDepthStencilState(depthStencilStateDesc);
    fRasterizerState = stateMgr->acquireRasterizerState(rasterizerStateDesc);
}

cgfxPassStateSetter::~cgfxPassStateSetter()
{
    MStateManager::releaseBlendState(fBlendState);
    fBlendState = NULL;

    MStateManager::releaseDepthStencilState(fDepthStencilState);
    fDepthStencilState = NULL;

    MStateManager::releaseRasterizerState(fRasterizerState);
    fRasterizerState = NULL;
}

void cgfxPassStateSetter::setPassState(MStateManager* stateMgr) const
{
    stateMgr->setBlendState(fBlendState);
    stateMgr->setDepthStencilState(fDepthStencilState);
    stateMgr->setRasterizerState(fRasterizerState);
}
