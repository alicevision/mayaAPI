////////////////////////////////////////////////////////////////////////
//
// skinClusterWeights.cpp
//
// see examples.mel in the same directory for examples
//
// skinClusterWeights -inf {"obj1", "obj2"} -sc {"skinCluster1", "skinCluster2"} {"myShape"};
// skinClusterWeights -edit -w {1, 2, 3, 4} "myShape.vtx[33]";
//
// The generic syntax is 
// skinClusterWeights -q/-edit -inf/influences $influenceArray -sc/skinClusters $skinClusterArray -w/weights $weightFloatArray $objectStringArray;
//
////////////////////////////////////////////////////////////////////////

#include <maya/MPxCommand.h>
#include <maya/MArgList.h>
#include <maya/MArgParser.h>

#include <maya/MDagPath.h>
#include <maya/MItSelectionList.h>
#include <maya/MSelectionList.h>

#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlugArray.h>
#include <maya/MPlug.h>
#include <maya/MItDependencyGraph.h>

#include <maya/MString.h>
#include <maya/MObjectArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MDagPathArray.h>

#include <maya/MGlobal.h>

#include <maya/MFnSkinCluster.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItSurfaceCV.h>
#include <maya/MItCurveCV.h>

#include <math.h>

#define kEditFlag                   "-e"
#define kEditFlagLong               "-edit"
#define kQueryFlag                  "-q"
#define kQueryFlagLong              "-query"
#define kInfluenceFlag	            "-inf"
#define kInfluenceFlagLong          "-influences"
#define kSkinClusterFlag            "-sc"
#define kSkinClusterFlagLong        "-skinClusters"
#define kWeightFlag                 "-w"
#define kWeightFlagLong             "-weights"
#define kAssignAllToSingleFlag      "-as"
#define kAssignAllToSingleFlagLong  "-assignAllToSingle"

///////////////////////////////////////////////////
//
// Command class declaration
//
///////////////////////////////////////////////////
class skinClusterWeights : public MPxCommand
{
public:
                         skinClusterWeights();
        virtual		~skinClusterWeights(); 

        virtual MStatus	doIt(const MArgList& args);
        virtual MStatus	undoIt();
        virtual MStatus	redoIt();
        virtual bool    isUndoable() const {return true;}

	static void*	creator();

private:

	void		doItEdit();
	void		doItQuery();
	bool		editUsed;
	bool		queryUsed;

        // Return the skinCluster given a geometry
	MObject		findSkinCluster(MDagPath&);

        void            populateInfluenceIndexArray(MFnSkinCluster &skinClusterFn, MIntArray &influenceIndexArray);

        // true if 1) skin cluster is not specified in the command or
        //         2) MObject is one of the skin clusters specified in the command
	bool		isSkinClusterIncluded(MObject&);

	MStatus		parseArgs( const MArgList &args );
	MDagPathArray	influenceArray;
	MObjectArray	skinClusterArray;
	MDoubleArray	weightArray;
	MStringArray	geometryArray;
	bool		assignAllToSingle;

        // data structures for undo
        MDagPathArray   fDagPathArray;
        MObjectArray    fComponentArray;
	MIntArray      *fInfluenceIndexArrayPtrArray;
	MDoubleArray   *fWeightsPtrArray;
 };

skinClusterWeights::skinClusterWeights() : assignAllToSingle(false) 
{
    fInfluenceIndexArrayPtrArray = NULL;
    fWeightsPtrArray = NULL;
}

skinClusterWeights::~skinClusterWeights() 
{
    if (fInfluenceIndexArrayPtrArray != NULL) delete [] fInfluenceIndexArrayPtrArray;
    if (fWeightsPtrArray != NULL) delete [] fWeightsPtrArray;
}

void* skinClusterWeights::creator()
{
    return new skinClusterWeights;
}

// Custom parsing to support array argument. Not using MSyntax.
MStatus skinClusterWeights::parseArgs( const MArgList& args )
{
    MStatus	    status = MS::kSuccess;
    MObject         node;
    MDagPath        dagPath;
    MSelectionList  selList;
    
    editUsed  = true;
    queryUsed = false;
    unsigned int i, nth = 0;
    unsigned int numArgs = args.length();	
    while(status == MS::kSuccess && nth < numArgs-1) {
	MString inputString = args.asString(nth, &status);

	if (status != MS::kSuccess) {
	    MGlobal::displayError("skinClusterWeights syntax error");
	    return status;
	}
      
	if (inputString == kEditFlag || inputString == kEditFlagLong) {
	    editUsed  = true;
	    queryUsed = false;
	    nth++;
	    continue;
	}
      
	if (inputString == kQueryFlag || inputString == kQueryFlagLong) {
	    queryUsed = true;
	    editUsed  = false;
	    nth++;
	    continue;
	}
      
	if (inputString == kInfluenceFlag || inputString == kInfluenceFlagLong) {	    
	    nth++;
	    MStringArray stringArray = args.asStringArray(nth, &status);
	    selList.clear();
	    for (i = 0; i < stringArray.length(); i++) {
	      selList.add(stringArray[i]);
	    }
	    for (i = 0; i < selList.length(); i++) {
	      status = selList.getDagPath(i, dagPath);
	      if (status == MS::kSuccess && dagPath.hasFn(MFn::kTransform)) {
		influenceArray.append(dagPath);
	      } else {
		MGlobal::displayError(inputString + " is not a valid influence object.\n");
		return status;
	      }
	    }
	    nth++;
	    continue;
	}

	if (inputString == kSkinClusterFlag || inputString == kSkinClusterFlagLong) {
	    nth++;
	    MStringArray stringArray = args.asStringArray(nth, &status);
	    selList.clear();
	    for (i = 0; i < stringArray.length(); i++) {
	      selList.add(stringArray[i]);
	    }
	    for (i = 0; i < selList.length(); i++) {
	      status = selList.getDependNode(i, node);
	      if (status == MS::kSuccess && node.hasFn(MFn::kSkinClusterFilter)) {
		skinClusterArray.append(node);
	      } else {
		MGlobal::displayError(inputString + " is not a valid skinCluster.\n");
		return status;
	      }
	    }
	    nth++;
	    continue;
	}

	if (inputString == kWeightFlag || inputString == kWeightFlagLong) {
	    nth++;
	    weightArray = args.asDoubleArray(nth, &status);
	    if (status != MS::kSuccess) {
		MGlobal::displayError("error while parsing weight array");
	    }
	    nth++;
	    continue;
	}

	if (inputString == kAssignAllToSingleFlag || inputString == kAssignAllToSingleFlagLong) {
	    assignAllToSingle = true;
	    nth++;
	    continue;
	}

	MGlobal::displayError("invalid command syntax at " + inputString);
	return MS::kFailure;
    }

    // parse command objects
    // nth should equals to numArgs-1 at this point
    geometryArray = args.asStringArray(nth, &status);
    if (status != MS::kSuccess) {
	MGlobal::displayError("Command object invalid");
	return status;
    }

    if (queryUsed) {
	if (assignAllToSingle) {
	    MGlobal::displayWarning("-as/-assignAllToSingle is ignored with query flag");
	}
	if (weightArray.length() > 0) {
	    MGlobal::displayWarning("-w/-weights is ignored with query flag");
	}
    }
    
    return status;
}

MStatus skinClusterWeights::doIt( const MArgList& args )
//
// Description
//
{
    MStatus status = parseArgs(args);
    if (status != MS::kSuccess) return status;

    if (editUsed) {
 	redoIt();
    } else if (queryUsed) {
       doItQuery();
    }
    return MS::kSuccess;
}

MStatus skinClusterWeights::redoIt()
{
    MStatus status;
    unsigned int ptr = 0;
	
    MSelectionList selList;
    int geomLen = geometryArray.length();
    fDagPathArray.setLength(geomLen);
    fComponentArray.setLength(geomLen);
    fInfluenceIndexArrayPtrArray = new MIntArray[geomLen];
    fWeightsPtrArray = new MDoubleArray[geomLen];

    for (int i = 0; i < geomLen; i++) {
	MDagPath dagPath;
	MObject  component;
	
	selList.clear();
	selList.add(geometryArray[i]);
	MStatus status = selList.getDagPath(0, dagPath, component);
	if (status != MS::kSuccess) {
	    continue;
	}
	if (component.isNull()) dagPath.extendToShape();

	MObject skinCluster = findSkinCluster(dagPath);
	if (!isSkinClusterIncluded(skinCluster)) {
	    continue;
	}

	MFnSkinCluster skinClusterFn(skinCluster, &status);
	if (status != MS::kSuccess) {
	    continue; 
	}

	MIntArray influenceIndexArray;
	populateInfluenceIndexArray(skinClusterFn, influenceIndexArray);

	unsigned numInf = influenceIndexArray.length();
	if (numInf == 0) continue;
	  
	unsigned numCV = 0;
	if (dagPath.node().hasFn(MFn::kMesh)) {
	    MItMeshVertex polyIter(dagPath, component, &status);
	    if (status == MS::kSuccess) {
		numCV = polyIter.count();
	    } 
	} else if (dagPath.node().hasFn(MFn::kNurbsSurface)) {
	    MItSurfaceCV nurbsIter(dagPath, component, true, &status);
	    if (status == MS::kSuccess) {
		while (!nurbsIter.isDone()) {
		    numCV++;
		    nurbsIter.next();
		}
	    }
	} else if (dagPath.node().hasFn(MFn::kNurbsCurve)) {
	    MItCurveCV curveIter(dagPath, component, &status);
	    if (status == MS::kSuccess) {
		while (!curveIter.isDone()) {
		    numCV++;
		    curveIter.next();
		}
	    }
	}
	unsigned numEntry = numCV * numInf;

	if (numEntry > 0) {
	    MDoubleArray weights(numEntry);
	    unsigned int numWeights = weightArray.length();
	    if (assignAllToSingle) {
		if (numInf <= numWeights) {
		    for (unsigned j = 0; j < numEntry; j++) {
			weights[j] = weightArray[j % numInf];
		    }
		} else {
		    MGlobal::displayError("Not enough weights specified\n");
		    return MS::kFailure;
		}
	    } else {
		for (unsigned j = 0; j < numEntry; j++, ptr++) {
		    if (ptr < numWeights) {
			weights[j] = weightArray[ptr];
		    } else {
			MGlobal::displayError("Not enough weights specified\n");
			return MS::kFailure;
		    }
		}
	    }

	    // support for undo
	    fDagPathArray[i] = dagPath;
	    fComponentArray[i] = component;
	    fInfluenceIndexArrayPtrArray[i] = influenceIndexArray;
	    MDoubleArray oldWeights;
	    skinClusterFn.getWeights(dagPath, component, influenceIndexArray, oldWeights);
	    fWeightsPtrArray[i] = oldWeights;
	    
	    skinClusterFn.setWeights(dagPath, component, influenceIndexArray, weights);
	}
    }
    return MS::kSuccess;
}

MStatus skinClusterWeights::undoIt()
{
    MStatus status;
    for (unsigned i = 0; i < fDagPathArray.length(); i++) {
        MDagPath &dagPath = fDagPathArray[i];
	MObject skinCluster = findSkinCluster(dagPath);
	if (!isSkinClusterIncluded(skinCluster)) {
	    continue;
	}

	MFnSkinCluster skinClusterFn(skinCluster, &status);
	if (status != MS::kSuccess) {
	    continue; 
	}

	MObject &component = fComponentArray[i];
	if (dagPath.isValid() && 
	    fInfluenceIndexArrayPtrArray[i].length() > 0 && fWeightsPtrArray[i].length() > 0) {

	    skinClusterFn.setWeights(dagPath, component, fInfluenceIndexArrayPtrArray[i], fWeightsPtrArray[i]);
	}
    }
    fDagPathArray.clear();
    fComponentArray.clear();
    delete [] fInfluenceIndexArrayPtrArray;
    delete [] fWeightsPtrArray;
    fInfluenceIndexArrayPtrArray = NULL;
    fWeightsPtrArray = NULL;
    return MS::kSuccess;
}

void skinClusterWeights::doItQuery()
{
    MStatus status;
    unsigned int i, j;

    MDoubleArray weights;
    // To allow "skinClusterWeights -q" to return empty double array
    setResult(weights);
    MSelectionList selList;
    for (i = 0; i < geometryArray.length(); i++) {
	MDagPath dagPath;
	MObject  component;
	  
	selList.clear();
	selList.add(geometryArray[i]);
	MStatus status = selList.getDagPath(0, dagPath, component);
	if (status != MS::kSuccess) {
	    continue;
	}
	if (component.isNull()) dagPath.extendToShape();
	  
	MObject skinCluster = findSkinCluster(dagPath);
	if (!isSkinClusterIncluded(skinCluster)) {
	    continue;
	}  

	MFnSkinCluster skinClusterFn(skinCluster, &status);
	if (status != MS::kSuccess) {
	    continue; 
	}

	MIntArray influenceIndexArray;
	populateInfluenceIndexArray(skinClusterFn, influenceIndexArray);
	  
	weights.clear();
	skinClusterFn.getWeights(dagPath, component, influenceIndexArray, weights);

	if (weights.length() > 0) {
	    for (j = 0; j < weights.length(); j++) {
		appendToResult(weights[j]);
	    }
	}
    }
}

MObject skinClusterWeights::findSkinCluster(MDagPath& dagPath)
{
    MStatus status;
    MObject skinCluster;
    MObject geomNode = dagPath.node();
    MItDependencyGraph dgIt(geomNode, MFn::kSkinClusterFilter, MItDependencyGraph::kUpstream);
    if (!dgIt.isDone()) {
	skinCluster = dgIt.currentItem();
    }
    return skinCluster;
}

bool skinClusterWeights::isSkinClusterIncluded(MObject &node)
{
    MStatus   status;
    unsigned int i;

    if (skinClusterArray.length() == 0) return true;

    for (i = 0; i < skinClusterArray.length(); i++) {
 	if (skinClusterArray[i] == node) return true;
    }

    return false;
}

void skinClusterWeights::populateInfluenceIndexArray(MFnSkinCluster &skinClusterFn, MIntArray &influenceIndexArray)
{
   MStatus status;

   MIntArray  allIndexArray;
   MDagPathArray pathArray;
   skinClusterFn.influenceObjects(pathArray, &status);
   for (unsigned j = 0; j < pathArray.length(); j++) {
	allIndexArray.append(skinClusterFn.indexForInfluenceObject(pathArray[j]));
   }

   if (influenceArray.length() > 0) {
        // Add the influence indices for the influence objects specified in the cmd
        for (unsigned j = 0; j < influenceArray.length(); j++) {
	    unsigned int index = skinClusterFn.indexForInfluenceObject(influenceArray[j], &status);
	    for (unsigned k = 0; k < allIndexArray.length(); k++) {
	        if ((int)index == allIndexArray[k]) {
		    influenceIndexArray.append(k);
		}
	    }
	}
    } else {
        // Add the influence indices for all the influence objects of the skinCluster
	for (unsigned j = 0; j < allIndexArray.length(); j++) {
	    influenceIndexArray.append(j);
	}
    }
}

/////////////////////////////////////////////////////////////////
//
// The following routines are used to register/unregister
// the command we are creating within Maya
//
/////////////////////////////////////////////////////////////////
MStatus initializePlugin( MObject obj )
{
    MStatus   status;
    MFnPlugin plugin(obj);

    status = plugin.registerCommand( "skinClusterWeights",
				     skinClusterWeights::creator);
    if (!status) {
	status.perror("registerCommand");
	return status;
    }

    return status;
}

MStatus uninitializePlugin( MObject obj)
{
    MStatus	  status;
    MFnPlugin plugin( obj );

    status = plugin.deregisterCommand( "skinClusterWeights" );
    if (!status) {
 	status.perror("deregisterCommand");
	return status;
    }
    
    return status;
}
