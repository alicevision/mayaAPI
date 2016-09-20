// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.

#ifndef __XGENSEEXPR_H__
#define __XGENSEEXPR_H__

#include <vector>
#include <shader.h>

#include <SePlatform.h>
#include <SeExpression.h>

namespace XGenMR
{

// SeExpr shader class.
class SeExprShader
{
public:
	// Ctor / Dtor
	SeExprShader();
	~SeExprShader();

	// Struct for the 8 attr params
	struct Attr
	{
		miTag name; 
		miVector vec;
	};

	// mental ray input parameters
	struct Params
	{
		miTag expression;	// Expression string to evaluate.
		Attr attrs[8];		// Override 8 variable with texturable vector values.
		miTag customs;		// Declaration of custom variable names used in the expression.
	};

	class MRSeExpression; // Forward decl

	// This is a bit mixed up. 
	// MRSeExprVarRef holds a back pointer to MRSeExpression and the var name.
	// So this class only delegates to MRSeExpression
	class MRSeExprVarRef : public SeExprVarRef
	{
	public:
		MRSeExprVarRef(MRSeExpression* parent=NULL, const char* in_name="", bool in_isVector=true )
		{
			m_parent = parent;
			m_name = in_name;
			m_isVector = in_isVector;
		}

		virtual ~MRSeExprVarRef() {}

		//! returns true for a vector type, false for a scalar type
		virtual bool isVec();

		//! returns this variable's value by setting result, node refers to 
		//! where in the parse tree the evaluation is occurring
		virtual void eval(const SeExprVarNode* node, SeVec3d& result);

	private:
		MRSeExpression* m_parent;
		std::string m_name;
		bool m_isVector;
	};

	// Mental ray class for SeExpression
	class MRSeExpression : public SeExpression
	{
	public:
		// TLS Values
		typedef std::map<std::string, std::pair<SeVec3d,bool> > TLSValues;
		// Map of TLS Values per mental ray instance.
		typedef std::pair<miTag,MRSeExpression::TLSValues> TLSMapPair;
		typedef std::map<miTag,MRSeExpression::TLSValues> TLSMap;
		typedef std::pair<TLSMap::iterator,bool> TLSMapInsert;

		// TLS for currently evaluated values.
		MRSeExpression()
		{
			m_tls = NULL;
		}
		~MRSeExpression()
		{
			m_tls = NULL; // Don't delete.
		}

		// Called before validating the expression and again before evaluation.
		void setTLSValues(TLSValues* in_values )
		{
			m_tls = in_values;
		}

		// Called by MRSeExprVarRef to retreive the variable values. 
		const TLSValues* getTLSValues( )
		{
			return m_tls;
		}

		// Creates variable references
		void createRefs();

		/** override resolveVar to add external variables */
		virtual SeExprVarRef* resolveVar(const std::string& /*name*/) const;

		TLSValues* m_tls; // TLS pointer
		std::map<std::string,MRSeExprVarRef> m_refs;
	};
	
	// Hair format helper built from the XGMR userdata attached to XGen miInstances.
	// Holds a map of names to state->tex_list[] offsets.
	// This isn't recomputed per thread. It only locks the multi threading one time per instance. 
	// It doesn't affect the performance soo much.
	typedef std::map<miTag,UserDataFormat> UserDataFormatMap;

	// Thread Local Storage data.
	// Wraps an expression and a cache of per instance TLSValues.
	struct 	TLSData
	{
		MRSeExpression::TLSMap	m_cacheTLS;		// Cache of computed values per instance.
		MRSeExpression			m_expression;	// Per thread Expression
		UserDataFormatMap		m_fmts;			// Per thread format
	};

	// Redirection of mr entry points
	void init( miState* state, Params* paras );
	miBoolean execute( miVector* result, miState* state, Params* paras );

private:
	// data from the string parameters. 
	std::string m_strExpression;			// The expression code
	std::vector<std::string> m_strNames;	// The explicit variable names.

	
	// Functions to fill the user data formats
	static bool findInstanceUserData( miState* state, miTag in_instance, miTag& out_user );
	static bool findPlaceholderItemUserData( miState* state, miTag in_instance, miTag& out_user );
	static bool findXGMRUserData( miState* state, miTag in_user, char*& out_string, int& out_size );
	static bool findXGMRUserDataOnInstance( miState* state, miTag in_instance, char*& out_string, int& out_size );
	bool recFindUserDataFormat( miState* state, MRSeExpression::TLSValues& values, UserDataFormatMap& fmts );
	bool findUserDataFormat( miState* state, miTag tagInstance, MRSeExpression::TLSValues& values, UserDataFormatMap& fmts );
	bool recFindUserDataScalars( miState* state, const float*& scalars, UserDataFormatMap& fmts );
	bool findUserDataScalars( miState* state, miTag tagInstance, const float*& scalars, UserDataFormatMap& fmts );
	
	// The expression from the init callback
	MRSeExpression::TLSValues m_declValues;

	// Declaration of Values from  init callback
	MRSeExpression* m_pExpression; 
};

}
#endif
