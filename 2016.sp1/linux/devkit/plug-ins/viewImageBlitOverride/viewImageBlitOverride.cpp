//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+


#include <maya/M3dView.h>
#include "viewImageBlitOverride.h"
#include <maya/MDrawContext.h>
#include <maya/MFrameContext.h>
#include <maya/MImage.h>
#include <maya/MStateManager.h>
#include <maya/MGlobal.h>
#include <stdlib.h>

//
// Sample plugin which will blit an image as the scene and rely on
// Maya's internal rendering for the UI only
// 
// Classes:
//
//	RenderOverride: The main override class. Contains all the operations
//					as well as keep track of texture resources.
//	SceneBlit:		A simple quad render responsible for blitting a
//					colour and depth image. Will also clear the background depth
//	UIDraw:			A scene override which filters out all but UI drawing
//
//  A stock "present" operation is also queued so that the results go to the viewport
//
namespace viewImageBlitOverride
{
	// Global override instance
	/*static*/ RenderOverride* RenderOverride::sViewImageBlitOverrideInstance = NULL;

	///////////////////////////////////////////////////////////////
	// Override
	//
	RenderOverride::RenderOverride( const MString & name )
	: MRenderOverride( name )
	, mUIName("Sample Image Blit Override")
	, mCurrentOperation(-1)
	, mLoadImagesFromDisk(false)
	{
		mOperations[0] = mOperations[1] = mOperations[2] = NULL;
		mOperationNames[0] = "viewImageBlitOverride_SceneBlit";
		mOperationNames[1] = "viewImageBlitOverride_UIDraw";
		mOperationNames[2] = "viewImageBlitOverride_Present";

		mColorTexture.texture = NULL;
		mColorTextureDesc.setToDefault2DTexture();
		mDepthTexture.texture = NULL;
		mDepthTextureDesc.setToDefault2DTexture();
	}
	RenderOverride::~RenderOverride()
	{
		// Release textures		
		MHWRender::MRenderer *renderer = MHWRender::MRenderer::theRenderer();
		MHWRender::MTextureManager* textureManager = renderer ? renderer->getTextureManager() : NULL;
		if (textureManager)
		{
			if (mColorTexture.texture)
				textureManager->releaseTexture(mColorTexture.texture);
			if (mDepthTexture.texture)
				textureManager->releaseTexture(mDepthTexture.texture);
		}

		// Release operations
		for (unsigned int i=0; i<3; i++)
		{
			if (mOperations[i])
			{
				delete mOperations[i];
				mOperations[i] = NULL;
			}
		}
	}
	
	MHWRender::DrawAPI RenderOverride::supportedDrawAPIs() const
	{
		return ( MHWRender::kOpenGL | MHWRender::kDirectX11 );
	}

	// Basic iterator methods
	bool RenderOverride::startOperationIterator()
	{
		mCurrentOperation = 0;
		return true;
	}
	MHWRender::MRenderOperation *RenderOverride::renderOperation()
	{
		if (mCurrentOperation >= 0 && mCurrentOperation < 3)
		{
			if (mOperations[mCurrentOperation])
			{
				return mOperations[mCurrentOperation];
			}
		}
		return NULL;
	}
	bool RenderOverride::nextRenderOperation()
	{
		mCurrentOperation++;
		if (mCurrentOperation < 3)
		{
			return true;
		}
		return false;
	}

	//
	// Update textures used for scene blit. Will update both a 
	// colour and a deph texture based on the current output target size.
	// 
	bool RenderOverride::updateTextures(MHWRender::MRenderer *theRenderer,
										MHWRender::MTextureManager* textureManager)
	{
		if (!theRenderer || !textureManager)
			return false;

		// Get current output size.
		unsigned int targetWidth = 0;
		unsigned int targetHeight = 0;
		theRenderer->outputTargetSize( targetWidth, targetHeight );

		bool acquireNewTexture = false;
		
		// Decide whether to load images from disk
		// If loading from disk, will use a single iff file which contains both
		// color and depth. Seperate files for color and depth could be chosen.
		//
		// For this example, we just use an environment variable to switch
		// to show both execution paths.
		//
		int val = 0;
		bool forceReload = false;
		if (MGlobal::getOptionVarValue("VIBO_LoadImagesFromDisk", val))
		{
			bool loadFromDisk = (val > 0);
			if (loadFromDisk != mLoadImagesFromDisk)
			{
				mLoadImagesFromDisk = loadFromDisk;
				forceReload = true;
			}
		}

		MString imageLocation(MString(getenv("MAYA_LOCATION")) + MString("\\devkit\\plug-ins\\viewImageBlitOverride\\"));
		MString colorImageFileName( imageLocation + MString("renderedImage.iff") );
		MString depthImageFileName( colorImageFileName );

		// If a resize occured, or we haven't allocated any texture yet,
		// then create new textures which match the output size. 
		// Release any existing textures.
		//
		if (forceReload ||
			!mColorTexture.texture ||
			!mDepthTexture.texture ||
			mColorTextureDesc.fWidth != targetWidth ||
			mColorTextureDesc.fHeight != targetHeight)
		{
			if (mColorTexture.texture)
			{
				textureManager->releaseTexture( mColorTexture.texture );
				mColorTexture.texture = NULL;
			}
			if (mDepthTexture.texture)
			{
				textureManager->releaseTexture( mDepthTexture.texture );
				mDepthTexture.texture = NULL;
			}
			acquireNewTexture = true;
		}
		if (!mColorTexture.texture)
		{
			unsigned char *textureData = NULL;

			MImage image;
			if (mLoadImagesFromDisk)
			{
				image.readFromFile( colorImageFileName );
				image.getSize( targetWidth, targetHeight );
				textureData = image.pixels();		
			}
			else
			{
				// Prepare some data which forms a checker pattern
				//
				textureData = new unsigned char[4*targetWidth*targetHeight];
				if (textureData)
				{
					for (unsigned int y = 0; y < targetHeight; y++)
					{
						unsigned char* pPixel = textureData + (y * targetWidth * 4);
						for (unsigned int x = 0; x < targetWidth; x++)
						{
							bool checker = (((x >> 5) & 1) ^ ((y >> 5) & 1)) != 0;
							*pPixel++ = checker ? 255 : 0;
							*pPixel++ = 0;
							*pPixel++ = 0;
							*pPixel++ = 255;
						}
					}
				}
			}

			mColorTextureDesc.fWidth = targetWidth;
			mColorTextureDesc.fHeight = targetHeight;
			mColorTextureDesc.fDepth = 1;
			mColorTextureDesc.fBytesPerRow = 4*targetWidth;
			mColorTextureDesc.fBytesPerSlice = mColorTextureDesc.fBytesPerRow*targetHeight;

			// Acquire a new texture.
			mColorTexture.texture = textureManager->acquireTexture( "", mColorTextureDesc, textureData );
			if (mColorTexture.texture)
				mColorTexture.texture->textureDescription( mColorTextureDesc );

			if (!mLoadImagesFromDisk)
			{
				// Don't need the data any more after upload to texture
				delete [] textureData;
			}
		}

		// Various methods for updating the colour texture with some sample data
		// for a checker pattern.
		// 
		// 1. The raw data can be read back from an existing texture, modified and sent
		//	  back to update the texture. This involves a GPU->CPU read and a CPU->GPU
		//	  transfer. Toggled using the 'updateWithRawData' variable.
		// 2. Raw data is allocated by the plug-in and a CPU-GPU transfer is used.
		//	  This is the same update interface as used in method 1.
		// 3. Data is allocated in an MImage and data is tranferred to the texture
		//	  in a CPU->GPU transfer. Toggle using the 'updateWithMImage' variable.
		//
		// All three are shown, with suitability to circumstances 
		// left to the plug-in writer to choose. The code as is follow option 2.
		//
		else
		{
			// Test code to update the image on every refresh
			//
			// The update code assumes the image size is the viewport size, so
			// don't use if loading from disk as the disk image may not be the viewport size.
			bool updateOnEveryRefresh = !mLoadImagesFromDisk;
			bool generateMipMaps = false;
			if (updateOnEveryRefresh)
			{
				// Test to read back data from the texture and update on
				// the copy. Does a colour invert. 
				bool updateWithRawData = false;
				if (updateWithRawData)
				{
					mColorTexture.texture->textureDescription( mColorTextureDesc );
					int rowPitch = 0;
					int slicePitch = 0;
					unsigned char* textureData = (unsigned char *)mColorTexture.texture->rawData(rowPitch, slicePitch);
					unsigned char* val = NULL;
					if (textureData && rowPitch > 0 && slicePitch > 0)
					{
						for (unsigned int i=0; i<mColorTextureDesc.fHeight; i++)
						{
							val = textureData + (i*rowPitch);
							for (unsigned int j=0; j<mColorTextureDesc.fWidth*4; j++)
							{
								*val = 255 - *val;
								val++;
							}
						}
						// Call the update method with raw data
						mColorTexture.texture->update(textureData, generateMipMaps, rowPitch);
					}
					delete [] textureData;
				}

				// Test to send over new data to update the texture
				else
				{
					MImage image;
					unsigned char* textureData = NULL;

					bool updateWithMImage = true;
					unsigned int tile = 5;
					// MImage allocation
					if (updateWithMImage)
					{
						image.create(targetWidth, targetHeight, 4, MImage::kByte);
						textureData = image.pixels();
					}
					// Raw data allocation
					else
					{
						textureData = new unsigned char[4*targetWidth*targetHeight];
					}

					// Some sample code to rotate colouring the checker differently
					// per update.
					if (textureData)
					{
						static unsigned int counter = 0;
						for (unsigned int y = 0; y < targetHeight; y++)
						{
							unsigned char* pPixel = textureData + (y * targetWidth * 4);
							for (unsigned int x = 0; x < targetWidth; x++)
							{
								bool checker = (((x >> tile) & 1) ^ ((y >> tile) & 1)) != 0;
								*pPixel++ = (counter == 0 && checker) ? 255 : 0;
								*pPixel++ = (counter == 1 && checker) ? 255 : 0;
								*pPixel++ = (counter == 2 && checker) ? 255 : 0;
								*pPixel++ = 255;
							}
						}
						counter = (counter + 1) % 3;
					}

					// Call the update method with raw data
					if (!updateWithMImage)
					{
						mColorTexture.texture->update( textureData, generateMipMaps );
						delete [] textureData;
					}
					// Call the update method with an MImage
					else
					{
						mColorTexture.texture->update( image, generateMipMaps );
					}
				}
			}
		}

		// Acquire a new "depth" texture as necessary
		//
		if (!mDepthTexture.texture)
		{
			MImage image;
			image.create(targetWidth, targetHeight, 4, MImage::kByte);

			mDepthTextureDesc.fWidth = targetWidth;
			mDepthTextureDesc.fHeight = targetHeight;
			mDepthTextureDesc.fDepth = 1;
			mDepthTextureDesc.fBytesPerRow = targetWidth;
			mDepthTextureDesc.fBytesPerSlice = mDepthTextureDesc.fBytesPerRow*targetHeight;

			// Load depth image from disk to create depth texture. Uses the MImage texture creation interface.
			//
			if (mLoadImagesFromDisk)
			{
				image.readDepthMap( depthImageFileName );
				image.getDepthMapSize( targetWidth, targetHeight );
				MHWRender::MDepthNormalizationDescription normalizationDesc;
				mDepthTexture.texture = textureManager->acquireDepthTexture("", 
					image, false, &normalizationDesc );
			}
			// Load depth using programmatically created data
			//
			else
			{
				float *textureData = new float[targetWidth*targetHeight];
				if (textureData)
				{
					// Use 'createDepthWithMImage' to switch between using the MImage
					// and raw data interfaces.
					//
					// Use 'useCameraDistanceValues' to switch between using -1/distance-to-camera
					// values versus using normalized depth coordinates [0...1].
					// The flag is set to create normalized values by default in order to
					// match the requirements of the shader used to render the texture.
					// Refer to the the comments in SceneBlit::shader() for more details.
					//
					bool createDepthWithMImage = false;
					bool useCameraDistanceValues = false;

					// Create some dummy 'checkered' depth data. 
					//
					float depthValue = useCameraDistanceValues ? -1.0f / 100.0f : 1.0f;
					float depthValue2 = useCameraDistanceValues ? -1.0f / 500.0f : 0.98f;
					for (unsigned int y = 0; y < targetHeight; y++)
					{
						float* pPixel = textureData + (y * targetWidth);
						for (unsigned int x = 0; x < targetWidth; x++)
						{
							bool checker = (((x >> 5) & 1) ^ ((y >> 5) & 1)) != 0;
							*pPixel++ = checker ? depthValue  : depthValue2;
						}
					}

					MHWRender::MDepthNormalizationDescription normalizationDesc;
					if (createDepthWithMImage)
					{
						image.setDepthMap( textureData, targetWidth, targetHeight );
						mDepthTexture.texture = textureManager->acquireDepthTexture("", 
							image, false, useCameraDistanceValues ? &normalizationDesc : NULL );
					}
					else
					{
						mDepthTexture.texture = textureManager->acquireDepthTexture("", 
							textureData,  targetWidth, targetHeight, false,
							useCameraDistanceValues ? &normalizationDesc : NULL );
					}

					// Not required anymore so can delete it.
					delete [] textureData;
				}
			}

			if (mDepthTexture.texture)
				mDepthTexture.texture->textureDescription( mDepthTextureDesc );
		}

		// Update the textures used for the blit operation.
		//
		if (acquireNewTexture)
		{
			SceneBlit* blit = (SceneBlit *)mOperations[0];
			blit->setColorTexture( mColorTexture );
			blit->setDepthTexture( mDepthTexture );
		}

		return (mDepthTexture.texture && mColorTexture.texture);
	}

	//
	// Create the set of operations as necessary, update
	// textures for image blits, and install a callback so we can setup and 
	// cleanup appropriatelyt when the override changes.
	//
	MStatus RenderOverride::setup( const MString & destination )
	{
		MHWRender::MRenderer *theRenderer = MHWRender::MRenderer::theRenderer();
		if (!theRenderer)
			return MStatus::kFailure;
		MHWRender::MTextureManager* textureManager = theRenderer->getTextureManager();
		if (!textureManager)
			return MStatus::kFailure;

		// Create a new set of operations as required
		if (!mOperations[0])
		{
			mOperations[0] = (MHWRender::MRenderOperation *) new SceneBlit( mOperationNames[0] );
			mOperations[1] = (MHWRender::MRenderOperation *) new UIDraw( mOperationNames[1] );
			mOperations[2] = (MHWRender::MRenderOperation *) new MHWRender::MPresentTarget(  mOperationNames[2] );
		}
		if (!mOperations[0] || 
			!mOperations[1] ||
			!mOperations[2])
		{
			return MStatus::kFailure;
		}

		// Update textures used for scene blit
		if (!updateTextures(theRenderer, textureManager))
			return MStatus::kFailure;

		//
		// Force the panel display style to smooth shaded if it's not already
		// this ensures that viewport selection behaviour works as if shaded
		//
		M3dView view;		
		if (destination.length() && 
			M3dView::getM3dViewFromModelPanel(destination, view) == MStatus::kSuccess)
		{
			if (view.displayStyle() != M3dView::kGouraudShaded)
			{
				view.setDisplayStyle(M3dView::kGouraudShaded);
			}
		}

		return MStatus::kSuccess;
	}

	MStatus RenderOverride::cleanup()
	{
		mCurrentOperation = -1;
		return MStatus::kSuccess;
	}

	///////////////////////////////////////////////////////////////
	//
	// Image blit used to perform the 'scene render'
	//
	SceneBlit::SceneBlit(const MString &name)
	: MQuadRender( name )
	, mShaderInstance(NULL)
	, mColorTextureChanged(false)
	, mDepthTextureChanged(false)
	, mDepthStencilState(NULL)
	{
		mColorTexture.texture = NULL;
		mDepthTexture.texture = NULL;
	}

	SceneBlit::~SceneBlit()
	{
		MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
		if (!renderer)
			return;

		// Release any shader used
		if (mShaderInstance)
		{
			const MHWRender::MShaderManager* shaderMgr = renderer->getShaderManager();
			if (shaderMgr)
			{
				shaderMgr->releaseShader(mShaderInstance);
			}
			mShaderInstance = NULL;
		}

		// Release any state
		if (mDepthStencilState)
		{
			MHWRender::MStateManager::releaseDepthStencilState( mDepthStencilState );
			mDepthStencilState = NULL;
		}
	}

	const MHWRender::MShaderInstance * SceneBlit::shader()
	{
		if (!mShaderInstance)
		{
			MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
			const MHWRender::MShaderManager* shaderMgr = renderer ? renderer->getShaderManager() : NULL;
			if (shaderMgr)
			{
				// Create the shader.
				//
				// The default shader technique will blit color and depth textures to the
				// the output color and depth buffers respectively. The values in the depth
				// texture are expected to be normalized. 
				// 
				// The flag 'showDepthAsColor' can be set to switch to the "DepthToColor"
				// technique which will route the depth texture to the color buffer.
				// This can be used for visualizing or debugging the contents of the depth texture.
				//
				bool showDepthAsColor = false;
				mShaderInstance = shaderMgr->getEffectsFileShader( "mayaBlitColorDepth", showDepthAsColor ? "DepthToColor" : "" );
			}
		}

		MStatus status = MStatus::kFailure;
		if (mShaderInstance)
		{
			// If texture changed then bind new texture to the shader
			// 
			status = MStatus::kSuccess;
			if (mColorTextureChanged)
			{
				status = mShaderInstance->setParameter("gColorTex", mColorTexture);
				mColorTextureChanged = false;
			}

			if (status == MStatus::kSuccess && mDepthTextureChanged)
			{
				status = mShaderInstance->setParameter("gDepthTex", mDepthTexture);
				mDepthTextureChanged = false;
			}
		}
		if (status != MStatus::kSuccess)
		{
			return NULL;
		}
		return mShaderInstance;
	}

	MHWRender::MClearOperation & SceneBlit::clearOperation()
	{
		mClearOperation.setClearGradient( false );
		mClearOperation.setMask( (unsigned int) MHWRender::MClearOperation::kClearAll );
		return mClearOperation;
	}

	//
	// Want to have this state override set to override the default
	// depth stencil state which disables depth writes.
	//
	const MHWRender::MDepthStencilState *SceneBlit::depthStencilStateOverride()
	{
		if (!mDepthStencilState)
		{
			MHWRender::MDepthStencilStateDesc desc;
			desc.depthEnable = true;
			desc.depthWriteEnable = true;
			desc.depthFunc = MHWRender::MStateManager::kCompareAlways;
			mDepthStencilState = MHWRender::MStateManager::acquireDepthStencilState(desc);	
		}
		return mDepthStencilState;
	}

	///////////////////////////////////////////////////////////////
	// Maya UI draw operation. Draw all UI except for a few exclusion
	// 
	UIDraw::UIDraw(const MString& name)
	: MHWRender::MSceneRender(name)
	{
	}

	UIDraw::~UIDraw()
	{
	}

	MHWRender::MSceneRender::MSceneFilterOption		
	UIDraw::renderFilterOverride()
	{
		return MHWRender::MSceneRender::kRenderNonShadedItems;
	}

	MHWRender::MSceneRender::MObjectTypeExclusions	
	UIDraw::objectTypeExclusions()
	{
		// Exclude drawing the grid and image planes
		return (MHWRender::MSceneRender::MObjectTypeExclusions)(MHWRender::MSceneRender::kExcludeGrid | MHWRender::MSceneRender::kExcludeImagePlane);
	}

	MHWRender::MClearOperation &					
	UIDraw::clearOperation()
	{
		// Disable clear since we don't want to clobber the scene colour blit.
		mClearOperation.setMask((unsigned int)MHWRender::MClearOperation::kClearNone);
		return mClearOperation;
	}

} //namespace

