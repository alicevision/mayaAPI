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

// Utilities for dealing with varying attribute on CgFX shaders
//
//
#ifndef _cgfxEffectDef_h_
#define _cgfxEffectDef_h_

// This class holds the definition of a CG effect, including the 
// techniques, passes and varying parameters that it includes. Uniform
// parameters (which apply to all techniques and all passes) are 
// handled in cgfxAttrDef.h/cpp
//

#include "cgfxShaderCommon.h"
#include "cgfxProfile.h"
#include "cgfxRCPtr.h"

class cgfxEffect;
class cgfxAttrDefList;

#include <maya/MDagPath.h>
#include <maya/MObjectHandle.h>

// A vertex attribute on the shader
// This describes both the cgfx varying parameter and where the data
// for that parameter is coming from in Maya
//
class cgfxVertexAttribute
{
public:
	cgfxVertexAttribute();

	void addRef() const { ++refcount; }
 	void release() const;

	// What is the CG varying parameter?
	MString			fName;
	MString			fSemantic;
	MString			fUIName;
	MString			fType;

	// Where is the data coming from in Maya?
	MString			fSourceName;
	enum SourceType
	{
		kNone,
		kPosition,
		kNormal,
		kUV,
		kTangent,
		kBinormal,
		kColor,
		kBlindData,
		kUnknown
	}				fSourceType;
	int				fSourceIndex;

	// The next vertex attribute in the list
	cgfxRCPtr<cgfxVertexAttribute> fNext;

private:
	mutable int refcount;    // The ref count
};

//
// source streams that the cgfx vertex attribute can be bound to 
//
struct sourceStreamInfo
{
	cgfxVertexAttribute::SourceType fSourceType;
	MString fSourceName; // Use this instead of source index
	unsigned int fOffset;
	unsigned int fStride;
	unsigned int fDimension;
	unsigned int fElementSize;  //size in bytes of each element in the stream
	GLuint fDataBufferId;
};

//
// A vertex attribute structure (e.g. pack uvSet1 and uvSet2 into a single float4)
//
#define MAX_STRUCTURE_ELEMENTS 16
class cgfxVaryingParameterStructure
{
public:
	// How many elements are in this structure
	int				fLength;

	// How many bytes are in the structure
	int				fSize;

	struct cgfxVaryingParameterElement
	{
		cgfxRCPtr<cgfxVertexAttribute> fVertexAttribute;	// Which vertex attribute controls this parameter?
		int					fSize;
	};

	// Our list of elements
	cgfxVaryingParameterElement fElements[ MAX_STRUCTURE_ELEMENTS];
};


//
// A trivial data cache for feeding packed structures
//
class cgfxStructureCache
{
public:
    cgfxStructureCache();
    ~cgfxStructureCache();

    // Find an entry in the cache. Return the allocated data block, or
    // null if the entry is not found.
    char* findEntry(const MDagPath& shape, const MString& name);

    // Add an entry in the cache. Return the allocated data block.
    char* addEntry(const MDagPath& shape, const MString& name, int stride, int count);

	void flush();
	void flush(const MDagPath& shape);

private:

    // Prohibited and not implemented.
	cgfxStructureCache(const cgfxStructureCache&);
	const cgfxStructureCache& operator=(const cgfxStructureCache&);
    
private:

    struct Entry
    {
        Entry(const MDagPath& shape, const MString& name, int stride, int count);
        ~Entry();
        
        MObjectHandle   fShape;
        MString			fName;
        char*		    fData;
        Entry*          fNext;
    };

    Entry* fEntries;
};

//
// A varying parameter to a pass
//
class cgfxVaryingParameter
{
private:

    // Only cgfxPass is allowed to create and destroy
    // cgfxVaryingParameter's.
    friend class cgfxPass;
    
    static inline void addRecursive(
        CGparameter parameter,
        cgfxVaryingParameter**& nextParameter
    );
    
    cgfxVaryingParameter( CGparameter parameter);
    ~cgfxVaryingParameter();

    // Prohibited and not implemented.
	cgfxVaryingParameter(const cgfxVaryingParameter&);
	const cgfxVaryingParameter& operator=(const cgfxVaryingParameter&);

	void setupAttributes(
        cgfxRCPtr<cgfxVertexAttribute>& vertexAttributes,
        CGprogram program
    );
    
	cgfxRCPtr<cgfxVertexAttribute> setupAttribute(
        MString                 name,
        const MString&          semantic,
        CGparameter             parameter,
        cgfxRCPtr<cgfxVertexAttribute>&   vertexAttributes
    );
    
	void			null() const;

public:
	void			bind( const MDagPath& shape, cgfxStructureCache* cache,
						int vertexCount, const float * vertexArray,
						int normalsPerVertex, int normalCount, const float ** normalArrays,
						int colorCount, const float ** colorArrays,
						int texCoordCount, const float ** texCoordArrays) const;
	bool			bind( const float* data, int stride) const;

	//viewport 2.0 implementation
	bool bind(const sourceStreamInfo& source) const;

private:
    // The cg parameter
	CGparameter		                fParameter;

    // The name of the parameter
    MString			                fName;

    // GL parameter type and index (e.g. TEXCOORD*7*) used to bind this parameter.
	int				                fGLType;
	int				                fGLIndex;			
    
    // Which vertex attribute controls this parameter? (not used if a structure is present)
	cgfxRCPtr<cgfxVertexAttribute>            fVertexAttribute;	

    // The structure of elements feeding this parameter (not used if an attribute is present)
	cgfxVaryingParameterStructure*  fVertexStructure;
    
    // The next parameter in this pass
    cgfxVaryingParameter*           fNext;
};


//
// A pass in a technique
//
class cgfxPass
{
private:

    // Only cgfxTechnique is allowed to create and destroy
    // cgfxPass's.
    friend class cgfxTechnique;

	cgfxPass(CGpass                  pass,
             const cgfxProfile*      profile);
	~cgfxPass();

    // Prohibited and not implemented.
	cgfxPass(const cgfxPass&);
	const cgfxPass& operator=(const cgfxPass&);

	void setupAttributes( cgfxRCPtr<cgfxVertexAttribute>& vertexAttributes) const;

    // Specify the Cg profile to use when compiling the shader. If
    // profile is NULL, the default Cg profile is used, i.e. the
    // profile specified in the .cgfx file.
    void setProfile(const cgfxProfile* profile) const;
    
public:

    void setCgState() const { cgSetPassState(fPass); }
    void resetCgState() const { cgResetPassState(fPass); }
    void updateCgParameters() const { cgUpdatePassParameters(fPass); }

    CGpass getCgPass() const { return fPass; }
    
    const cgfxPass* getNext() const { return fNext; }
    
	void bind(
        const MDagPath& shape, cgfxStructureCache* cache,
        int vertexCount, const float * vertexArray,
        int normalsPerVertex, int normalCount, const float ** normalArrays,
        int colorCount, const float ** colorArrays,
        int texCoordCount, const float ** texCoordArrays) const;

	//viewport 2.0 implementation
	void bind(const sourceStreamInfo dataSources[], const int sourceCount) const;
    
private:
    
	CGpass			fPass;				// The cg pass
	CGprogram		fProgram;			// The vertex program
	MString			fName;				// The name of the pass
	cgfxVaryingParameter* fParameters;	// The list of parameters in this pass

    cgfxProfile     fDefaultProfile;    // The default profile of this pass.
    
    cgfxPass*		fNext;				// The next pass in this technique
};


//
// A technique in an effect
//
class cgfxTechnique
{
private:
    // Only cgfxEffect is allowed to create and destroy
    // cgfxTechnique's.
    friend class cgfxEffect;

    cgfxTechnique( CGtechnique technique, const cgfxProfile* profile);
    ~cgfxTechnique();

    // Prohibited and not implemented.
	cgfxTechnique(const cgfxTechnique&);
	const cgfxTechnique& operator=(const cgfxTechnique&);

    // Specify the Cg profile to use when compiling the shader. If
    // profile is NULL, the default Cg profile is used, i.e. the
    // profile specified in the .cgfx file.
    void setProfile(const cgfxProfile* profile) const;

    void validate() const;
    
    const cgfxProfile* getSupportedProfile(const cgfxProfile* profile) const;
    
    static bool hasBlending(CGtechnique technique);
    
public:

    MString getName() const { return fName; }
    bool isValid() const { return fValid; }
    MString getCompilationErrors() const { return fErrorString; }
    bool hasBlending() const { return fHasBlending; }

    const cgfxPass* getFirstPass() const { return fPasses; }
    const cgfxTechnique* getNext() const { return fNext; }
    int getNumPasses() const { return fNumPasses; }

    // The caller is the owner of the returned list!
    cgfxRCPtr<cgfxVertexAttribute> getVertexAttributes() const;

private:
	MString			        fName;				// The name of this technique
	CGtechnique		        fTechnique;			// The cg technique
    mutable bool            fValid;             // The cg technique is valid for the current profile
    mutable MString         fErrorString;       // Compilation errors for this technique
    bool                    fHasBlending;       // The technique uses blending
	cgfxPass*		        fPasses;			// The list of passes in this technique
	int			            fNumPasses;			// Number of passes
	cgfxTechnique*	        fNext;				// The next technique in the effect
};


//
// An effect
//
class cgfxEffect
{
public:
	static cgfxRCPtr<const cgfxEffect> loadEffect(const MString& fileName, const cgfxProfile* profile);

    bool isValid() const { return fEffect != NULL && fTechniques != 0; }
        
    const cgfxTechnique* getFirstTechnique() const { return fTechniques; }
    
    const cgfxTechnique* getTechnique(MString techniqueName) const;

    cgfxRCPtr<cgfxAttrDefList> attrsFromEffect() const;
        
    // Specify the Cg profile to use when compiling the shader. If
    // profile is NULL, the default Cg profile is used, i.e. the
    // profile specified in the .cgfx file.
    void setProfile(const cgfxProfile* profile) const;

private:
    
    friend class cgfxRCPtr<const cgfxEffect>;

    // Create a CGeffect with the proper Cg compilation flags.
    cgfxEffect(const MString& fileName, const cgfxProfile* profile);

    ~cgfxEffect();

	void addRef() const { ++refcount; }
 	void release() const;

    // Prohibited and not implemented.
	cgfxEffect(const cgfxEffect&);
	const cgfxEffect& operator=(const cgfxEffect&);

    mutable int                       refcount;    // The ref count
    
	CGeffect		                  fEffect;		// The cg effect
	const cgfxTechnique*	          fTechniques;	// The list of techniques in this effect
	mutable const cgfxProfile*        fProfile;		// The current profile
};


#undef VALID
#endif /* _cgfxAttrDef_h_ */
