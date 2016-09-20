// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.

#ifndef __XGENHELPERS_H__
#define __XGENHELPERS_H__

#include <vector>
#include <string>
#include <shader.h>

namespace XGenMR
{
	// A simple string tokenizer
	void split(std::vector<std::string>& out_vec, const std::string& in_str, const std::string& in_delimiters );

	// Get a string value out of a miTag
	void eval_string(miState* state, miTag& io_tag, std::string& out_str);

	// Get tokens from user/override strings
	void tokenize(const std::string& s, std::vector<std::string>& tokens);
}
#endif
