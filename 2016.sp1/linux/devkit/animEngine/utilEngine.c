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

#include <utilEngine.h>

/*
//	Function Name:
//		engineUtilStringsMatch
//
//	Description:
//		A helper function to compare two strings
//
//  Input Arguments:
//		EtByte *string1				The first string to compare
//		EtByte *string2				The second string to compare
//
//  Return Value:
//      EtBoolean result			A handle for the opened file
//			kEngineTRUE					the two strings match
//			kEngineFALSE				the two strings do not match
*/
EtBoolean
engineUtilStringsMatch (EtByte *string1, EtByte *string2)
{
	/* make sure both strings are valid */
	if ((string1 == kEngineNULL) || (string2 == kEngineNULL)) {
		return (kEngineFALSE);
	}
	/* just in case both pointers are the same */
	if (string1 == string2) {
		return (kEngineTRUE);
	}
	/* compare the two strings */
	return (strcmp ((const char *)string1, (const char *)string2) == 0);
}

/*
//	Function Name:
//		engineUtilCopyString
//
//	Description:
//		A helper function to copy one string to another
//
//  Input Arguments:
//		EtByte *src					The string to copy
//		EtByte *dest				The string destination
//
//  Return Value:
//		None
*/
EtVoid
engineUtilCopyString (EtByte *src, EtByte *dest)
{
	/* make sure both strings are valid */
	if ((src == kEngineNULL) || (dest == kEngineNULL)) {
		return;
	}
	/* copy the string */
	strcpy ((char *)dest, (const char *)src);
}

/*
//	Function Name:
//		engineUtilAllocate
//
//	Description:
//		A helper function to allocate a block of memory
//
//  Input Arguments:
//		EtInt bytes					The number of bytes to allocate
//
//  Return Value:
//		EtByte *block				A pointer to the requested memory block
*/
EtByte *
engineUtilAllocate (EtInt bytes)
{
	return (malloc (bytes));
}

/*
//	Function Name:
//		engineUtilFree
//
//	Description:
//		A helper function to free a block of memory allocated using
//	engineUtilAllocate
//
//  Input Arguments:
//		EtByte *block				A pointer to the memory block to free
//
//  Return Value:
//		None
*/
EtVoid
engineUtilFree (EtByte *block)
{
	if (block != kEngineNULL) {
		free (block);
	}
}
