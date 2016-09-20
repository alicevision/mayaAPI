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

#ifndef _cgfxPassStateSetter_h_
#define _cgfxPassStateSetter_h_

#include <maya/MStateManager.h>

#include "cgfxShaderCommon.h"

namespace MHWRender {
  class MDrawContext;
}

class cgfxPassStateSetter
{
public:

	enum ViewportMode
	{
		kDefaultViewport,
		kVP20Viewport,
		kUnknown
	};

	static bool registerCgStateCallBacks(ViewportMode mode);

    // Create a state setter for the pass. The init member function
    // must be called after the default constructor. The default
    // constructor is provided so that cgfxPassStateSetter can be
    // stored into arrays.
    cgfxPassStateSetter();
    void init(MHWRender::MStateManager* stateMgr, CGpass pass);
    ~cgfxPassStateSetter();

    // Return whether a glPushAttrib()/glPopAttrib() pair is required
    // because of an unhandled CgFX state in the pass.
    bool isPushPopAttribsRequired() const { return fIsPushPopAttribsRequired; }

    void setPassState(MHWRender::MStateManager* stateMgr) const;
    
private:
	static ViewportMode sActiveViewportMode;

    // The state associated with the pass.
	const MHWRender::MBlendState* fBlendState;
	const MHWRender::MRasterizerState* fRasterizerState;
   	const MHWRender::MDepthStencilState* fDepthStencilState;	

    bool fIsPushPopAttribsRequired;
    
    // Prohibited and not implemented.
    cgfxPassStateSetter(const cgfxPassStateSetter&);
    const cgfxPassStateSetter& operator=(const cgfxPassStateSetter&);
};

#endif
