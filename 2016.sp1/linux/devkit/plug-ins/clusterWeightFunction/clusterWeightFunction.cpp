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


#include <maya/MIOStream.h>
#include <math.h>
#include <maya/MSimple.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MSelectionList.h>
#include <maya/MObjectArray.h>
#include <maya/MFnWeightGeometryFilter.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MItGeometry.h>
#include <maya/MPoint.h>


#define kSineFlag								"-s"
#define kSineFlagLong							"-sine"
#define kSineDistanceFlag						"-sd"
#define kSineDistanceFlagLong					"-sineDistance"
#define kSineDistance2Flag						"-sd2"
#define kSineDistance2FlagLong					"-sineDistance2"
#define kDistanceSineDistanceFlag				"-dsd"
#define kDistanceSineDistanceFlagLong			"-distanceSineDistance"
#define kInverseDistanceSineDistanceFlag		"-ids"
#define kInverseDistanceSineDistanceFlagLong	"-inverseDistanceSineDistance"
#define kDistanceFlag							"-d"
#define kDistanceFlagLong						"-distance"
#define kDistance2Flag							"-d2"
#define kDistance2FlagLong						"-distance2"
#define kDistance3Flag							"-d3"
#define kDistance3FlagLong						"-distance3"
#define kDistance4Flag							"-d4"
#define kDistance4FlagLong						"-distance4"
#define kInverseDistanceFlag					"-id"
#define kInverseDistanceFlagLong				"-inverseDistance"
#define kInverseDistance2Flag					"-id2"
#define kInverseDistance2FlagLong				"-inverseDistance2"
#define kInverseDistance3Flag					"-id3"
#define kInverseDistance3FlagLong				"-inverseDistance3"
#define kInverseDistance4Flag					"-id4"
#define kInverseDistance4FlagLong				"-inverseDistance4"


class clusterWeightFunctionCmd : public MPxCommand
{
public:
	clusterWeightFunctionCmd();
	virtual ~clusterWeightFunctionCmd();
	virtual MStatus doIt(const MArgList &args);
	static void *creator();
	static MSyntax newSyntax();
	
	enum effectType {
		kSine,
		kSineDistance,
		kSineDistance2,
		kDistanceSineDistance,
		kInverseDistanceSineDistance,
		kDistance,
		kDistance2,
		kDistance3,
		kDistance4,
		kInverseDistance,
		kInverseDistance2,
		kInverseDistance3,
		kInverseDistance4
	};

private:
	MStatus parseArgs(const MArgList &);
	MStatus performWeighting(MFnWeightGeometryFilter &,
							 MDagPath &,
							 MObject &);

	effectType fEffectType;
};


clusterWeightFunctionCmd::clusterWeightFunctionCmd()
{
}


clusterWeightFunctionCmd::~clusterWeightFunctionCmd()
{
}


MStatus clusterWeightFunctionCmd::doIt(const MArgList &argList)
{
	MStatus status;

	MSelectionList list;
	MGlobal::getActiveSelectionList(list);
	
	// Get the cluster
	MFnWeightGeometryFilter cluster;
	MObject dgNode;
	list.getDependNode(0, dgNode);
	if (!cluster.setObject(dgNode))
		return MS::kFailure;

	// Get the object
	MDagPath dagPath;
	MObject component;
	list.getDagPath(1, dagPath, component);

	status = parseArgs(argList);
	if (MS::kSuccess == status)
		status = performWeighting(cluster, dagPath, component);
	return status;
}


void *clusterWeightFunctionCmd::creator()
{
	return new clusterWeightFunctionCmd;
}


MSyntax clusterWeightFunctionCmd::newSyntax()
{
	MSyntax syntax;

	syntax.addFlag(kSineFlag, kSineFlagLong);
	syntax.addFlag(kSineDistanceFlag, kSineDistanceFlagLong);
	syntax.addFlag(kSineDistance2Flag, kSineDistance2FlagLong);
	syntax.addFlag(kDistanceSineDistanceFlag, kDistanceSineDistanceFlagLong);
	syntax.addFlag(kInverseDistanceSineDistanceFlag, 
				   kInverseDistanceSineDistanceFlagLong);
	syntax.addFlag(kDistanceFlag, kDistanceFlagLong);
	syntax.addFlag(kDistance2Flag, kDistance2FlagLong);
	syntax.addFlag(kDistance3Flag, kDistance3FlagLong);
	syntax.addFlag(kDistance4Flag, kDistance4FlagLong);
	syntax.addFlag(kInverseDistanceFlag, kInverseDistanceFlagLong);
	syntax.addFlag(kInverseDistance2Flag, kInverseDistance2FlagLong);
	syntax.addFlag(kInverseDistance3Flag, kInverseDistance3FlagLong);
	syntax.addFlag(kInverseDistance4Flag, kInverseDistance4FlagLong);

	return syntax;
}


MStatus clusterWeightFunctionCmd::parseArgs(const MArgList &argList)
{
	MArgDatabase argData(syntax(), argList);

	if (argData.isFlagSet(kSineFlag)) {
		fEffectType = kSine;
	}
	else if (argData.isFlagSet(kSineDistanceFlag)) {
		fEffectType = kSineDistance;
	}
	else if (argData.isFlagSet(kSineDistance2Flag)) {
		fEffectType = kSineDistance2;
	}
	else if (argData.isFlagSet(kDistanceSineDistanceFlag)) {
		fEffectType = kDistanceSineDistance;
	}
	else if (argData.isFlagSet(kInverseDistanceSineDistanceFlag)) {
		fEffectType = kInverseDistanceSineDistance;
	}
	else if (argData.isFlagSet(kDistanceFlag)) {
		fEffectType = kDistance;
	}
	else if (argData.isFlagSet(kDistance2Flag)) {
		fEffectType = kDistance2;
	}
	else if (argData.isFlagSet(kDistance3Flag)) {
		fEffectType = kDistance3;
	}
	else if (argData.isFlagSet(kDistance4Flag)) {
		fEffectType = kDistance4;
	}
	else if (argData.isFlagSet(kInverseDistanceFlag)) {
		fEffectType = kInverseDistance;
	}
	else if (argData.isFlagSet(kInverseDistance2Flag)) {
		fEffectType = kInverseDistance2;
	}
	else if (argData.isFlagSet(kInverseDistance3Flag)) {
		fEffectType = kInverseDistance3;
	}
	else if (argData.isFlagSet(kInverseDistance4Flag)) {
		fEffectType = kInverseDistance4;
	}

	return MS::kSuccess;
}


MStatus clusterWeightFunctionCmd::performWeighting(MFnWeightGeometryFilter &cluster,
										   MDagPath &dagPath,
										   MObject &component)
{
	MStatus status;
	MItGeometry geomIter(dagPath, component, &status);
	MObject comp;
	MObjectArray compArray;
	MDoubleArray weightArray;
	double weight = 0.0;

	if (MS::kSuccess == status) {
		for (; !geomIter.isDone(); geomIter.next()) {
			comp = geomIter.component();

			MPoint pnt = geomIter.position(MSpace::kWorld, &status);

			if (MS::kSuccess != status) {
				return MS::kFailure;
			}

			if (kSine == fEffectType) {
				weight = sin(pnt.x)*sin(pnt.z);
			}
			else if (kSineDistance == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = sin(distance);
			}
			else if (kSineDistance2 == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = sin(distance*distance);
			}
			else if (kDistanceSineDistance == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = distance*sin(distance);
			}
			else if (kInverseDistanceSineDistance == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = sin(distance)/(distance+1);
			}
			else if (kDistance == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = distance;
			}
			else if (kDistance2 == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = distance*distance;
			}
			else if (kDistance3 == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = distance*distance*distance;
			}
			else if (kDistance4 == fEffectType) {
				double distance = pnt.distanceTo(MPoint::origin);
				weight = distance*distance*distance*distance;
			}
			else if (kInverseDistance == fEffectType) {
				double dist = pnt.distanceTo(MPoint::origin) + 1;
				weight = 1/(dist);
			}
			else if (kInverseDistance2 == fEffectType) {
				double dist = pnt.distanceTo(MPoint::origin) + 1;
				weight = 1/(dist*dist);
			}
			else if (kInverseDistance3 == fEffectType) {
				double dist = pnt.distanceTo(MPoint::origin) + 1;
				weight = 1/(dist*dist*dist);
			}
			else if (kInverseDistance4 == fEffectType) {
				double dist = pnt.distanceTo(MPoint::origin) + 1;
				weight = 1/(dist*dist*dist*dist);
			}

			compArray.append(comp);
			weightArray.append(weight);
		}

		unsigned length = compArray.length();
		for (unsigned i = 0; i < length; i++) {
			cluster.setWeight(dagPath, compArray[i], (float) weightArray[i]);
		}

		return MS::kSuccess;
	} else
		return MS::kFailure;
}


extern MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "3.0", "Any");
	
	status = plugin.registerCommand("clusterWeightFunction",
									clusterWeightFunctionCmd::creator,
									clusterWeightFunctionCmd::newSyntax);
	
	if (!status) {
		status.perror("registerCommand");
	}

	return status;
}


extern MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);
	
	status = plugin.deregisterCommand("clusterWeightFunction");

	if (!status) {
		status.perror("deregisterCommand");
	}	

	return status;
}

