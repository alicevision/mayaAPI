#include <maya/MFnPlugin.h>
#include <maya/MArgList.h>
#include <maya/MArgParser.h>
#include <maya/MSyntax.h>
#include <maya/MFileObject.h>
#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MTextureManager.h>
#include <maya/MRenderTargetManager.h>

#include "hwApiTextureTestStrings.h"
#include "hwRendererHelper.h"

#ifndef _WIN32 
	#include <unistd.h>
#endif

namespace
{
	// load argument
	const char *loadArgName = "-l";
	const char *loadArgLongName = "-load";

	// draw argument
	const char *drawArgName = "-d";
	const char *drawArgLongName = "-draw";

	// edit argument
	const char *editArgName = "-e";
	const char *editArgLongName = "-edit";	

	// tile argument
	const char *tileArgName = "-t";
	const char *tileArgLongName = "-tile";

	// save argument
	const char *saveArgName = "-s";
	const char *saveArgLongName = "-save";

	// format argument
	const char *formatArgName = "-f";
	const char *formatArgLongName = "-format";

	// list argument
	const char *listArgName = "-ls";
	const char *listArgLongName = "-list";


	MString fileExtension(const MString& filePath)
	{
		MString extension;

		MStringArray elements;
		if( filePath.split('.', elements) == MStatus::kSuccess )
		{
			if( elements.length() > 1 )
			{
				extension = elements[elements.length() - 1];
			}
		}

		return extension;
	}

	MString changeExtension(const MString& filePath, const MString& extension)
	{
		MString newPath = filePath;

		int index = filePath.rindexW('.');
		if(index > 0)
		{
			newPath = filePath.substring(0, index) + extension;
		}

		return newPath;
	}
};

#ifdef _WIN32
#define mySleep(s)	Sleep(s * 1000)
#else
#define	mySleep(s)	sleep(s)
#endif

///////////////////////////////////////////////////
//
// Command class declaration
//
// Example MEL usage:
//
// -- To launch the load textures test.
// hwApiTextureTest -load <path> [-draw] [-edit];
//
// <path> The path where to look for textures.
// -draw Optional flag to display the loaded textures in active viewport.
// -edit Optional flag to perform a modification on the texture.
//
// -- To launch save texture test
// hwApiTextureTest -save <path> -format <format1> [-format <format2];
//
// <path> The path where to save the texture(s) to.
// -format <format1> Format of the texture to save. At least one needed, multiple formats possible
// If selected format is "all" will save the texture will all the available formats
//
// -- To get the list of supported formats
// hwApiTextureTest -list
//
// -- Load and save mix together
// hwApiTextureTest -load <path1> -save <path2>
// Will load all the texture files from path1 and save them to path2
//
///////////////////////////////////////////////////

class hwApiTextureTestCmd : public MPxCommand
{
public:
					hwApiTextureTestCmd();
	virtual			~hwApiTextureTestCmd();

	static void*	creator();
	static MSyntax	newSyntax();

	virtual MStatus	doIt( const MArgList& args );

private:
	MStatus			parseArgs( const MArgList& args );

	MStatus			loadTextures();
	MStatus			saveAsTexture();
	MStatus			editTexture();
	MStatus			listFormats();

private:
	bool			fLoadTextures;
	MFileObject		fLoadTexturesPath;
	bool			fDrawTextures;
	bool			fEditTextures;
	bool			fTileTextures;

	bool			fSaveAsTexture;
	MFileObject		fSaveAsTexturePath;
	MStringArray	fSaveAsTextureFormats;
	bool			fSaveAllFormats;

	bool			fListFormats;

	MStringArray	fSupportedFormats;
};

///////////////////////////////////////////////////
//
// Command class implementation
//
///////////////////////////////////////////////////

// Constructor
//
hwApiTextureTestCmd::hwApiTextureTestCmd()
: fLoadTextures(false)
, fLoadTexturesPath()
, fDrawTextures(false)
, fEditTextures(false)
, fTileTextures(false)
, fSaveAsTexture(false)
, fSaveAsTexturePath()
, fSaveAsTextureFormats()
, fSaveAllFormats(false)
, fListFormats(false)
{
	fSupportedFormats.append( L"bmp" );
	fSupportedFormats.append( L"dds" );
	fSupportedFormats.append( L"exr" );
	fSupportedFormats.append( L"gif" );
	fSupportedFormats.append( L"iff" );
	fSupportedFormats.append( L"jpg" );
	fSupportedFormats.append( L"pct" );
	fSupportedFormats.append( L"pic" );
	fSupportedFormats.append( L"png" );
	fSupportedFormats.append( L"psd" );
	fSupportedFormats.append( L"rla" );
	fSupportedFormats.append( L"sgi" );
	fSupportedFormats.append( L"tga" );
	fSupportedFormats.append( L"tif" );
}

// Destructor
//
hwApiTextureTestCmd::~hwApiTextureTestCmd()
{
	// Do nothing
}

// creator
//
void* hwApiTextureTestCmd::creator()
{
	return (void *) (new hwApiTextureTestCmd);
}

// newSyntax
//
MSyntax hwApiTextureTestCmd::newSyntax()
{
	MSyntax syntax;

	syntax.addFlag(loadArgName, loadArgLongName, MSyntax::kString /*path*/);
	syntax.addFlag(drawArgName, drawArgLongName);
	syntax.addFlag(editArgName, editArgLongName);
	syntax.addFlag(tileArgName, tileArgLongName);

	syntax.addFlag(saveArgName, saveArgLongName, MSyntax::kString /*path*/);
	syntax.addFlag(formatArgName, formatArgLongName, MSyntax::kString /*format*/);
	syntax.makeFlagMultiUse(formatArgName);

	syntax.addFlag(listArgName, listArgLongName);

	return syntax;
}

// parseArgs
//
MStatus hwApiTextureTestCmd::parseArgs( const MArgList& args )
{
	MStatus status = MStatus::kSuccess;

	MSyntax syntax = newSyntax();
	MArgParser argParser(syntax, args, &status);

	if( status == MStatus::kSuccess )
	{
		// load flag
		if( argParser.isFlagSet(loadArgName) )
		{
			fLoadTextures = true;

			// get path
			MString path;
			if( argParser.getFlagArgument(loadArgName, 0, path) == MStatus::kSuccess )
				fLoadTexturesPath.setRawPath(path);
		}

		// draw flag
		if( argParser.isFlagSet(drawArgName) )
		{
			fDrawTextures = true;
		}

		// edit flag
		if( argParser.isFlagSet(editArgName) )
		{
			fEditTextures = true;
		}			

		// tile flag
		if( argParser.isFlagSet(tileArgName) )
		{
			fTileTextures = true;
			// Tiling and editing are no compatible.
			fEditTextures = false;
		}			

		// save flag
		if( argParser.isFlagSet(saveArgName) )
		{
			fSaveAsTexture = true;

			// get path
			MString path;
			if( argParser.getFlagArgument(saveArgName, 0, path) == MStatus::kSuccess )
				fSaveAsTexturePath.setRawPath(path);

			// get formats
			unsigned int numFormats = argParser.numberOfFlagUses(formatArgName);
			for(unsigned int flag = 0; flag < numFormats; ++flag)
			{
				MArgList formats;
				if( argParser.getFlagArgumentList(formatArgName, flag, formats) == MStatus::kSuccess )
				{
					for(unsigned int arg = 0; arg < formats.length(); ++arg)
					{
						MString format;
						if( formats.get(arg, format) == MStatus::kSuccess )
						{
							if(format == "all")
								fSaveAllFormats = true;
							else
								fSaveAsTextureFormats.append(format);
						}
					}
				}
			}
		}

		// list flag
		if( argParser.isFlagSet(listArgName) )
		{
			fListFormats = true;
		}

	}

	return status;
}

// doIt
//
MStatus hwApiTextureTestCmd::doIt(const MArgList& args)
{
	MStatus status = parseArgs(args);
	if(status)
	{
		if(fListFormats)
		{
			status = listFormats();
		}
		else if(fLoadTextures)
		{
			status = loadTextures();
		}
		else if(fSaveAsTexture)
		{
			status = saveAsTexture();
		}
	}

	return status;
}

MStatus hwApiTextureTestCmd::loadTextures()
{
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if(renderer == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorRenderer ) );
		return MStatus::kFailure;
	}

	MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
	if(textureManager == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorTextureManager ) );
		return MStatus::kFailure;
	}

	if(fLoadTexturesPath.isSet() == false || fLoadTexturesPath.exists() == false)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorLoadPathArg ) );
		return MStatus::kFailure;
	}

	MString saveAsFormat;
	if(fSaveAsTexture)
	{
		if(fSaveAsTexturePath.isSet() == false || fSaveAsTexturePath.exists() == false)
		{
			MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSavePathArg ) );
			return MStatus::kFailure;
		}

		if(fSaveAsTextureFormats.length() > 0)
			saveAsFormat = fSaveAsTextureFormats[0];
	}

	MStringArray texturesFiles;
	{
		MString fileListCmd;
		fileListCmd.format("getFileList -folder \"^1s\";", fLoadTexturesPath.resolvedPath());

		MStringArray allFiles;
		MGlobal::executeCommand(fileListCmd, allFiles);

		// Filter files
		for(unsigned int fileIdx = 0; fileIdx < allFiles.length(); ++fileIdx)
		{
			const MString& fileName = allFiles[fileIdx];

			MString extension = fileExtension(fileName);
			extension.toLowerCase();

			for(unsigned int formatIdx = 0; formatIdx < fSupportedFormats.length(); ++formatIdx)
			{
				const MString& format = fSupportedFormats[formatIdx];

				if(format == extension)
				{
					texturesFiles.append(fileName);
				}
			}
		}
	}

	unsigned int textureFilesCount = texturesFiles.length(); 
	if(textureFilesCount == 0)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorLoadNoTexture ) );
		return MStatus::kFailure;
	}

	hwRendererHelper* renderHelper = NULL;
	if(fDrawTextures)
	{
		renderHelper = hwRendererHelper::create(renderer);
	}

	MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kBeginLoadTest ) );

	if (fTileTextures && textureFilesCount > 1)
	{
		MStringArray tilePaths;
		MFloatArray tilePositions;

		// Create texture path list
		for(unsigned int textIdx = 0; textIdx < textureFilesCount; ++textIdx)
		{
			const MString& fileName = texturesFiles[textIdx];
			const MString filePath = fLoadTexturesPath.resolvedPath() + fileName;
			tilePaths.append( filePath );
		}

		// Create a tile position list corresponding to the list of tile textures.
		unsigned int tileU = (unsigned int) (sqrt( (float)textureFilesCount ));
		unsigned int tileV = (tileU*tileU != textureFilesCount) ? tileU+1 : tileU;		
		printf("file count=%d, tileU=%d, tileV=%d\n", 
			textureFilesCount, tileU, tileV);
		for (unsigned int i=0; i<tileU; i++)
		{
			for (unsigned int j=0; j<tileV; j++)
			{
				tilePositions.append( (float)i );
				tilePositions.append( (float)j );
			}
		}

		MHWRender::MTexture* texture = NULL;
		
		MString textureName("uvTiledTexture");
		MColor undefinedColor;
		unsigned int maxWidth = 4096;
		unsigned int maxHeight = 4096;

		MStringArray failedTilePaths;
		MFloatArray uvScaleOffset;

		try
		{
			texture = textureManager->acquireTiledTexture(textureName,
														tilePaths,
														tilePositions,
														undefinedColor,
														maxWidth, maxHeight, 
														failedTilePaths,
														uvScaleOffset);	
			// Simple code to test for caching. This will just return the texture
			// created in the previous acquire, assuming that succeeded.
			//
			MHWRender::MTexture *texture2 = textureManager->acquireTiledTexture(textureName,
														tilePaths,
														tilePositions,
														undefinedColor,
														maxWidth, maxHeight, 
														failedTilePaths,
														uvScaleOffset);
			// Release it here as we don't want to hold on to this reference anymore.
			if (texture2)
				textureManager->releaseTexture(texture2);
		}
		catch(...)
		{
			texture = NULL;
		}

		if(texture == NULL)
		{
			MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorLoadTexture, "Tiled texture" ) );
		}
		else
		{
			MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kSuccessLoadTexture, "Tiled texture") );
			for (unsigned int i=0; i<failedTilePaths.length(); i++)
			{
				MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorTileTexture, failedTilePaths[i].asChar() ) );
			}
			MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kTileTransform, 
				uvScaleOffset[0],uvScaleOffset[1],uvScaleOffset[2],uvScaleOffset[3]) );

			if(fSaveAsTexture)
			{
				MString savePath = fSaveAsTexturePath.resolvedPath() + MString("uvTiledTexture.dds");

				if(saveAsFormat.length() > 0)
				{
					savePath = changeExtension(savePath, saveAsFormat);
				}

				const MString extension = "dds";
				MStatus status = MStatus::kFailure;
				try
				{
					status = textureManager->saveTexture( texture, savePath );
				}
				catch(...)
				{
					status = MStatus::kFailure;
				}

				if(status)
				{
					MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kSuccessSaveTexture, savePath, extension ) );
				}
				else
				{
					MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSaveTexture, savePath, extension ) );
				}
			}

			if(fDrawTextures)
			{
				if(renderHelper->renderTextureToScreen(texture))
				{
					// Let the user see the screen ...
					mySleep(2);
				}
			}

			textureManager->releaseTexture(texture);
		}
	}
	else
	{
		for(unsigned int textIdx = 0; textIdx < textureFilesCount; ++textIdx)
		{
			const MString& fileName = texturesFiles[textIdx];

			const MString filePath = fLoadTexturesPath.resolvedPath() + fileName;

			MHWRender::MTexture* texture = NULL;

			try
			{
				texture = textureManager->acquireTexture( filePath );
			}
			catch(...)
			{
				texture = NULL;
			}

			if(texture == NULL)
			{
				MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorLoadTexture, filePath ) );
			}
			else
			{
				MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kSuccessLoadTexture, filePath ) );

				// The "edit" here is to invert the pixels for 4 bpp RGBA formats.
				//
				if (fEditTextures)
				{
					MHWRender::MTextureDescription desc;
					texture->textureDescription(desc);
					unsigned int bpp = texture->bytesPerPixel();
					if (bpp == 4 && 
						(desc.fFormat == MHWRender::kR8G8B8A8_UNORM ||
						desc.fFormat == MHWRender::kB8G8R8A8))
					{
						int rowPitch = 0;
						int slicePitch = 0;
						bool generateMipMaps = true;
						unsigned char* pixelData = (unsigned char *)texture->rawData(rowPitch, slicePitch);
						unsigned char* val = NULL;

						if (pixelData && rowPitch > 0 && slicePitch > 0)
						{
							bool updateEntireImage = false;
							if (updateEntireImage)
							{
								for (unsigned int i=0; i<desc.fHeight; i++)
								{
									val = pixelData + (i*rowPitch);
									for (unsigned int j=0; j<desc.fWidth*4; j++)
									{
										*val = 255 - *val;
										val++;
									}
								}
								texture->update(pixelData, generateMipMaps, rowPitch);
							}
							else
							{
								unsigned int minX = desc.fWidth / 3;
								unsigned int maxX = (desc.fWidth * 2) / 3;
								unsigned int newWidth = maxX - minX;
								unsigned int minY = desc.fHeight/ 3;
								unsigned int maxY = (desc.fHeight *2) / 3;
								unsigned int newHeight = maxY - minY;

								unsigned char* newData = new unsigned char[newWidth * newHeight * 4];
								if (newData)
								{
									unsigned char* newVal = newData;
									for (unsigned int i=minY; i<maxY; i++)
									{
										val = pixelData + (i*rowPitch) + minX*4;
										for (unsigned int j=0; j<newWidth*4; j++)
										{
											*newVal = 255 - *val;
											val++; 
											newVal++;
										}
									}
									MHWRender::MTextureUpdateRegion updateRegion;
									updateRegion.fXRangeMin = minX;
									updateRegion.fXRangeMax = maxX;
									updateRegion.fYRangeMin = minY;
									updateRegion.fYRangeMax = maxY;
									texture->update(newData, generateMipMaps, newWidth*4, &updateRegion);

									delete [] newData;
								}
							}
						}

						delete [] pixelData;
					}
				}

				if(fSaveAsTexture)
				{
					MString savePath = fSaveAsTexturePath.resolvedPath() + fileName;

					if(saveAsFormat.length() > 0)
					{
						savePath = changeExtension(savePath, saveAsFormat);
					}

					const MString extension = fileExtension(savePath);

					MStatus status = MStatus::kFailure;

					try
					{
						status = textureManager->saveTexture( texture, savePath );
					}
					catch(...)
					{
						status = MStatus::kFailure;
					}

					if(status)
					{
						MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kSuccessSaveTexture, savePath, extension ) );
					}
					else
					{
						MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSaveTexture, savePath, extension ) );
					}
				}

				if(fDrawTextures)
				{
					if(renderHelper->renderTextureToScreen(texture))
					{
						// Let the user see the screen ...
						mySleep(1);
					}
				}

				textureManager->releaseTexture(texture);
			}
		}
	}

	delete renderHelper;

	MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kEndLoadTest ) );

	return MStatus::kSuccess;
}

MStatus hwApiTextureTestCmd::saveAsTexture()
{
	MHWRender::MRenderer* renderer = MHWRender::MRenderer::theRenderer();
	if(renderer == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorRenderer ) );
		return MStatus::kFailure;
	}

	MHWRender::MTextureManager* textureManager = renderer->getTextureManager();
	if(textureManager == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorTextureManager ) );
		return MStatus::kFailure;
	}

	if(fSaveAsTexturePath.isSet() == false || fSaveAsTexturePath.exists() == false)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSavePathArg ) );
		return MStatus::kFailure;
	}

	const MStringArray& saveAsFormats = (fSaveAllFormats ? fSupportedFormats : fSaveAsTextureFormats);
	if(saveAsFormats.length() == 0)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSaveFormatArg ) );
		return MStatus::kFailure;
	}

	hwRendererHelper* renderHelper = hwRendererHelper::create(renderer);
	MHWRender::MTexture* texture = renderHelper->createTextureFromScreen();
	delete renderHelper;

	if(texture == NULL)
	{
		MGlobal::displayWarning( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSaveAcquireTexture ) );
		return MStatus::kFailure;
	}

	MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kBeginSaveTest ) );

	for(unsigned int formatIdx = 0; formatIdx < saveAsFormats.length(); ++formatIdx)
	{
		const MString& format = saveAsFormats[formatIdx];
		MString extension = format;

		const MString filePath = fSaveAsTexturePath.resolvedPath() + "hwApiTextureTest." + extension;

		MStatus status = MStatus::kFailure;

		try
		{
			status = textureManager->saveTexture( texture, filePath );
		}
		catch(...)
		{
			status = MStatus::kFailure;
		}

		if(status)
		{
			MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kSuccessSaveTexture, filePath, format ) );
		}
		else
		{
			MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kErrorSaveTexture, filePath, format ) );
		}
	}

	textureManager->releaseTexture(texture);

	MGlobal::displayInfo( hwApiTextureTestStrings::getString( hwApiTextureTestStrings::kEndSaveTest ) );

	return MStatus::kSuccess;
}

MStatus	hwApiTextureTestCmd::listFormats()
{
	MString allFormats;

	for(unsigned int formatIdx = 0; formatIdx < fSupportedFormats.length(); ++formatIdx)
	{
		const MString& format = fSupportedFormats[formatIdx];

		allFormats += format;
		allFormats += MString(L" ");
	}

	MGlobal::displayInfo( allFormats );

	return MStatus::kSuccess;
}

///////////////////////////////////////////////////
//
// Plug-in functions
//
///////////////////////////////////////////////////

MStatus initializePlugin( MObject obj )
{
	MStatus   status;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any");

	// Register string resources
	//
	status = plugin.registerUIStrings( hwApiTextureTestStrings::registerMStringResources, "" );
    if (!status)
	{
        status.perror("registerUIStrings");
        return status;
    }

	// Register the command so we can actually do some work
	//
	status = plugin.registerCommand("hwApiTextureTest",
									hwApiTextureTestCmd::creator,
									hwApiTextureTestCmd::newSyntax);
	if (!status)
	{
		status.perror("registerCommand");
	}

	return status;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus   status;
	MFnPlugin plugin( obj );

	// Deregister the command
	//
	status = plugin.deregisterCommand("hwApiTextureTest");
	if (!status)
	{
		status.perror("deregisterCommand");
	}

	return status;
}


//-
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license agreement
// provided at the time of installation or download, or which otherwise
// accompanies this software in either electronic or hard copy form.
//+
