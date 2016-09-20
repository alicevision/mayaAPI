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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

/*******************************************************************\
 								 
    Name:	Pic.h						 
 								 
    Purpose:							 
 	Type declarations and associated macros for use		 
 	with picture files (raw PPM files).				 
 								 
\*******************************************************************/


/*
 *  Data structures
 */

typedef unsigned char   boolean;

#ifndef FALSE
#define   FALSE   ((boolean) 0)
#endif
#ifndef TRUE
#define   TRUE    ((boolean) 1)
#endif

typedef unsigned char  Pic_byte;

typedef struct Pic_Pixel
{
    Pic_byte   r, g, b;
}
Pic_Pixel;

typedef struct
{
    FILE     *fptr;
    char     *filename;

    short     width;
    short     height;
    short     scanline;
}
Pic;


/*
 *  Memory allocation and other macros :
 */

#define StrAlloc(n)    ((char *) malloc((unsigned)(n)))
#define PixelAlloc(n)  ((Pic_Pixel *) malloc((unsigned)((n)*sizeof(Pic_Pixel))))
#define PixelFree(p)   ((void) free((char *)(p)))


/*
 *  General routines
 */

extern	Pic       *PicOpen(const char* filename, short width, short height );
extern	boolean    PicWriteLine(Pic* ppmFile, Pic_Pixel* pixels);
extern  void       PicClose(Pic* ppmFile);

#ifdef __cplusplus
}
#endif

/*** THE END ***/
