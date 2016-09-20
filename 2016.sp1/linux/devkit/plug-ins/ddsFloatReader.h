//
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
//

#ifndef DDS_FLOAT_READER_H
#define DDS_FLOAT_READER_H

#define DEBUG_DDS( x) 
//#define DEBUG_DDS( x) x

#ifdef _WIN32
#pragma pack(1)
#endif

#ifndef _WIN32
typedef unsigned char BYTE;
typedef unsigned int DWORD;
typedef unsigned short WORD;
#endif

namespace dds_Float_Reader
{
// 
// Our DDS constants
//

#define DDS_MAGIC_NUMBER				0x20534444

#define DDS_CAPS_FLAG					0x00000001
#define DDS_HEIGHT_FLAG					0x00000002
#define DDS_WIDTH_FLAG					0x00000004
#define DDS_PITCH_FLAG					0x00000008
#define DDS_PIXEL_FORMAT_FLAG			0x00001000
#define DDS_MIPMAP_COUNT_FLAG			0x00020000
#define DDS_LINEARSIZE_FLAG				0x00080000
#define DDS_DEPTH_FLAG					0x00800000
#define DDS_HAS_ALPHA_FLAG				0x00000001
#define DDS_FOURCC_FLAG					0x00000004
#define DDS_RGB_FLAG					0x00000040
#define DDS_PALETTEINDEXED8				0x00000020			
#define DDS_PALETTEINDEXED4TO8			0x00000010
#define DDS_PALETTEINDEXED4				0x00000008

#define DDSCAPS_COMPLEX_FLAG			0x00000008
#define DDSCAPS_TEXTURE_FLAG			0x00001000
#define DDSCAPS_MIPMAP_FLAG				0x00400000

#define DDSCAPS2_CUBEMAP_FLAG			0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX_FLAG	0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX_FLAG	0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY_FLAG	0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY_FLAG	0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ_FLAG	0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ_FLAG	0x00008000

#define DDSCAPS2_VOLUME_FLAG			0x00200000

#define DDS_DXT1						0x31545844
#define DDS_DXT2						0x32545844
#define DDS_DXT3						0x33545844
#define DDS_DXT4						0x34545844
#define DDS_DXT5						0x35545844


// Float and half float formats specified in FourCC
#define DDS_R16F						111
#define DDS_G16R16F						112
#define DDS_A16B16G16R16F				113
#define DDS_R32F						114
#define DDS_G32R32F						115
#define DDS_A32B32G32R32F				116

//
// This structure describes the format of the file's pixel data 
//
typedef struct DDS_FORMAT {
	DWORD		fSize;
	DWORD		fFlags;
	DWORD		fPixelFormat; //fFourCC;
	DWORD		fRGBBitCount;
	DWORD		fRedBitMask;
	DWORD		fGreenBitMask;
	DWORD		fBlueBitMask;
	DWORD		fAlphaBitMask;
} DDS_FORMAT;


//
// DDS capabilities structure
//
typedef struct DDSCAPS2 {
    DWORD       dwCaps;         // capabilities of surface wanted
    DWORD       dwCaps2;
    DWORD       dwCaps3;
    union
    {
        DWORD       dwCaps4;
        DWORD       dwVolumeDepth;
    };
} DDSCAPS2;


//
// The header for a DDS file
//
typedef struct DDS_HEADER {
	DWORD		fDDSMagicNumber;
	DWORD		fSize;
	DWORD		fFlags;
	DWORD		fHeight;
	DWORD		fWidth;
	union {
		DWORD		fUncompressedPitch;
		DWORD		fCompressedSize;
	};
	DWORD		fDepth;
	DWORD		fMipMapCount;
	DWORD		fReserved1[11];
	DDS_FORMAT	fFormat;			// Pixel format 
	DDSCAPS2	fCapabilities;
	DWORD		fReserved2;
} DDS_HEADER;

}

#ifdef _WIN32
#pragma pack()
#endif /*WIN32*/

#endif 
