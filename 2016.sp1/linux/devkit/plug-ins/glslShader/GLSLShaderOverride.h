//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+

#ifndef GLSLSHADER_GLSLSHADEROVERRIDE_H
#define GLSLSHADER_GLSLSHADEROVERRIDE_H

#include <maya/MHWGeometry.h>
#include <maya/MPxShaderOverride.h>
#include <maya/MShaderManager.h>
#include <maya/MUniformParameter.h>
#include <maya/MUniformParameterList.h>
#include <maya/MVaryingParameterList.h>

#include "GLSLShader.h"


class GLSLShaderOverride : public MHWRender::MPxShaderOverride
{
public:
	GLSLShaderOverride(const MObject& obj);
	static MHWRender::MPxShaderOverride* Creator(const MObject& obj);

	virtual ~GLSLShaderOverride();

	virtual MString initialize( const MInitContext& initContext,MInitFeedback& initFeedback);
	
	virtual void updateDG(MObject object);
	virtual void updateDevice();
	virtual void endUpdate();

	virtual void activateKey(MHWRender::MDrawContext& context, const MString& key);
	virtual bool handlesDraw(MHWRender::MDrawContext& context);
	virtual bool draw(MHWRender::MDrawContext& context,const MHWRender::MRenderItemList& renderItemList) const;
	virtual void terminateKey(MHWRender::MDrawContext& context, const MString& key);

	virtual MHWRender::DrawAPI supportedDrawAPIs() const;
	virtual bool isTransparent();
	virtual bool supportsAdvancedTransparency() const;
	virtual bool overridesDrawState();
	virtual double boundingBoxExtraScale() const;
	virtual bool overridesNonMaterialItems() const;

	virtual MHWRender::MShaderInstance* shaderInstance() const;

	virtual bool rebuildAlways();
	
private:
	double fBBoxExtraScale;
	bool fShaderBound;

	//GLSLShader associated with the shader override
	GLSLShaderNode* fShaderNode;
	
};

#endif
