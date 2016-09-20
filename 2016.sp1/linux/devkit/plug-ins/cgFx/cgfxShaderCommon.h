//
// Copyright (C) 2002-2003 NVIDIA 
// 
// File: cgfxShaderCommon.h

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

#ifndef _cgfxShaderCommon_h_
#define _cgfxShaderCommon_h_

#ifdef _WIN32
#  pragma warning(disable: 4786)
#endif

// This header file simply defines some things that I
// want to use throughout the plug-in
//


// If assertion is false, throw an InternalError exception.
// Note: A Maya MStatus object can be used as the assertion... true means success.
#define M_CHECK(assertion)  if (assertion) ; else throw ((cgfxShaderCommon::InternalError*)__LINE__)

#ifdef DEBUG
#define CGFX_DEBUG 1
#endif

namespace cgfxShaderCommon
{
#ifdef _WIN32
	class InternalError;    // Never defined.  Used like this:
	//   throw (InternalError*)__LINE__;
#else
	struct InternalError
	{
		char* message;
	};
	//   throw (InternalError*)__LINE__;
#endif
}


#define RETURNSTAT(s, msg)	\
	if (!s)					\
{						\
	s.perror(msg);		\
	return s;			\
}


#define lengthof(array) (sizeof(array) / sizeof(array[0]))

#if !defined(TEXTURES_BY_NAME) && !defined(TEXTURES_BY_NODE)
#  define TEXTURES_BY_NODE 1
#endif

#if defined(CGFX_DEBUG) && defined(_WIN32)
#  ifndef _CRTDBG_MAP_ALLOC
#    define _CRTDBG_MAP_ALLOC
#  endif
#endif /* CGFX_DEBUG && _WIN32 */


#include <stdlib.h>

#if defined(_WIN32)
// We must include <stdlib.h> before <crtdbg.h> or we get
// errors about overloading calloc.
//
#  include <crtdbg.h>
#endif /* _WIN32 */

#if defined(CGFX_DEBUG2)
#  define OutputDebugString(s)			fprintf(stderr, "%s", s)
#  define OutputDebugStrings(s1, s2)	fprintf(stderr, "%s%s\n", s1, s2);
#else
#  if defined (_WIN32)
// In optimized mode, send the string to the debugger
#     define OutputDebugStrings(s1, s2) \
	(OutputDebugString(s1), OutputDebugString(s2), OutputDebugString("\n"))
#  else
#    define OutputDebugString(s)		((void) 0)
#    define OutputDebugStrings(s1, s2)	((void) 0)
#  endif
#endif /* CGFX_DEBUG */

// Return true if item is found in Maya array.
template < typename Tarray, typename Titem >
bool
arrayContains( const Tarray& array, const Titem& item )
{
	int i;
	for ( i = array.length(); i > 0; --i )
		if ( array[ i - 1 ] == item )
			break;
	return i > 0;
}


// Append item to Maya array if not already present.
// Returns index of the new or existing array element.
template < typename Tarray, typename Titem >
int
findOrAppend( Tarray& array, const Titem& item )
{
	int i;
	int n = array.length();
	for ( i = 0; i < n; ++i )
		if ( array[ i ] == item )
			break;
	if ( i == n )
		array.append( item );
	return i;
}

#ifndef CGFXSHADER_VERSION  
#define CGFXSHADER_VERSION  "4.4"
#endif

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>

#if defined(LINUX) && !defined(GL_GLEXT_PROTOTYPES)
	#define GL_GLEXT_PROTOTYPES
#endif

#include <maya/MGL.h>	// OpenGL

#include <Cg/cg.h>
#if defined(WIN32) || defined(LINUX)
#else
	typedef void (* PFNGLCLIENTACTIVETEXTUREARBPROC) (GLenum texture);
	typedef void (* PFNGLVERTEXATTRIBPOINTERARBPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
	typedef void (* PFNGLENABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
	typedef void (* PFNGLDISABLEVERTEXATTRIBARRAYARBPROC) (GLuint index);
	typedef void (* PFNGLVERTEXATTRIB4FARBPROC) (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
	typedef void (* PFNGLSECONDARYCOLORPOINTEREXTPROC) (GLint size, GLenum type, GLsizei stride, GLvoid *pointer);
	typedef void (* PFNGLSECONDARYCOLOR3FEXTPROC) (GLfloat red, GLfloat green, GLfloat blue);
	typedef void (* PFNGLMULTITEXCOORD4FARBPROC) (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
    typedef void (* PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
#endif
#include <GL/glext.h>
#include <Cg/cgGL.h>


//
// The gl Registers
//
class glRegister
{
public:
	enum
	{
		kUnknown,
		kPosition,
		kVertexWeight,
		kNormal,
		kColor,
		kSecondaryColor,
		kFogCoord,
		kTexCoord,
		kLastTexCoord = kTexCoord + 7,
		kVertexAttrib,
		kLastVertexAttrib = kVertexAttrib + 15,
		kLast
	};
};


//
// A wee cache to minimise our gl state changes
//
class glStateCache
{
public:
				glStateCache();
	static 	glStateCache& instance() { return gInstance; }
	inline void reset() { fRequiredRegisters = 0; fEnabledRegisters = 0; fActiveTextureUnit = -1; }

	void		flushState();
	inline void disableAll();

	inline void enablePosition();
	inline void enableNormal();
	inline void disableNormal();
	inline void enableColor();
	inline void enableSecondaryColor();
	void		activeTexture( int i);
	inline void enableAndActivateTexCoord( int i);
	void		enableVertexAttrib( int i);

	static int	sMaxTextureUnits;

	// GL extensions
	static PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;
	static PFNGLVERTEXATTRIBPOINTERARBPROC glVertexAttribPointer;
	static PFNGLENABLEVERTEXATTRIBARRAYARBPROC glEnableVertexAttribArray;
	static PFNGLDISABLEVERTEXATTRIBARRAYARBPROC glDisableVertexAttribArray;
	static PFNGLVERTEXATTRIB4FARBPROC glVertexAttrib4f;
	static PFNGLSECONDARYCOLORPOINTEREXTPROC glSecondaryColorPointer;
	static PFNGLSECONDARYCOLOR3FEXTPROC glSecondaryColor3f;
	static PFNGLMULTITEXCOORD4FARBPROC glMultiTexCoord4fARB;


private:

	static glStateCache gInstance;
	long	fRequiredRegisters;
	long	fEnabledRegisters;
	int		fActiveTextureUnit;
};

inline void glStateCache::disableAll() { fRequiredRegisters = 0; flushState(); }

inline void glStateCache::enablePosition() { if( !(fEnabledRegisters & (1 << glRegister::kPosition))) { glEnableClientState(GL_VERTEX_ARRAY); fEnabledRegisters |= (1 << glRegister::kPosition); } fRequiredRegisters |= (1 << glRegister::kPosition); }
inline void glStateCache::enableNormal() { if( !(fEnabledRegisters & (1 << glRegister::kNormal))) { glEnableClientState(GL_NORMAL_ARRAY); fEnabledRegisters |= (1 << glRegister::kNormal); } fRequiredRegisters |= (1 << glRegister::kNormal); }
inline void glStateCache::disableNormal() { if( fEnabledRegisters & (1 << glRegister::kNormal)) { glDisableClientState(GL_NORMAL_ARRAY); fEnabledRegisters &= ~(1 << glRegister::kNormal); } fRequiredRegisters &= ~(1 << glRegister::kNormal); }
inline void glStateCache::enableColor() { if( !(fEnabledRegisters & (1 << glRegister::kColor))) { glEnableClientState(GL_COLOR_ARRAY); fEnabledRegisters |= (1 << glRegister::kColor); }  fRequiredRegisters |= (1 << glRegister::kColor); }
inline void glStateCache::enableSecondaryColor() { if( !(fEnabledRegisters & (1 << glRegister::kSecondaryColor))) { glEnableClientState(GL_SECONDARY_COLOR_ARRAY_EXT); fEnabledRegisters |= (1 << glRegister::kSecondaryColor); }  fRequiredRegisters |= (1 << glRegister::kSecondaryColor); }
inline void glStateCache::enableAndActivateTexCoord( int i) { activeTexture( i); if( !(fEnabledRegisters & (1 << (glRegister::kTexCoord + i)))) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); fEnabledRegisters |= (1 << (glRegister::kTexCoord + i)); } fRequiredRegisters |= (1 << (glRegister::kTexCoord + i)); }

#endif /* _cgfxShaderCommon_h */
