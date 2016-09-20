//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

///////////////////////////////////////////////////////////////////
// DESCRIPTION: 
//	Simple Image File plugin. This plugin registers a new image
//  file format against file extension ".moo". Loading any ".moo"
//  image file will produce a procedurally generated colour 
//  spectrum including values outside 0 to 1.
//
///////////////////////////////////////////////////////////////////

#include <maya/MPxImageFile.h>
#include <maya/MImageFileInfo.h>
#include <maya/MImage.h>
#include <maya/MFnPlugin.h>
#include <maya/MStringArray.h>
#include <maya/MIOStream.h>
#include <maya/MGL.h>

MString kImagePluginName( "SimpleImageFile");

class SimpleImageFile : public MPxImageFile
{
public:
                    SimpleImageFile();
    virtual         ~SimpleImageFile();
    static void*	creator();
	virtual MStatus open( MString pathname, MImageFileInfo* info);
	virtual MStatus load( MImage& image, unsigned int idx);
	virtual MStatus glLoad( const MImageFileInfo& info, unsigned int imageNumber);

private:
	void			populateTestImage( float* pixels, unsigned int w, unsigned int h);
};

//
// DESCRIPTION:
///////////////////////////////////////////////////////
SimpleImageFile::SimpleImageFile()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
SimpleImageFile::~SimpleImageFile()
{
}

//
// DESCRIPTION:
///////////////////////////////////////////////////////
void * SimpleImageFile::creator()
{
    return new SimpleImageFile();
}

//
// DESCRIPTION:
// Configure the image characteristics. A real image file
// format plugin would extract these values from the image
// file header. 
//
///////////////////////////////////////////////////////
MStatus SimpleImageFile::open( MString pathname, MImageFileInfo* info)
{
	if( info)
	{
		info->width( 512);
		info->height( 512);
		info->channels( 3);
		info->pixelType( MImage::kFloat);

		// Only necessary if your format defines a native
		// hardware texture loader
		info->hardwareType( MImageFileInfo::kHwTexture2D);
	}
	return MS::kSuccess;
}


//
// DESCRIPTION:
// Internal helper method to populate our procedural test image
//
///////////////////////////////////////////////////////
void SimpleImageFile::populateTestImage( float* pixels, unsigned int w, unsigned int h)
{
	#define RAINBOW_SCALE 4.0f
	unsigned int x, y;
	for( y = 0; y < h; y++)
	{
		float g = RAINBOW_SCALE * y / (float)h;
		for( x = 0; x < w; x++)
		{
			float r = RAINBOW_SCALE * x / (float)w;
			*pixels++ = r;
			*pixels++ = g;
			*pixels++ = RAINBOW_SCALE * 1.5f - r - g;
		}
	}
}


//
// DESCRIPTION:
// Load the contents of this image file into an MImage. A real
// file format plugin would extract the pixel data from the image
// file here.
//
///////////////////////////////////////////////////////
MStatus SimpleImageFile::load( MImage& image, unsigned int)
{
	unsigned int w = 512;
	unsigned int h = 512;

	// Create a floating point image and fill it with
	// a pretty rainbow test image.
	//
	image.create( w, h, 3, MImage::kFloat);
	populateTestImage( image.floatPixels(), w, h);
	return MS::kSuccess;
}


//
// DESCRIPTION:
// Load the contents of this image file into an OpenGL texture. A 
// real file format plugin would extract the pixel data from the 
// image file here.
//
///////////////////////////////////////////////////////
MStatus SimpleImageFile::glLoad( const MImageFileInfo& info, unsigned int)
{
	// Create a floating point image
	unsigned int w = info.width();
	unsigned int h = info.height();
	float* pixels = new float[ w * h * 3];
	populateTestImage( pixels, w, h);
	
	// Now load it into OpenGL as a floating point image
	::glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_FLOAT, pixels);

	delete[] pixels;
	return MS::kSuccess;
}


// DESCRIPTION:
//////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
    MFnPlugin plugin( obj, PLUGIN_COMPANY, "8.0", "Any" );
	MStringArray extensions;
	extensions.append( "moo");
    CHECK_MSTATUS( plugin.registerImageFile(
					kImagePluginName,
					SimpleImageFile::creator, 
					extensions));
    
    return MS::kSuccess;
}

// DESCRIPTION:
///////////////////////////////////////////////////////
MStatus uninitializePlugin( MObject obj )
{
    MFnPlugin plugin( obj );
    CHECK_MSTATUS( plugin.deregisterImageFile( kImagePluginName ) );

    return MS::kSuccess;
}

