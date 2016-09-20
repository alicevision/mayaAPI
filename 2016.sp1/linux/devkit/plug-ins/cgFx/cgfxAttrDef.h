//
// Copyright (C) 2002-2003 NVIDIA 
// 
// File: cgfxAttrDef.h
//
// Utilities for dealing with CgFX shaders
//
// Author: Jim Atkinson
//
// Changes:
//  12/2003  Kurt Harriman - www.octopusgraphics.com +1-415-893-1023
//           - Added cgfxAttrDef members:  fNumericSoftMin, fNumericSoftMax,
//             fSemantic, fTweaked, fInitOnUndo, typeName(),
//             compoundAttrSuffixes(), attrFromNode(), initializeAttributes(),
//             isInitialValueEqual(), setInitialValue(), purgeMObjectCache(),
//             validateMObjectCache(), getExtraAttrSuffix()
//           - Deleted members: fMustInit, setDefaultValues()
//           - Changed members: fTextureId
//           - Added cgfxAttrDefList member: findInsensitive()
//           - Use MDGModifier instead of MDagModifier
//           - Minimizing unnecessary #includes so other Maya tools can
//             borrow this header without pulling in extra dependencies.
//
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
#ifndef _cgfxAttrDef_h_
#define _cgfxAttrDef_h_

// This class holds the definition of a single attribute
// as it has been extracted from the CGeffect interface.
// All the data has been stored in native Maya format for
// simplicity of use.
//

#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MMatrix.h>
#include <maya/MImage.h>
#include <maya/MMessage.h>
#include <maya/MDistance.h>

#include "cgfxRCPtr.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

class  cgfxAttrDefList;                // below
class  cgfxEffect;                     // in "cgfxEffectDef.h"
class  cgfxShaderNode;                 // in "cgfxShaderNode.h"
class  cgfxTextureCacheEntry;
class  MDGModifier;                    // in <maya/MDGModifier.h>
class  MFnDependencyNode;              // in <maya/MFnDependencyNode.h>
#define kNullCallback 0

//--------------------------------------------------------------------//
//                            cgfxAttrDef                             //
//--------------------------------------------------------------------//
// Hold the definition of an attribute.

class cgfxAttrDef
{
public:
	enum cgfxAttrType
	{
		// Unknown.
		//
		kAttrTypeUnknown,

		// Boolean value.  Create with MFnNumericAttribute; set default
		// value
		//
		kAttrTypeBool,

		// Integer value.  Create with MFnNumericAttribute; set min,
		// max, default value
		//
		kAttrTypeInt,

		// Float value.  Create with MFnNumericAttribute; set min, max,
		// default value
		//
		kAttrTypeFloat,

		// String value.  Create with MFnTypedAttribute; set default
		// value
		//
		kAttrTypeString,

		// 2, 3, and 4 element vectors. Create with MFnNumericAttribute;
		// children are *X, *Y, *Z, and (maybe) *W; set min, max,
		// default for each child.
		//
		kAttrTypeVector2,
		kAttrTypeVector3,
		kAttrTypeVector4,

		kAttrTypeObjectDir,		// Object coordinates
		kAttrTypeFirstDir = kAttrTypeObjectDir,
		kAttrTypeWorldDir,		// World coordinates
		kAttrTypeViewDir,		// Eye coordinates
		kAttrTypeProjectionDir,	// Clip coordinates
		kAttrTypeScreenDir,		// Screen coordinates
		kAttrTypeLastDir = kAttrTypeScreenDir,

		kAttrTypeObjectPos,		// Object coordinates
		kAttrTypeFirstPos = kAttrTypeObjectPos,
		kAttrTypeWorldPos,		// World coordinates
		kAttrTypeViewPos,		// Eye coordinates
		kAttrTypeProjectionPos,	// Clip coordinates
		kAttrTypeScreenPos,		// Screen coordinates
		kAttrTypeLastPos = kAttrTypeScreenPos,

		// Color value.  Create with MFnCompoundAttribute; children are
		// *R, *G, *B, and (maybe) *A; set min, max, default for each
		// child.
		//
		kAttrTypeColor3,
		kAttrTypeColor4,

		// Matrix value.  Create with MFnMatrixAttribute; default is
		// identity.
		//
		kAttrTypeMatrix,
		kAttrTypeFirstMatrix = kAttrTypeMatrix,
		kAttrTypeWorldMatrix,
		kAttrTypeViewMatrix,
		kAttrTypeProjectionMatrix,
		kAttrTypeWorldViewMatrix,
		kAttrTypeWorldViewProjectionMatrix,
		kAttrTypeLastMatrix = kAttrTypeWorldViewProjectionMatrix,

		// Texture types.  Create as a color connected to a new file
		// texture node.
		//
		kAttrTypeColor1DTexture,
		kAttrTypeFirstTexture = kAttrTypeColor1DTexture,
		kAttrTypeColor2DTexture,
		kAttrTypeColor3DTexture,
		kAttrTypeColor2DRectTexture,
		kAttrTypeNormalTexture,
		kAttrTypeBumpTexture,
		kAttrTypeCubeTexture,
		kAttrTypeEnvTexture,
		kAttrTypeNormalizationTexture,
		kAttrTypeLastTexture = kAttrTypeNormalizationTexture,

#ifdef _WIN32
		// Time
		kAttrTypeTime,
#endif

		// Hardware Fog Staff
		kAttrTypeHardwareFogEnabled,
		kAttrTypeHardwareFogMode,
		kAttrTypeHardwareFogStart,
		kAttrTypeHardwareFogEnd,
		kAttrTypeHardwareFogDensity,
		kAttrTypeHardwareFogColor,

		// Other value.  This is for attributes that have odd type or
		// dimensionality.  Create them as multi, multi, multi
		// attributes as needed; default is zero.
		//
		kAttrTypeOther,

		// Last value.
		_kAttrTypeLast
	};

	// Static methods to return useful info about a cgfxAttrType.
	static const char*  typeName( cgfxAttrType eAttrType );
	static const char** compoundAttrSuffixes( cgfxAttrType eAttrType );

	// These are hints as to what might be connected to a direction or
	// position input.
	//
	enum cgfxVectorHint
	{
		kVectorHintNone,
		kVectorHintDirLight,
		kVectorHintPointLight,
		kVectorHintSpotLight,
		kVectorHintEye,

		_kVectorHintLast
	};

	//----------------------------------------------------------------//

	MString			fName;
	cgfxAttrType	fType;
	int				fSize;			// The number of elements, not bytes.
	MObject			fAttr;			// The attribute itself
	cgfxVectorHint	fHint;

	// Vector4 or Color4 types use an extra Maya attribute to hold W or Alpha.
	MObject			fAttr2;			// An extra attribute if necessary.

	double*			fNumericMin;	// we use doubles even for int data.
	double*			fNumericMax;
	double*         fNumericSoftMin;
	double*         fNumericSoftMax;
	double*         fNumericDef;    // Numeric initial value
	MDistance::Unit	fUnits;

	MString         fStringDef;     // String initial value

	MString         fDescription;   // Description (if supplied)
	MString         fSemantic;      // Semantic (if supplied)

	CGparameter     fParameterHandle;

    cgfxRCPtr<cgfxTextureCacheEntry> fTexture;
	MCallbackId		fTextureMonitor;
	MString			fTextureUVLink;	// A string value for texture UV link.

	bool			fInvertMatrix;	// Matrix elements should be inverted.
	bool			fTransposeMatrix;//Matrix elements should be transposed.

	bool            fTweaked;       // true => user has changed attr value
	bool            fInitOnUndo;    // true => set attr to initial value when
	//   changing back to this effect on undo

private:
	bool			fIsConvertedToInternal;
	// the symbol to mark if the unit is converted to internal one
	const static char fSymbol;

	//----------------------------------------------------------------//

public:
	// Constructor
	cgfxAttrDef(CGparameter cgParameter);
    cgfxAttrDef(
        const MString&          sAttrName,
        const cgfxAttrType      eAttrType,
        const MString&          sDescription,
        const MString&          sSemantic,
        MObject                 obNode,
        MObject                 obAttr
    );

	// Destructor
	~cgfxAttrDef();

private:
	// Prohibited and not implemented.
	cgfxAttrDef(const cgfxAttrDef&);
	const cgfxAttrDef& operator=(const cgfxAttrDef&);

public:
	// Release any associated resources
	void			release();
	void			releaseTexture();
	void			releaseCallback();

	// Return a string representation of fType.
	MString         typeName() const { return typeName( fType ); }

	static cgfxRCPtr<cgfxAttrDefList> attrsFromNode(MObject& node);
    
	static void     updateNode(
        const cgfxRCPtr<const cgfxEffect>& effect,
		cgfxShaderNode*                     pNode, 
		MDGModifier*                        dgMod,
        cgfxRCPtr<cgfxAttrDefList>&         attrDefList,
		MStringArray&                       attributeList);
	static void buildAttrDefList(MObject& node);
	static void     initializeAttributes(
        MObject&                            node, 
        const cgfxRCPtr<cgfxAttrDefList>&   list,
		bool                                bUndoing,
		MDGModifier*	                    dgMod );
	static void     purgeMObjectCache(const cgfxRCPtr<cgfxAttrDefList>& list);
	static void     validateMObjectCache(const MObject&   obCgfxShader, 
                                         const cgfxRCPtr<cgfxAttrDefList>& list);
    
	static cgfxAttrDef* attrFromNode(
		const MFnDependencyNode& fnNode,
		const MString&           sAttrName,
		const cgfxAttrType       eAttrType,
		const MString&           sDescription,
		const MString&           sSemantic);

	// Return suffix for Color4/Vector4 extra attribute, or NULL.
	const char*     getExtraAttrSuffix() const;

	bool            createAttribute(const MObject& oNode, MDGModifier* mod, cgfxShaderNode*   pNode);
	bool            destroyAttribute(MObject& oNode, MDGModifier* mod);

protected:

	// setTextureType
	//
	// Description:
	//
	// Parameters:
	//      param - parameter
	//
	// Returns:
	//      None
	//
	void setTextureType(CGparameter param);

	void setSamplerType(CGparameter param);


	void setMatrixType(CGparameter param);

	// setVectorType
	//
	// Description:
	//  
	// Parameters:
	//      param - parameter
	//
	// Returns:
	//      None
	//
	void 

		setVectorType(CGparameter param);

	// Return true if initial value of 'this' is same as 'that'.
	bool            isInitialValueEqual( const cgfxAttrDef& that ) const; 

	// Copy initial value from given attribute.
	void            setInitialValue( const cgfxAttrDef& from );

    // Set attribute flag
	void setAttributeFlags();

public:
	// These routines all get the value of the attribute referenced by 
	// this cgfxAttrDef object.
	//
	void            getValue( MObject& oNode, bool& value ) const;
	void            getValue( MObject& oNode, int& value ) const;
	void            getValue( MObject& oNode, float& value ) const;
	void            getValue( MObject& oNode, MString& value ) const;
	void            getValue( MObject& oNode, float& v1, float& v2 ) const;
	void            getValue( MObject& oNode,
		float& v1, float& v2, float& v3 ) const;
	void            getValue( MObject& oNode,
		float& v1, float& v2, float& v3, float& v4 ) const;
	void            getValue( MObject& oNode, MMatrix& value ) const;
	void            getValue( MObject& oNode, MImage& value ) const;

	// This routine finds the DG input to this attribute (e.g. texture
	// node connections)
	//
	void            getSource( MObject& oNode, MPlug& src) const;


	// These routines all set the value of the attribute referenced by 
	// this cgfxAttrDef object.
	//
	void            setValue( MObject& oNode, bool value );
	void            setValue( MObject& oNode, int value );
	void            setValue( MObject& oNode, float value );
	void            setValue( MObject& oNode, const MString& value );
	void            setValue( MObject& oNode, float v1, float v2 );
	void            setValue( MObject& oNode, float v1, float v2, float v3 );
	void            setValue( MObject& oNode,
		float v1, float v2, float v3, float v4 );
	void            setValue( MObject& oNode, const MMatrix& v );
	void            setTexture( MObject& oNode, const MString& value, MDGModifier* dgMod);

	void			setUnitsToInternal(CGparameter& cgParameter);
};

#define VALID(ptr) (ptr == 0 || _CrtIsValidHeapPointer(ptr))


//--------------------------------------------------------------------//
//                          cgfxAttrDefList                           //
//--------------------------------------------------------------------//
// Hold a list of cgfxAttrDef* objects that can be searched by name

class cgfxAttrDefList
{
protected:
	class element;
	friend class element;

public:
	class iterator;
	friend class iterator;

protected:
	class element
	{
	public:
		element*		prev;
		element*		next;
		cgfxAttrDef*	data;

		element()
			: prev(0)
			, next(0)
			, data(0)
		{ /* nothing */ };

		~element()
		{
#ifdef _WIN32
			_ASSERTE(VALID(prev));
			_ASSERTE(VALID(next));
			_ASSERTE(VALID(data));
#endif
			if (prev)
			{
				prev->next = next;
			}
			if (next)
			{
				next->prev = prev;
			}
			delete data;
		};
	};

	element* head;
	element* tail;
	int refcount;

public:
	class iterator
	{
		friend class cgfxAttrDefList;

	protected:
		element*		item;

	public:
#ifdef _WIN32
		cgfxAttrDef* operator *() { _ASSERTE(VALID(item)); _ASSERTE(VALID(item->data)); return item->data; };
		iterator& operator ++ () { _ASSERTE(VALID(item)); _ASSERTE(VALID(item->next)); item = item->next; return *this; };
#else
		cgfxAttrDef* operator *() { return item->data; };
		iterator& operator ++  () { item = item->next; return *this; };
#endif

		iterator operator ++ (int)
		{
			iterator tmp = *this;
#ifdef _WIN32
			_ASSERTE(VALID(item));
			_ASSERTE(VALID(item->next)); 
#endif
			item = item->next;
			return tmp;
		};

		operator bool() { return (item != 0); };

		iterator(): item(0) { /* nothing */ };
		iterator(cgfxAttrDefList& list): item(list.head)
	   	{
#ifdef _WIN32
			   	_ASSERTE(VALID(item));
#endif
	   	};
		iterator(const cgfxRCPtr<cgfxAttrDefList>& list)
		{
#ifdef _WIN32
			_ASSERTE(VALID(list.operator->()));
			item = list.isNull() ? 0 : list->head;
			_ASSERTE(VALID(item));
#else
			item = list.isNull() ? 0 : list->head;
#endif
		};
		void reset() { item = 0; };

	protected:
		iterator(element* e): item(e) { /* nothing */ };
	};

	cgfxAttrDefList()
		:	head(0)
		,	tail(0)
		,	refcount(0)
	{ /* nothing */ };

private:

    friend class cgfxRCPtr<cgfxAttrDefList>;
    
	~cgfxAttrDefList()
	{
		clear();
	};

	void addRef()
	{
		++refcount;
	};

	void release();

    // Prohibited and not implemented.
	cgfxAttrDefList(const cgfxAttrDefList&);
	const cgfxAttrDefList& operator=(const cgfxAttrDefList&);

public:

    void releaseTextures();
    
    void clear()
	{
#ifdef _WIN32
		_ASSERTE(VALID(head));
#endif
		if (head)
		{
			element *tmp = new element;
			tmp->next = head;
			head->prev = tmp;

			while (tmp->next)
			{
				delete tmp->next;
			}
			delete tmp;
		}

		head = tail = 0;
	};

	bool empty() { return (head == 0); };

	iterator find(const MString& name)
	{
		iterator it(*this);

		while (it)
		{
			if ((*it)->fName == name)
			{
				break;
			}
			++it;
		}

		return it;
	};

	iterator        findInsensitive( const MString& name );

	void add(cgfxAttrDef* aDef)
	{
		element * e = new element;
		e->data = aDef;
		if (tail)
		{
			tail->next = e;
			e->prev = tail;
		}
		else
		{
			head = e;
		}
		tail = e;
	}

	iterator begin()
	{
		return iterator(*this);
	};

    void dump(const char* name);
};

#undef VALID
#endif /* _cgfxAttrDef_h_ */
