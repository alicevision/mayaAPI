#ifndef __hwRendererHelper_h__
#define __hwRendererHelper_h__

namespace MHWRender
{
	class MRenderer;
	class MTexture;
	class MRenderTarget;
	//class MTextureDescription;
}

class hwRendererHelper
{
public:
	static hwRendererHelper* create(MHWRender::MRenderer* renderer);

protected:
	hwRendererHelper(MHWRender::MRenderer* renderer);
	
public:
	virtual ~hwRendererHelper();
	
	MHWRender::MTexture* createTextureFromScreen();
	bool renderTextureToScreen(MHWRender::MTexture* texture);

protected:
	virtual bool renderTextureToTarget(MHWRender::MTexture* texture, MHWRender::MRenderTarget *target) = 0;
	
protected:
	MHWRender::MRenderer* fRenderer;
};

#endif 

//-
// Copyright 2012 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
