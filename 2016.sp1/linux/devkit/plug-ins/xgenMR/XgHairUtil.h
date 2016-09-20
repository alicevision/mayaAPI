// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.

#ifndef __XGENHAIRUTIL_H__
#define __XGENHAIRUTIL_H__

#include <vector>
#include <shader.h>

namespace XGenMR
{

// All user data descriptions
// This is used at shader execution to query the index of a particular user data.
struct UserDataFormat
{
public:

	// Single User data definition
	struct Entry
	{
		Entry();
		Entry( const std::string& str, unsigned int offset, unsigned int numScalars );

		// Checks if name and size are valid.
		bool isValid() const;

		// We'll persist all the hair layout information in a custom user data, on the hair object
		void toString( std::string& out_str ) const;

		std::string m_name;
		unsigned int m_offset;
		unsigned int m_numScalars;
	};

	UserDataFormat();
	UserDataFormat( const char* str, size_t size );

	enum EType
	{
		eInvalid,
		eTexList,
		eInplace,
	};
	EType getType() const { return m_type; }
	void setType( EType t ) { m_type = t; }

	// We'll persist all the hair layout information in a custom user data, on the hair object
	void toString( std::string& out_str ) const;

	// register a user data to the list, the map is filled later on by buildMap()
	void registerUserData( const std::string& str, unsigned int numScalars, int perPoint );

	// build the map from the list of registered user data.
	// It builds the offsets correctly
	void buildMap();

	// Add a user data to the map. The offset must be known.
	bool addUserData( const std::string& str, unsigned int offset, unsigned int numScalars );

	Entry* find( const std::string& str );

	Entry* getEntry( size_t i );
	const Entry* getEntry( size_t i ) const;

	size_t getEntryCount() const;

	size_t getOffsetScalars( ) const { return m_offsetScalars;}

private:
	EType m_type;
	std::map<std::string,Entry> m_map;
	std::vector<Entry> m_list; // temp list used by register and build map.
	size_t m_offsetScalars; // inplace user data scalars
};

// hair util shader class. 
// Will deprecate it soon. The seexpr shader does much more and better. 
class HairUtil
{
public:
	// mental ray input parameters
	struct Params
	{
		miTag format;
	};

	void init( miState* state, Params* paras );

	// calls getField for the 3 components, based on the what init() picked.
	miBoolean execute( miVector* result, miState* state, Params* paras );

	static const char* Names[43];

	// What data to retreive on the miState
	enum Field
	{
		no_field, bary, point, normal, normal_geom, direction, tex,
		derivs0, derivs1, derivs2, derivs3,	derivs4, derivs5, derivs6, derivs7, derivs8, derivs9, 
		scalars0, scalars1, scalars2, scalars3, scalars4, scalars5, scalars6, scalars7, scalars8, scalars9, 
		hair_min_raster_size, hair_raster_area,
		hair_min_pixel_size, hair_pixel_area,
		tex_list0, tex_list1, tex_list2, tex_list3, tex_list4, tex_list5, tex_list6, tex_list7, tex_list8, tex_list9, 
	};

	// A component of the Vector3
	enum Comp
	{
		no_comp, x, y, z
	};

	// Return the data from the miState
	static float getField( miState* state, Field field, Comp component );

private:
	// Texture Helper
	static bool getTextures( miState *state, int& max_tex, miVector** out_textures );
	static bool getHairScalars( miState *state, int& max_scalars, std::vector<miScalar>& out_scalars );

	

	// Data for the shader instance, initialized in init()
	std::string m_format;
	Field m_fields[3];
	Comp m_components[3];
};

}
#endif
