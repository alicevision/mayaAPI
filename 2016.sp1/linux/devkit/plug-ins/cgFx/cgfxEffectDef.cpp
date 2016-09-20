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
//

#include <maya/MHardwareRenderer.h>
#include <maya/MGlobal.h>
#include <maya/MGLFunctionTable.h>
#include "cgfxEffectDef.h"
#include "cgfxFindImage.h"
#include "cgfxShaderNode.h"

#include <sys/stat.h>
#include <map>

#ifdef _WIN32
#else
#	include <sys/timeb.h>
#	include <string.h>
#
#   define stricmp strcasecmp
#   define strnicmp strncasecmp
#endif

#undef ENABLE_TRACE_API_CALLS
#ifdef ENABLE_TRACE_API_CALLS
#define TRACE_API_CALLS(x) cerr << "cgfxShader: "<<(x)<<"\n"
#else
#define TRACE_API_CALLS(x)
#endif

//
// A per-vertex attribute on a shader
//

cgfxVertexAttribute::cgfxVertexAttribute()
 : fNext( NULL), fSourceType( kUnknown), fSourceIndex( 0), refcount(0)
{
}

void cgfxVertexAttribute::release() const
{
    --refcount;
    if (refcount <= 0)
    {
        M_CHECK( refcount == 0 );
        delete this;
    }
}


//
// A varying parameter to a pass
//

cgfxVaryingParameter::cgfxVaryingParameter(CGparameter parameter)
 :	fParameter( parameter),
	fVertexAttribute( NULL),
	fVertexStructure( NULL),
	fNext( NULL)
{
	if( parameter)
	{
		fName = cgGetParameterName( parameter);
	}
}


cgfxVaryingParameter::~cgfxVaryingParameter()
{
	delete fVertexStructure;
	delete fNext;
}


void cgfxVaryingParameter::setupAttributes( cgfxRCPtr<cgfxVertexAttribute>& vertexAttributes, CGprogram program)
{
	// Make sure our parameter name is acceptable is a Maya attribute name
	MString attrName = fName;
	int lastDot = attrName.rindex( '.');
	if( lastDot >= 0)
		attrName = attrName.substring( lastDot + 1, attrName.length() - 1);
	
    MString semanticName = cgGetParameterSemantic( fParameter);

    MString semantic(semanticName);
    cgGetParameterSemantic( fParameter);
	semantic.toUpperCase();

	// Is this varying parameter packed or atomic?
	CGtype type = cgGetNamedUserType( program, attrName.asChar());
	if( type != CG_UNKNOWN_TYPE)
	{
		// It's packed: explode the inputs into the structure elements
		CGcontext context = cgGetProgramContext( program); 
		CGparameter packing = cgCreateParameter( context, type);
		fVertexStructure = new cgfxVaryingParameterStructure();
		fVertexStructure->fLength = 0;
		fVertexStructure->fSize = 0;
		CGparameter element = cgGetFirstStructParameter( packing);
		while( element)
		{
			MString elementName = cgGetParameterName( element);
			int lastDot = elementName.rindex( '.');
			if( lastDot >= 0)
				elementName = elementName.substring( lastDot + 1, elementName.length() - 1);
			cgfxRCPtr<cgfxVertexAttribute> attr = setupAttribute( elementName, semantic, element, vertexAttributes);
			fVertexStructure->fElements[ fVertexStructure->fLength].fVertexAttribute = attr;
			int size = cgGetParameterRows( element) * cgGetParameterColumns( element);
			CGtype type = cgGetParameterBaseType( element);
			if( type == CG_FLOAT) size *= sizeof( GLfloat);
			else if( type == CG_INT) size *= sizeof( GLint);
			fVertexStructure->fElements[ fVertexStructure->fLength].fSize = size;
			fVertexStructure->fLength++;
			fVertexStructure->fSize += size;
			element = cgGetNextParameter( element);
		}
		cgDestroyParameter( packing); 
	}
	else
	{
		// It's atomic - create a single, simple input
		fVertexAttribute = setupAttribute( attrName, semantic, fParameter, vertexAttributes);
	}

	// Now pull apart the semantic string to work out where to bind
	// this value in open GL (as the automagic binding through cgGL
	// didn't work so well when this was written)
	int radix = 1;
	fGLIndex = 0;
	unsigned int length = semantic.length();
	const char* str = semantic.asChar();

	// If sematic is NULL then stop here, bug 327649
	if (length == 0) {
		 fGLType = glRegister::kUnknown;
		 return;
	}

	for(;;)
	{
		char c = str[ length - 1];
		if( c < '0' || c > '9') break;
		fGLIndex += radix * (c - '0');
		radix *= 10;
		--length;
	}
	if( semantic.length() != length)
		semantic = semantic.substring( 0, length - 1);
	
	// Determine the semantic and setup the gl binding type we should use
	// to set this parameter. If there's a sensible default value, set that
	// while we're here.
	// Note there is no need to set the source type, this gets determined
	// when the vertex attribute sources are analysed
	if( semantic == "POSITION")
	{
		fGLType = glRegister::kPosition;
		fVertexAttribute->fSourceName = "position";
	}
	else if( semantic == "NORMAL")
	{
		fGLType = glRegister::kNormal;
		if( fVertexAttribute.isNull() == false ) 
			fVertexAttribute->fSourceName = "normal";
	}
	else if( semantic == "TEXCOORD")
	{
		fGLType = glRegister::kTexCoord;
		if( fVertexAttribute.isNull() == false ) 
		{
			if( attrName.toLowerCase() == "tangent")
				fVertexAttribute->fSourceName = "tangent:map1";
			else if( attrName.toLowerCase() == "binormal")
				fVertexAttribute->fSourceName = "binormal:map1";
			else
				fVertexAttribute->fSourceName = "uv:map1";
		}
	}
	else if( semantic == "TANGENT")
	{
		fGLType = glRegister::kTexCoord;
		fGLIndex += 6; // TANGENT is TEXCOORD6
		if( fVertexAttribute.isNull() == false ) 
			fVertexAttribute->fSourceName = "tangent:map1";
	}
	else if( semantic == "BINORMAL")
	{
		fGLType = glRegister::kTexCoord;
		fGLIndex += 7; // BINORMAL is TEXCOORD7
		if( fVertexAttribute.isNull() == false ) 
			fVertexAttribute->fSourceName = "binormal:map1";
	}
	else if( semantic == "COLOR")
	{
		fGLType = fGLIndex == 1 ? glRegister::kSecondaryColor : glRegister::kColor;
	}
	else if( semantic == "ATTR")
	{
		fGLType = glRegister::kVertexAttrib;
        if( fVertexAttribute.isNull() == false ) 
        {
            fVertexAttribute->fSourceName = semanticName;
        }
	}
	else if( semantic == "PSIZE")
	{
		fGLType = glRegister::kVertexAttrib;
		fGLIndex = 6;
	}
	else
	{
		fGLType = glRegister::kUnknown;
	}
}

cgfxRCPtr<cgfxVertexAttribute> cgfxVaryingParameter::setupAttribute(
    MString name, 
    const MString& semantic, 
    CGparameter parameter, 
    cgfxRCPtr<cgfxVertexAttribute>& vertexAttributes
)
{
	// Does a varying parameter of this name already exist?
	cgfxRCPtr<cgfxVertexAttribute>* attribute = &vertexAttributes;
	while( attribute->isNull() == false )
	{
		if( (*attribute)->fName == name)
		{
			return *attribute;
		}
		attribute = &(*attribute)->fNext;
	}

	// Add a new input for this parameter
	cgfxRCPtr<cgfxVertexAttribute> attr = cgfxRCPtr<cgfxVertexAttribute>(new cgfxVertexAttribute());
	*attribute = attr;

	// Setup the varying parameter description
	attr->fName = name;
	attr->fType = cgGetTypeString( cgGetParameterType( parameter));
	attr->fSemantic = semantic;
	return attr;
}


void cgfxVaryingParameter::bind(
    const MDagPath& shape, cgfxStructureCache* cache,
    int vertexCount, const float * vertexArray,
    int normalsPerVertex, int normalCount, const float ** normalArrays,
    int colorCount, const float ** colorArrays,
    int texCoordCount, const float ** texCoordArrays
) const
{
    bool result = false;
	if( fVertexAttribute.isNull() == false  && fParameter)
	{
		switch( fVertexAttribute->fSourceType)
		{
			case cgfxVertexAttribute::kPosition:
				result = bind( vertexArray, 3);
				break;

			case cgfxVertexAttribute::kNormal:
				if( normalCount > 0 && normalArrays[ 0])
					result = bind( normalArrays[0], 3);
				break;

			case cgfxVertexAttribute::kUV:
				if( texCoordCount > fVertexAttribute->fSourceIndex && texCoordArrays[ fVertexAttribute->fSourceIndex])
					result = bind( texCoordArrays[ fVertexAttribute->fSourceIndex], 2);
				break;

			case cgfxVertexAttribute::kTangent:
				if( normalCount >= normalsPerVertex * fVertexAttribute->fSourceIndex + 1 && normalArrays[ normalsPerVertex * fVertexAttribute->fSourceIndex + 1])
					result = bind( normalArrays[ normalsPerVertex * fVertexAttribute->fSourceIndex + 1], 3);
				break;

			case cgfxVertexAttribute::kBinormal:
				if( normalCount >= normalsPerVertex * fVertexAttribute->fSourceIndex + 2 && normalArrays[ normalsPerVertex * fVertexAttribute->fSourceIndex + 2])
					result = bind( normalArrays[ normalsPerVertex * fVertexAttribute->fSourceIndex + 2], 3);
				break;

			case cgfxVertexAttribute::kColor:
				if( colorCount > fVertexAttribute->fSourceIndex && colorArrays[ fVertexAttribute->fSourceIndex])
					result = bind( colorArrays[ fVertexAttribute->fSourceIndex], 4);
				break;

			default:
				break;
		}
	}
	else if( fVertexStructure && fParameter && vertexCount)
	{
		// Build a unique name for the contents of this structure
		MString structureName;
		structureName += fVertexStructure->fSize;
		for( int i = 0; i < fVertexStructure->fLength; i++)
		{
				cgfxRCPtr<cgfxVertexAttribute> vertexAttribute = fVertexStructure->fElements[ i].fVertexAttribute;
				if( vertexAttribute.isNull() == false ) structureName += fVertexStructure->fElements[ i].fVertexAttribute->fSourceName;
				structureName += fVertexStructure->fElements[ i].fSize;
		}

		// See if this data already exists in the cache
		char* data = cache->findEntry(shape, structureName);

		// If we couldn't find it, add it to the cache
		if (!data)
		{
			// Allocate storage for this structure
			//printf( "Added new cache entry for %s on %s\n", structureName.asChar(), shape.fullPathName().asChar());
			data = cache->addEntry(
                shape, structureName, fVertexStructure->fSize, vertexCount);
			char* dest = data;
			for( int i = 0; i < fVertexStructure->fLength; i++)
			{
				cgfxRCPtr<cgfxVertexAttribute> vertexAttribute = fVertexStructure->fElements[ i].fVertexAttribute;
				if( vertexAttribute.isNull() == false )
				{
					const char* src = NULL;
					int size = 0;
					switch( vertexAttribute->fSourceType)
					{
						case cgfxVertexAttribute::kPosition:
							src = (const char*)vertexArray;
							size = 3 * sizeof( float);
							break;

						case cgfxVertexAttribute::kNormal:
							if( normalCount > 0 && normalArrays[ 0])
							{
								src = (const char*)normalArrays[0];
								size = 3 * sizeof( float);
							}
							break;

						case cgfxVertexAttribute::kUV:
							if( texCoordCount > vertexAttribute->fSourceIndex && texCoordArrays[ vertexAttribute->fSourceIndex])
							{
								src = (const char*)texCoordArrays[ vertexAttribute->fSourceIndex];
								size = 2 * sizeof( float);
							}
							break;

						case cgfxVertexAttribute::kTangent:
							if( normalCount >= normalsPerVertex * vertexAttribute->fSourceIndex + 1 && normalArrays[ normalsPerVertex * vertexAttribute->fSourceIndex + 1])
							{
								src = (const char*)normalArrays[ normalsPerVertex * vertexAttribute->fSourceIndex + 1];
								size = 3 * sizeof( float);
							}
							break;

						case cgfxVertexAttribute::kBinormal:
							if( normalCount >= normalsPerVertex * vertexAttribute->fSourceIndex + 2 && normalArrays[ normalsPerVertex * vertexAttribute->fSourceIndex + 2])
							{
								src = (const char*)normalArrays[ normalsPerVertex * vertexAttribute->fSourceIndex + 2];
								size = 3 * sizeof( float);
							}
							break;

						case cgfxVertexAttribute::kColor:
							if( colorCount > vertexAttribute->fSourceIndex && colorArrays[ vertexAttribute->fSourceIndex])
							{
								src = (const char*)colorArrays[ vertexAttribute->fSourceIndex];
								size = 4 * sizeof( float);
							}
							break;

						default:
							break;
					}

					// Do we have a valid input?
					if( src && size)
					{
						// Setup this element
						int srcSkip = 0;
						if( size > fVertexStructure->fElements[ i].fSize)
						{
							srcSkip = size - fVertexStructure->fElements[ i].fSize;
							size = fVertexStructure->fElements[ i].fSize;
						}
						int dstSkip = fVertexStructure->fSize - size;
						char* dst = dest;
						for( int v = 0; v < vertexCount; v++)
						{
							for( int b = 0; b < size; b++)
								*dst++ = *src++;
							src += srcSkip;
							dst += dstSkip;
						}
					}
					else
					{
						// NULL this element
						size = fVertexStructure->fElements[ i].fSize;
						int dstSkip = fVertexStructure->fSize - size;
						char* dst = dest;
						for( int v = 0; v < vertexCount; v++)
						{
							for( int b = 0; b < size; b++)
								*dst++ = 0;
							dst += dstSkip;
						}
					}
				}
				dest += fVertexStructure->fElements[ i].fSize;
			}
		}

		result = bind( (const float*)data, fVertexStructure->fSize / sizeof( float));
	}

	// If we were unable to bind a stream of data to this register, set a friendly NULL value
	if( !result)
		null();
}


//
// Bind data to GL
//
bool cgfxVaryingParameter::bind( const float* data, int stride) const
{
	bool result = false;
	switch( fGLType)
	{
		case glRegister::kPosition:
			glStateCache::instance().enablePosition();
			glVertexPointer( stride, GL_FLOAT, 0, data);
			result = true;
			break;

		case glRegister::kNormal:
			if( stride == 3)
			{
				glStateCache::instance().enableNormal();
				glNormalPointer( GL_FLOAT, 0, data);
				result = true;
			}
			break;

		case glRegister::kTexCoord:
			if( fGLIndex < glStateCache::sMaxTextureUnits)
			{
				glStateCache::instance().enableAndActivateTexCoord( fGLIndex);
				glTexCoordPointer( stride, GL_FLOAT, 0, data);
				result = true;
			}
			break;

		case glRegister::kColor:
			if( stride > 2)
			{
				glStateCache::instance().enableColor();
				glColorPointer( stride, GL_FLOAT, 0, data);
				result = true;
			}
			break;

		case glRegister::kSecondaryColor:
			if( stride > 2)
			{
				glStateCache::instance().enableSecondaryColor();
				if( glStateCache::glVertexAttribPointer) 
					glStateCache::glSecondaryColorPointer( stride, GL_FLOAT, 0, (GLvoid*)data);
				result = true;
			}
			break;

		case glRegister::kVertexAttrib:
			glStateCache::instance().enableVertexAttrib( fGLIndex);
			if( glStateCache::glVertexAttribPointer) 
				glStateCache::glVertexAttribPointer( fGLIndex, stride, GL_FLOAT, GL_FALSE, 0, data);
			result = true;
			break;

		/// TODO add secondaryColor, vertexWeight, vertexAttrib, fog
		default:
			break;
	}
	return result;
}

bool cgfxVaryingParameter::bind(const sourceStreamInfo& source) const
{
	// should assert(dataBufferId > 0) here
	// ...
	
	static MGLFunctionTable *gGLFT = 0;
    if ( 0 == gGLFT )
        gGLFT = MHardwareRenderer::theRenderer()->glFunctionTable();
	
	const unsigned int stride = source.fStride;
	const unsigned int offset = source.fOffset;
	const unsigned int dimension  = source.fDimension;
	const unsigned int elementSize  = source.fElementSize;
	const GLuint bufferId = source.fDataBufferId;

	gGLFT->glBindBufferARB(MGL_ARRAY_BUFFER_ARB, bufferId);

	#define GLOBJECT_BUFFER_OFFSET(i) ((char *)NULL + (i)) // For GLObject offsets

	switch( fGLType)
	{
		case glRegister::kPosition:
			glStateCache::instance().enablePosition();
			glVertexPointer(dimension, GL_FLOAT, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));
			break;

		case glRegister::kNormal:
			glStateCache::instance().enableNormal();
			glNormalPointer(GL_FLOAT, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));
			break;

		case glRegister::kTexCoord:
			if( fGLIndex < glStateCache::sMaxTextureUnits)
			{
				glStateCache::instance().enableAndActivateTexCoord( fGLIndex);
				glTexCoordPointer(dimension, GL_FLOAT, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));
			}
			break;

		case glRegister::kColor:
			glStateCache::instance().enableColor();
			glColorPointer(dimension, GL_FLOAT, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));
			break;

		case glRegister::kSecondaryColor:
			glStateCache::instance().enableSecondaryColor();
			if( glStateCache::glVertexAttribPointer) 
				glStateCache::glSecondaryColorPointer(dimension, GL_FLOAT, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));
			break;

		case glRegister::kVertexAttrib:
			glStateCache::instance().enableVertexAttrib( fGLIndex);
			if( glStateCache::glVertexAttribPointer) 
				glStateCache::glVertexAttribPointer( fGLIndex, dimension, GL_FLOAT, GL_FALSE, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));

			if(source.fSourceType == cgfxVertexAttribute::kPosition) {
				glStateCache::instance().enablePosition();
				glVertexPointer(dimension, GL_FLOAT, stride*elementSize, GLOBJECT_BUFFER_OFFSET(offset));
			}
			break;

		/// TODO add secondaryColor, vertexWeight, vertexAttrib, fog
		default:
			return false;  //these we don't support yet
	}

	return true;
}



//
// Send null data to GL
//
void cgfxVaryingParameter::null() const
{
	switch( fGLType)
	{
		case glRegister::kPosition:
			//null position is not expected, give a warning
			MGlobal::displayWarning( "There is no position data!" );
			break;

		case glRegister::kNormal:
			glNormal3f( 0.0f, 0.0f, 1.0f);
			break;

		case glRegister::kTexCoord:
			glStateCache::instance().activeTexture( fGLIndex);
			glStateCache::glMultiTexCoord4fARB( GL_TEXTURE0 + fGLIndex, 0.0f, 0.0f, 0.0f, 0.0f );
			break;

		case glRegister::kColor:
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f);
			break;

		case glRegister::kSecondaryColor:
			if( glStateCache::glSecondaryColor3f) 
				glStateCache::glSecondaryColor3f( 1.0f, 1.0f, 1.0f);
			break;

		case glRegister::kVertexAttrib:
			if( glStateCache::glVertexAttrib4f) 
				glStateCache::glVertexAttrib4f( fGLIndex, 0.0f, 0.0f, 0.0f, 0.0f);
			break;

		/// TODO add secondaryColor, vertexWeight, vertexAttrib, fog
		default:
			break;
	}
}



inline void cgfxVaryingParameter::addRecursive(
    CGparameter parameter,
    cgfxVaryingParameter**& nextParameter
)
{
	if( cgGetParameterVariability( parameter) == CG_VARYING)
	{
		if( cgGetParameterType( parameter) == CG_STRUCT)
		{
			CGparameter input = cgGetFirstStructParameter( parameter);
			while( input)
			{
				addRecursive( input, nextParameter);
				input = cgGetNextParameter( input);
			}
		}
		else if( cgIsParameterReferenced( parameter))
		{
			*nextParameter = new cgfxVaryingParameter( parameter);
			nextParameter = &(*nextParameter)->fNext;
		}
	}
}


//
// A pass in a technique
//

cgfxPass::cgfxPass(
    CGpass                  pass,
    const cgfxProfile*      profile
)
 :	fPass( pass),
	fProgram( NULL),
	fParameters( NULL),
    fDefaultProfile("default", pass),
	fNext( NULL)
{
	if( pass)
	{
		fName = cgGetPassName( pass);
		CGstateassignment stateAssignment = cgGetFirstStateAssignment( pass);
		cgfxVaryingParameter** nextParameter = &fParameters;
		while( stateAssignment )
		{
			CGstate state = cgGetStateAssignmentState( stateAssignment);
			if( cgGetStateType( state) == CG_PROGRAM_TYPE && 
					( stricmp( cgGetStateName( state), "vertexProgram") == 0 ||
					  stricmp( cgGetStateName( state), "vertexShader") == 0))
			{
				fProgram = cgGetProgramStateAssignmentValue( stateAssignment);
				if( fProgram)
				{
					CGparameter parameter = cgGetFirstParameter( fProgram, CG_PROGRAM);
					while( parameter)
					{
                        cgfxVaryingParameter::addRecursive( parameter, nextParameter);
						parameter = cgGetNextParameter( parameter);
					}
				}
			}
            setProfile(profile);
			stateAssignment = cgGetNextStateAssignment( stateAssignment);
		}
	}
}


cgfxPass::~cgfxPass()
{
	delete fNext;
	delete fParameters;
}


void cgfxPass::setupAttributes(cgfxRCPtr<cgfxVertexAttribute>& vertexAttributes) const
{
	cgfxVaryingParameter* parameter = fParameters;
	while( parameter)
	{
		parameter->setupAttributes( vertexAttributes, fProgram);
		parameter = parameter->fNext;
	}
}

void cgfxPass::setProfile(const cgfxProfile* profile) const
{
    // If profile is null, we use the default Cg profile, i.e. the
    // profile specified in the .cgfx file.
    if (profile == NULL) profile = &fDefaultProfile;

    CGprogram vp = cgGetPassProgram(fPass, CG_VERTEX_DOMAIN);
    if (vp != NULL &&
        cgGetProgramProfile(vp) != profile->getVertexProfile())
    {
        cgSetProgramProfile(vp, profile->getVertexProfile());
    }
    CGprogram gp = cgGetPassProgram(fPass, CG_GEOMETRY_DOMAIN);
    if (gp != NULL &&
        profile->getGeometryProfile() != CG_PROFILE_UNKNOWN &&
        cgGetProgramProfile(gp) != profile->getGeometryProfile())
    {
        cgSetProgramProfile(gp, profile->getGeometryProfile());
    }
    CGprogram fp = cgGetPassProgram(fPass, CG_FRAGMENT_DOMAIN);
    if (fp != NULL &&
        cgGetProgramProfile(fp) != profile->getFragmentProfile())
    {
        cgSetProgramProfile(fp, profile->getFragmentProfile());
    }
}

void cgfxPass::bind(
    const MDagPath& shape, cgfxStructureCache* cache,
    int vertexCount, const float * vertexArray,
    int normalsPerVertex, int normalCount, const float ** normalArrays,
    int colorCount, const float ** colorArrays,
    int texCoordCount, const float ** texCoordArrays
) const
{
	cgfxVaryingParameter* parameter = fParameters;
	while( parameter)
	{
		parameter->bind(shape, cache, 
						vertexCount, vertexArray, 
                        normalsPerVertex, normalCount, normalArrays, 
                        colorCount, colorArrays, 
                        texCoordCount, texCoordArrays);
		parameter = parameter->fNext;
	}
}

void cgfxPass::bind(const sourceStreamInfo dataSources[], const int sourceCount) const
{
	TRACE_API_CALLS("cgfxPass::bind");

	cgfxVaryingParameter* parameter = fParameters;
	while( parameter)
	{
		// Here we only deal with fVertexAttribute.  How to do fVertexStructure?
		if (parameter->fVertexAttribute.isNull() == false) {
            //find the corresponding data buffer
            int index = 0;
            for(index = 0; index < sourceCount; ++index)
            {
                // printf(
                // "    Compare param %s (0x%p) [%s] name (type=%d) with data source name [%s] (type=%d)\n", 
                //         parameter->fName.asChar(),
                //         parameter,
                //         parameter->fVertexAttribute->fSourceName.asChar(), 
                //         parameter->fVertexAttribute->fSourceType,
                //         dataSources[index].fSourceName.asChar(),
                //         dataSources[index].fSourceType);

                if (dataSources[index].fSourceName == parameter->fVertexAttribute->fSourceName )
                {
                    // we find the correct vertex stream
                    break;
                }
            }	

            if(index < sourceCount)
            {
                // printf("    Binding source name [%s]\n", dataSources[index].fSourceName.asChar());
                if(!parameter->bind(dataSources[index])) {

                    // This is a true error. Binding should normally
                    // always succeed here as the geometry
                    // requirements are verified in
                    // cgfxShaderOverride::initialize().
                    parameter->null();

                    MString s = "cgfxShader : Couldn't bind source \"";
                    s += dataSources[index].fSourceName;
                    s += "\" for vertex attribute \"";
                    s += parameter->fVertexAttribute->fSourceName;;
                    s +="\".";
                    MGlobal::displayError(s);
                }
            }
            else
            {
                // printf("    Can't find the source for source name [%s] for parameter [%s]\n", 
                //        parameter->fVertexAttribute->fSourceName.asChar(),
                //        parameter->fVertexAttribute->fName.asChar());

                // There is no matching source for this parameter. We
                // therefore bind null data for this parameter. Note
                // that this fact should have already been reported
                // to the user in cgfxShaderOverride::initialize(). We
                // don't report it to the user here because it would
                // get repetitively reported for each redraw.
                parameter->null();
            }
        }
        
		parameter = parameter->fNext;
	}

    // printf("    Successfully bound sources\n");
}

//
// A technique in an effect
//

cgfxTechnique::cgfxTechnique(
    CGtechnique         technique,
    const cgfxProfile*  profile
)
:	fTechnique( technique),
    fValid(false),
	fPasses(NULL),
    fNumPasses(0),
	fNext(NULL)
{
	if (technique)
	{
		fName = cgGetTechniqueName(technique);
		CGpass pass = cgGetFirstPass(technique);
		cgfxPass** nextPass = &fPasses;
		while (pass)
		{
            ++fNumPasses;
            *nextPass = new cgfxPass(pass, profile);
            nextPass = &(*nextPass)->fNext;
			pass = cgGetNextPass(pass);
		}

        fHasBlending = hasBlending(fTechnique);

        setProfile(profile);
	}
}

cgfxTechnique::~cgfxTechnique()
{
	delete fNext;
	delete fPasses;
    fNext = 0;
    fPasses = 0;
}

void cgfxTechnique::setProfile(const cgfxProfile* profile) const
{
    const cgfxProfile* supportedProfile = getSupportedProfile(profile);
    
    const cgfxPass* pass = fPasses;
    while (pass) {
        pass->setProfile(supportedProfile);
        pass = pass->fNext;
    }

    // Changing the profile might change the validity of the technique.
    validate();
}

void cgfxTechnique::validate() const
{
    fValid = (cgValidateTechnique(fTechnique) == CG_TRUE);
    if (fValid) {
        fErrorString = "";
    }
    else {
        CGerror error = cgGetError();
        if (error != CG_NO_ERROR) {
            fErrorString = cgGetErrorString(cgGetError());
        }
        fErrorString += "\nCg compilation errors for technique \"";
        fErrorString += fName;
        fErrorString += "\":\n";
        fErrorString += cgGetLastListing(cgfxShaderNode::sCgContext);
        fErrorString += "\n";
    }
}

const cgfxProfile* cgfxTechnique::getSupportedProfile(const cgfxProfile* profile) const
{
    if (profile == NULL) {
        // The user wants to use the default profile. Let's see if
        // this is supported on the current platform.
        bool allPassProfilesSupported = true;
        const cgfxPass* pass = fPasses;
        while (pass) {
            if (!pass->fDefaultProfile.isSupported()) {
                allPassProfilesSupported = false;
                break;
            }
            pass = pass->fNext;
        }
        if (allPassProfilesSupported) {
            // Ok. We can use the default profile!
            return NULL;
        }
        else {
            MString es;
            es += "The technique \"";
            es += fName;
            es += "\" specifies Cg profiles that are unsupported on this platform. "
                "The profile \"";
            es += cgfxProfile::getBestProfile()->getName();
            es += "\" will be used instead.";
            MGlobal::displayWarning(es);
        
            return cgfxProfile::getBestProfile();
        }
    }
    else {
        return profile;
    }
}

cgfxRCPtr<cgfxVertexAttribute> cgfxTechnique::getVertexAttributes() const
{
    cgfxRCPtr<cgfxVertexAttribute> vertexAttributes;
    
    const cgfxPass* pass = fPasses;
    while (pass) {
        pass->setupAttributes(vertexAttributes);
        pass = pass->fNext;
    }

    return vertexAttributes;
}

//
// Scan the technique for passes which use blending
//
bool cgfxTechnique::hasBlending(CGtechnique technique)
{
	// Assume not blending
	bool hasBlending = false;

	// Check for : BlendEnable=true, BlendFunc=something valid on the first pass only.
	//
	// We ignore any depth enable and functions for now...
	//
	CGpass cgPass = cgGetFirstPass(technique);
	bool foundBlendEnabled = false;
	bool foundBlendFunc = false;
	if (cgPass)
	{
		CGstateassignment stateAssignment = cgGetFirstStateAssignment(cgPass);
		while ( stateAssignment )
		{
			CGstate state = cgGetStateAssignmentState( stateAssignment);
			const char *stateName = cgGetStateName(state);

			// Check for blend enabled.
			if (!foundBlendEnabled && stricmp( stateName, "BlendEnable") == 0)
			{
				int numValues = 0;
				const CGbool *values = cgGetBoolStateAssignmentValues(stateAssignment, &numValues);
				if (values && numValues)
				{
					if (values[0])
					{
						foundBlendEnabled = true;
					}
				}
			}

			// Check for valid blend function
			else if (!foundBlendFunc && ( stricmp( stateName, "BlendFunc") == 0 ||
										  stricmp( stateName, "BlendFuncSeparate") == 0 ))
			{
				int numValues = 0;
				const int * values = cgGetIntStateAssignmentValues(stateAssignment, &numValues);
				if (values)
				{
#if defined(CGFX_DEBUG_BLEND_FUNCTIONS)
					/*
					#define GL_SRC_COLOR                      0x0300 = 768
					#define GL_ONE_MINUS_SRC_COLOR            0x0301 = 769
					#define GL_SRC_ALPHA                      0x0302 = 770
					#define GL_ONE_MINUS_SRC_ALPHA            0x0303 = 771
					#define GL_DST_ALPHA                      0x0304 = 772
					#define GL_ONE_MINUS_DST_ALPHA            0x0305 = 773
					*/
					MString blendStringTable[6] =
					{
						"GL_SRC_COLOR", // SrcColor
						"GL_ONE_MINUS_SRC_COLOR", // OneMinusSrcColor
						"GL_SRC_ALPHA", // SrcAlpha
						"GL_ONE_MINUS_SRC_ALPHA", // OneMinusSrcAlpha
						"GL_DST_ALPHA", // DstAlpha
						"GL_ONE_MINUS_DST_ALPHA" // OneMinusDstAlpha
					};
#endif
					for (int i=0; i<numValues; i++)
					{
						if ((values[i] >= GL_SRC_COLOR) && (values[i] <= GL_ONE_MINUS_DST_ALPHA))
						{
#if defined(CGFX_DEBUG_BLEND_FUNCTIONS)
							printf("Found blend function = %s, %s\n",
							blendStringTable[ values[0]-GL_SRC_COLOR].asChar(),
							blendStringTable[ values[1]-GL_SRC_COLOR].asChar());
#endif
							foundBlendFunc = true;
							break;
						}
					}
				}
			}
			hasBlending = foundBlendEnabled && foundBlendFunc;
			if (hasBlending)
				break;
			stateAssignment = cgGetNextStateAssignment( stateAssignment);
		}
	}

    return hasBlending;
}


namespace cgfxEffectInternal
{
	time_t fileTimeStamp(const MString& fileName)
	{
		struct stat statBuf;
		if( stat(fileName.asChar(), &statBuf) != 0 )
			return 0;

		return statBuf.st_mtime;
	}

	struct EffectKey
	{
		const cgfxProfile* profile;
		MString fileName;
		time_t timeStamp;
	};

	bool operator< (const EffectKey& lhs, const EffectKey& rhs)
	{
		return (lhs.profile <  rhs.profile) ||
			   (lhs.profile == rhs.profile && ( (lhs.timeStamp <  rhs.timeStamp) ||
											    (lhs.timeStamp == rhs.timeStamp && strcmp(lhs.fileName.asChar(), rhs.fileName.asChar()) < 0) ) );
	}

	// Collection of effects
	// The collection does not use smart pointer (cgfxRCPtr) to store the effects
	// Otherwise they will never get released (refCount always >= 1)
	// Effect will have to be manually removed from collection by desctructor (~cgfxEffect())
	class cgfxEffectCollection
	{
	public:
		cgfxEffect* find(const MString& fileName, const cgfxProfile* profile) const;
		void add(cgfxEffect* effect, const MString& fileName, const cgfxProfile* profile);
		void remove(cgfxEffect* effect);

	private:
		typedef std::map< cgfxEffect*, EffectKey > Effect2KeyMap;
		Effect2KeyMap effect2KeyMap;

		typedef std::map< EffectKey, cgfxEffect* > Key2EffectMap;
		Key2EffectMap key2EffectMap;
	};


	cgfxEffect* cgfxEffectCollection::find(const MString& fileName, const cgfxProfile* profile) const
	{
		cgfxEffect* effect = NULL;

		EffectKey key = { profile, fileName, fileTimeStamp(fileName) } ;

		Key2EffectMap::const_iterator it = key2EffectMap.find(key);
		if(it != key2EffectMap.end())
		{
			effect = it->second;
		}

		return effect;
	}

	void cgfxEffectCollection::add(cgfxEffect* effect, const MString& fileName, const cgfxProfile* profile)
	{
		EffectKey key = { profile, fileName, fileTimeStamp(fileName) } ;

		key2EffectMap.insert( std::make_pair(key, effect) );
		effect2KeyMap.insert( std::make_pair(effect, key) );
	}

	void cgfxEffectCollection::remove(cgfxEffect* effect)
	{
		Effect2KeyMap::iterator it = effect2KeyMap.find(effect);
		if(it != effect2KeyMap.end())
		{
			key2EffectMap.erase( it->second );
			effect2KeyMap.erase( it );
		}
	}

	static cgfxEffectCollection gEffectsCollection;
}

cgfxRCPtr<const cgfxEffect> cgfxEffect::loadEffect(const MString& fileName, const cgfxProfile* profile)
{
	cgfxEffect *effect = cgfxEffectInternal::gEffectsCollection.find(fileName, profile);
	if(effect == NULL)
	{
		effect = new cgfxEffect(fileName, profile);
		cgfxEffectInternal::gEffectsCollection.add(effect, fileName, profile);
	}

	return cgfxRCPtr<const cgfxEffect>(effect);
}

//
// An effect
//
cgfxEffect::cgfxEffect(const MString& fileName, const cgfxProfile* profile)
  :	refcount(0),
    fEffect(NULL),
	fTechniques(NULL),
    fProfile(NULL)
{
    MStringArray fileOptions;
    cgfxGetFxIncludePath( fileName, fileOptions );
    fileOptions.append("-DMAYA_CGFX=1");

    if (cgfxProfile::getTexCoordOrientation() == cgfxProfile::TEXCOORD_OPENGL) {
        fileOptions.append("-DMAYA_TEXCOORD_ORIENTATION_OPENGL=1");
    }
    else {
        fileOptions.append("-DMAYA_TEXCOORD_ORIENTATION_DIRECTX=1");
    }
    
    const char *opts[_CGFX_PLUGIN_MAX_COMPILER_ARGS_];
    unsigned int numOpts = fileOptions.length();
    if (numOpts)
    {
        numOpts = (numOpts > _CGFX_PLUGIN_MAX_COMPILER_ARGS_-1) ?
            _CGFX_PLUGIN_MAX_COMPILER_ARGS_-1 : numOpts;
        for (unsigned int i=0; i<numOpts; i++)
            opts[i] = fileOptions[i].asChar();
        opts[numOpts] = NULL;
    }

    fEffect = cgCreateEffectFromFile(cgfxShaderNode::sCgContext, fileName.asChar(), opts);
    if (fEffect)
    {
        CGtechnique technique = cgGetFirstTechnique(fEffect);
        cgfxTechnique** nextTechnique = const_cast<cgfxTechnique**>(&fTechniques);
        while (technique)
        {
            *nextTechnique = new cgfxTechnique(technique, profile);
            nextTechnique = &(*nextTechnique)->fNext;
            technique = cgGetNextTechnique(technique);
        }

        fProfile = profile;
    }
}


cgfxEffect::~cgfxEffect()
{
	// Remove this effect from the collection
	cgfxEffectInternal::gEffectsCollection.remove(this);

    delete fTechniques;

    if (fEffect) {
        cgDestroyEffect(fEffect);
        fEffect = NULL;
    }
    
    fTechniques = NULL;
}


void cgfxEffect::release() const
{
    --refcount;
    if (refcount <= 0)
    {
        M_CHECK( refcount == 0 );
        delete this;
    }
}
    

const cgfxTechnique* cgfxEffect::getTechnique(MString techniqueName) const
{
	const cgfxTechnique* technique = fTechniques;
	while(technique)
	{
		if(technique->fName == techniqueName)
		{
			break;
		}
		technique = technique->fNext;
	}

    return technique;
}


void cgfxEffect::setProfile(const cgfxProfile* profile) const
{
    if (fProfile != profile) {
        fProfile = profile;

        const cgfxTechnique* technique = fTechniques;
        while(technique)
        {
            technique->setProfile(profile);
            technique = technique->fNext;
        }
    }
}

// ========== cgfxEffect::attrsFromEffect ==========
//
// This function parses through an effect and builds a list of
// cgfxAttrDef objects.
//
cgfxRCPtr<cgfxAttrDefList> cgfxEffect::attrsFromEffect() const
{
	if (!fEffect)
        return cgfxRCPtr<cgfxAttrDefList>();

	cgfxRCPtr<cgfxAttrDefList> list(new cgfxAttrDefList);

    CGparameter cgParameter = cgGetFirstEffectParameter(fEffect);
    int i = 0;
    while (cgParameter)
    {
        cgfxAttrDef* aDef = new cgfxAttrDef(cgParameter);
        list->add(aDef);

        cgParameter = cgGetNextParameter(cgParameter);
        ++i;
    } // end of for each parameter

    return list;
}

cgfxStructureCache::cgfxStructureCache()
    : fEntries(NULL)
{}

cgfxStructureCache::~cgfxStructureCache()
{
    flush();
}

cgfxStructureCache::Entry::Entry(
    const MDagPath& shape, const MString& name, int stride, int count
)
  : fShape(shape.node()),
    fName(name),
    fData(new char[ stride * count])
{
}

cgfxStructureCache::Entry::~Entry()
{
    delete[] fData;
    fNext = 0;
}

char* cgfxStructureCache::findEntry(const MDagPath& shape, const MString& name)
{
    Entry** entry = &fEntries;
    
    while (*entry)
    {
        if( !(*entry)->fShape.isValid() || !(*entry)->fShape.isAlive())
        {
            Entry* staleEntry = *entry;
            *entry = staleEntry->fNext;
            delete staleEntry;
        }
        else 
        {
            if( (*entry)->fShape == shape.node() && (*entry)->fName == name)
            {
                return (*entry)->fData;
            }
            entry = &(*entry)->fNext;
        }
    }

    return NULL;
}

char* cgfxStructureCache::addEntry(
    const MDagPath& shape,
    const MString&  name,
    int             stride,
    int             count
)
{
    Entry* cacheEntry = new Entry(shape, name, stride, count);

    cacheEntry->fNext = fEntries;
    fEntries = cacheEntry;

    return cacheEntry->fData;
}

void cgfxStructureCache::flush()
{
    delete fEntries;
    fEntries = NULL;
}


void cgfxStructureCache::flush(const MDagPath& shape)
{
	Entry** cacheEntry = &fEntries;
	while( *cacheEntry)
	{
		if( !(*cacheEntry)->fShape.isValid() || !(*cacheEntry)->fShape.isAlive() ||
            (*cacheEntry)->fShape == shape.node())
		{
			Entry* staleEntry = (*cacheEntry);
			*cacheEntry = staleEntry->fNext;
			staleEntry->fNext = NULL;
			delete staleEntry;
		}
		else
		{
			cacheEntry = &(*cacheEntry)->fNext;
		}
	}
}
