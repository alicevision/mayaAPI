#ifndef _MEvaluationNode
#define _MEvaluationNode
// ===========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk license
// agreement provided at the time of installation or download, or which
// otherwise accompanies this software in either electronic or hard copy form.
// 
#if defined __cplusplus

// ****************************************************************************
// INCLUDED HEADER FILES

#include <maya/MTypes.h>
#include <maya/MEvaluationNodeIterator.h>
#include <maya/MObject.h>

// ****************************************************************************
// FORWARD DECLARATIONS
class MPlug;

// ****************************************************************************
// CLASS DECLARATION (MEvaluationNode)

//! \ingroup OpenMaya
//! \sa MEvaluationNodeIterator
//! \brief Provides access to Evaluation Manager node information. 
/*!
	An evaluation node is an object that is placed within an 
	evaluation graph.  Evaluation graphs are scheduled to speed
	up the operations within Maya.  

	This class contains methods for querying which attributes or plugs on a node
	will be set dirty prior to evaluation by the evaluation manager.

	It is sometimes necessary to know the dirty
	state of plugs/attributes if your node is handling custom
	information.  This handling would be done in the MPxNode
	preEvaluation()/postEvaluation() methods.

	If you need to access the list of dirty plugs in the evaluation
	node then use the internal iterator:

		for( MEvaluationNodeIterator nodeIt = theNode.iterator();
			 ! nodeIt.isDone(); nodeIt.next() )
		{
			doSomePlugThing( nodeIt.plug() );
		}
*/
class OPENMAYA_EXPORT MEvaluationNode
{
public:
	~MEvaluationNode();

	MEvaluationNodeIterator	iterator( MStatus * ReturnStatus = NULL ) const;
	bool			dirtyPlugExists	(const MObject& attribute, MStatus * ReturnStatus = NULL ) const;
	MPlug			dirtyPlug		(const MObject& attribute, MStatus * ReturnStatus = NULL ) const;

	static const char*	className();

private:
	friend class MEvaluationNodeIterator;

	MEvaluationNode();
	MEvaluationNode( const void* );
	const void *fEvaluationNode;
};

#endif /* __cplusplus */
#endif /* _MEvaluationNode */
