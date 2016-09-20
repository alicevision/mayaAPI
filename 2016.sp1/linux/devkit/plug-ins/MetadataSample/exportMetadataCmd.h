#ifndef _exportMetadataCmd_h_
#define _exportMetadataCmd_h_

#include "metadataBase.h"

//======================================================================
//
// Export a data stream out to a file
//
class exportMetadataCmd : public metadataBase
{
public:
	static MSyntax	  cmdSyntax ();
	static void*		creator	();
	static const char*	   name	();

			exportMetadataCmd	();
	virtual	~exportMetadataCmd	();

protected:
	virtual MStatus	checkArgs	( MArgDatabase& argDb );
	virtual MStatus doCreate	();
	virtual MStatus doQuery		();
};

//-
// ==================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
// 
// This computer source code  and related  instructions and comments are
// the unpublished confidential and proprietary information of Autodesk,
// Inc. and are  protected  under applicable  copyright and trade secret
// law. They may not  be disclosed to, copied or used by any third party
// without the prior written consent of Autodesk, Inc.
// ==================================================================
//+
#endif // _exportMetadataCmd_h_
