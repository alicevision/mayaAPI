#ifndef _adskRenderGlobal_h_
#define _adskRenderGlobal_h_

///////////////////////////////////////////////////////////////////////////////
//
// AdskPrepareRenderGlobals
//
// Assembly render node settings
//
////////////////////////////////////////////////////////////////////////////////

#include <maya/MPxNode.h>

class AdskPrepareRenderGlobals : public MPxNode
{
public:

   /*----- static member functions -----*/

   static void* creator();
   static MStatus initialize();
   static MStatus uninitialize();   
   static const MTypeId id;
   static const MString typeName;

   /*----- member functions -----*/
      
   AdskPrepareRenderGlobals();
   ~AdskPrepareRenderGlobals();

private:

   // Return the name of the default icon for the node.
   virtual MString      getDefaultIcon() const; 

   /*----- member functions -----*/
      
   // Prohibited and unimplemented.
   AdskPrepareRenderGlobals(const AdskPrepareRenderGlobals& obj);
   const AdskPrepareRenderGlobals& operator=(const AdskPrepareRenderGlobals& obj);
   
   /*----- static data members -----*/
    
   static MObject aRepName;   
   static MObject aRepLabel;  
   static MObject aRepType;  
   static MObject aUseRegEx;
 
   };

#endif

//-
//*****************************************************************************
// Copyright 2013 Autodesk, Inc. All rights reserved.
// 
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy
// form.
//*****************************************************************************
//+
