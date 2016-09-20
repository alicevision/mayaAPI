/** Copyright 2015 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomFileUtils.cpp
//
//
//		Utility classes to read and write .atom files.
//

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <set>
#include <string>

#include <maya/MIOStream.h>
#include <maya/MFStream.h>

#include "atomFileUtils.h"
#include "atomImportExportStrings.h"
#include "atomNodeNameReplacer.h"
#include "atomCachedPlugs.h"
#include "atomAnimLayers.h"
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MFnAnimCurve.h>
#include <maya/MAnimCurveClipboard.h>
#include <maya/MAnimCurveClipboardItem.h>
#include <maya/MAnimCurveClipboardItemArray.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MObjectArray.h>
#include <maya/MItSelectionList.h>
#include <maya/MFn.h>
#include <maya/MItDag.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MAnimControl.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MDGModifier.h>

#if defined (OSMac_)
#	include <sys/param.h>
using namespace std;
#endif
//-------------------------------------------------------------------------
//	Class atomUnitNames
//-------------------------------------------------------------------------

//	String names for units.
//
const char *kMmString = 		"mm";
const char *kCmString =			"cm";
const char *kMString =			"m";
const char *kKmString =			"km";
const char *kInString =			"in";
const char *kFtString =			"ft";
const char *kYdString =			"yd";
const char *kMiString =			"mi";

const char *kMmLString =		"millimeter";
const char *kCmLString =		"centimeter";
const char *kMLString =			"meter";
const char *kKmLString =		"kilometer";
const char *kInLString =		"inch";
const char *kFtLString =		"foot";
const char *kYdLString =		"yard";
const char *kMiLString =		"mile";

const char *kRadString =		"rad";
const char *kDegString =		"deg";
const char *kMinString =		"min";
const char *kSecString =		"sec";

const char *kRadLString =		"radian";
const char *kDegLString =		"degree";
const char *kMinLString =		"minute";
const char *kSecLString =		"second";

const char *kHourTString =		"hour";
const char *kMinTString =		"min";
const char *kSecTString =		"sec";
const char *kMillisecTString =	"millisec";

const char *kGameTString =		"game";
const char *kFileTString =		"film";
const char *kPalTString =		"pal";
const char *kNtscTString =		"ntsc";
const char *kShowTString =		"show";
const char *kPalFTString =		"palf";
const char *kNtscFTString =		"ntscf";

const char *kUnitlessString = "unitless";
const char *kUnknownTimeString =	"Unknown Time Unit";
const char *kUnknownAngularString =	"Unknown Angular Unit";
const char *kUnknownLinearString = 	"Unknown Linear Unit";

const char *kExportEditsString = "offlineFile"; //in atom 0.1 this will be followed by the .editMA file name. 
												//In atom 1.0 this will be followed by a  " ;" which will tell us that		
												//the end of the file will contain an embedded .editMA file.

const char *kExportEditsDataString = "offlineFileData"; //in atom 1.0, the data following this is the .editMA file. This must come last since
														//we will read this data to the end of the file and copy all of this data over to a 
														//temporary .editMA file that we will then use to import.

atomUnitNames::atomUnitNames()
//
//	Description:
//		Class constructor.
//
{
}

atomUnitNames::~atomUnitNames()
//
//	Description:
//		Class destructor.
//
{
}

/* static */
void atomUnitNames::setToLongName(const MAngle::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the long text name of the angle unit.
//
{
	switch(unit) {
		case MAngle::kDegrees:
			name.set(kDegLString);
			break;
		case MAngle::kRadians:
			name.set(kRadLString);
			break;
		case MAngle::kAngMinutes:
			name.set(kMinLString);
			break;
		case MAngle::kAngSeconds:
			name.set(kSecLString);
			break;
		default:
			name.set(kUnknownAngularString);
			break;
	}
}

/* static */
void atomUnitNames::setToShortName(const MAngle::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the short text name of the angle unit.
//
{
	switch(unit) {
		case MAngle::kDegrees:
			name.set(kDegString);
			break;
		case MAngle::kRadians:
			name.set(kRadString);
			break;
		case MAngle::kAngMinutes:
			name.set(kMinString);
			break;
		case MAngle::kAngSeconds:
			name.set(kSecString);
			break;
		default:
			name.set(kUnknownAngularString);
			break;
	}
}

/* static */
void atomUnitNames::setToLongName(const MDistance::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the long text name of the distance unit.
//
{
	switch(unit) {
		case MDistance::kInches:
			name.set(kInLString);
			break;
		case MDistance::kFeet:
			name.set(kFtLString);
			break;
		case MDistance::kYards:
			name.set(kYdLString);
			break;
		case MDistance::kMiles:
			name.set(kMiLString);
			break;
		case MDistance::kMillimeters:
			name.set(kMmLString);
			break;
		case MDistance::kCentimeters:
			name.set(kCmLString);
			break;
		case MDistance::kKilometers:
			name.set(kKmLString);
			break;
		case MDistance::kMeters:
			name.set(kMLString);
			break;
		default:
			name.set(kUnknownLinearString);
			break;
	}
}

/* static */
void atomUnitNames::setToShortName(const MDistance::Unit& unit, MString& name)
//
//	Description:
//		Sets the string with the short text name of the distance unit.
//
{
	switch(unit) {
		case MDistance::kInches:
			name.set(kInString);
			break;
		case MDistance::kFeet:
			name.set(kFtString);
			break;
		case MDistance::kYards:
			name.set(kYdString);
			break;
		case MDistance::kMiles:
			name.set(kMiString);
			break;
		case MDistance::kMillimeters:
			name.set(kMmString);
			break;
		case MDistance::kCentimeters:
			name.set(kCmString);
			break;
		case MDistance::kKilometers:
			name.set(kKmString);
			break;
		case MDistance::kMeters:
			name.set(kMString);
			break;
		default:
			name.set(kUnknownLinearString);
			break;
	}
}

/* static */
void atomUnitNames::setToLongName(const MTime::Unit &unit, MString &name)
//
//	Description:
//		Sets the string with the long text name of the time unit.
//
{
	switch(unit) {
		case MTime::kHours:
			name.set(kHourTString);
			break;
		case MTime::kMinutes:
			name.set(kMinTString);
			break;
		case MTime::kSeconds:
			name.set(kSecTString);
			break;
		case MTime::kMilliseconds:
			name.set(kMillisecTString);
			break;
		case MTime::kGames:
			name.set(kGameTString);
			break;
		case MTime::kFilm:
			name.set(kFileTString);
			break;
		case MTime::kPALFrame:
			name.set(kPalTString);
			break;
		case MTime::kNTSCFrame:
			name.set(kNtscTString);
			break;
		case MTime::kShowScan:
			name.set(kShowTString);
			break;
		case MTime::kPALField:
			name.set(kPalFTString);
			break;
		case MTime::kNTSCField:
			name.set(kNtscFTString);
			break;
		default:
			name.set(kUnknownTimeString);
			break;
	}
}

/* static */
void atomUnitNames::setToShortName(const MTime::Unit &unit, MString &name)
//
//	Description:
//		Sets the string with the short text name of the time unit.
//
{
	setToLongName(unit, name);
}

/* static */
bool atomUnitNames::setFromName(const MString &str, MAngle::Unit &unit)
//
//	Description:
//		The angle unit is set based on the passed string. If the string
//		is not recognized, the angle unit is set to MAngle::kInvalid.
//
{
	bool state = true;

	const char *name = str.asChar();

	if ((strcmp(name, kDegString) == 0) || 
		(strcmp(name, kDegLString) == 0)) {
		unit = MAngle::kDegrees;
	} else if (	(strcmp(name, kRadString) == 0) ||
				(strcmp(name, kRadLString) == 0)) {
		unit = MAngle::kRadians;
	} else if (	(strcmp(name, kMinString) == 0) ||
				(strcmp(name, kMinLString) == 0)) {
		unit = MAngle::kAngMinutes;
	} else if (	(strcmp(name, kSecString) == 0) ||
				(strcmp(name, kSecLString) == 0)) {
		unit = MAngle::kAngSeconds;
	} else {
		//	This is not a recognized angular unit.
		//
		unit = MAngle::kInvalid;
		MStatus stat;
		MString msg; 
		// Use format to place variable string into message
		MString msgFmt = MStringResource::getString(kInvalidAngleUnits, stat);
		msg.format(msgFmt, str);
		MGlobal::displayError(msg);
		state = false;
	}

	return state;
}

/* static */
bool atomUnitNames::setFromName(const MString &str, MDistance::Unit &unit)
//
//	Description:
//		The distance unit is set based on the passed string. If the string
//		is not recognized, the distance unit is set to MDistance::kInvalid.
//
{
	bool state = true;

	const char *name = str.asChar();

	if ((strcmp(name, kInString) == 0) ||
		(strcmp(name, kInLString) == 0)) {
		unit = MDistance::kInches;
	} else if (	(strcmp(name, kFtString) == 0) ||
				(strcmp(name, kFtLString) == 0)) {
		unit = MDistance::kFeet;
	} else if (	(strcmp(name, kYdString) == 0) ||
				(strcmp(name, kYdLString) == 0)) {
		unit = MDistance::kYards;
	} else if (	(strcmp(name, kMiString) == 0) ||
				(strcmp(name, kMiLString) == 0)) {
		unit = MDistance::kMiles;
	} else if (	(strcmp(name, kMmString) == 0) ||
				(strcmp(name, kMmLString) == 0)) {
		unit = MDistance::kMillimeters;
	} else if (	(strcmp(name, kCmString) == 0) ||
				(strcmp(name, kCmLString) == 0)) {
		unit = MDistance::kCentimeters;
	} else if (	(strcmp(name, kKmString) == 0) ||
				(strcmp(name, kKmLString) == 0)) {
		unit = MDistance::kKilometers;
	} else if (	(strcmp(name, kMString) == 0) ||
				(strcmp(name, kMLString) == 0)) {
		unit = MDistance::kMeters;
	} else {
		//  This is not a recognized distance unit.
		//
		state = false;
		MStatus stat;
		MString msg; 
		// Use format to place variable string into message
		MString msgFmt = MStringResource::getString(kInvalidLinearUnits, stat);
		msg.format(msgFmt, str);
		MGlobal::displayError(msg);
		unit = MDistance::kInvalid;
	}

	return state;
}

/* static */
bool atomUnitNames::setFromName(const MString &str, MTime::Unit &unit)
//
//	Description:
//		The time unit is set based on the passed string. If the string
//		is not recognized, the time unit is set to MTime::kInvalid.
//
{
	bool state = true;
	const char *name = str.asChar();

	if (strcmp(name, kHourTString) == 0) {
		unit = MTime::kHours;
	} else if (strcmp(name, kMinTString) == 0) {
		unit = MTime::kMinutes;
	} else if (strcmp(name, kSecTString) == 0) {
		unit = MTime::kSeconds;
	} else if (strcmp(name, kMillisecTString) == 0) {
		unit = MTime::kMilliseconds;
	} else if (strcmp(name, kGameTString) == 0) {
		unit = MTime::kGames;
	} else if (strcmp(name, kFileTString) == 0) {
		unit = MTime::kFilm;
	} else if (strcmp(name, kPalTString) == 0) {
		unit = MTime::kPALFrame;
	} else if (strcmp(name, kNtscTString) == 0) {
		unit = MTime::kNTSCFrame;
	} else if (strcmp(name, kShowTString) == 0) {
		unit = MTime::kShowScan;
	} else if (strcmp(name, kPalFTString) == 0) {
		unit = MTime::kPALField;
	} else if (strcmp(name, kNtscFTString) == 0) {
		unit = MTime::kNTSCField;
	} else {
		//  This is not a recognized time unit.
		//
		unit = MTime::kInvalid;
		MStatus stat;
		MString msg; 
		// Use format to place variable string into message
		MString msgFmt = MStringResource::getString(kInvalidTimeUnits, stat);
		msg.format(msgFmt, str);
		MGlobal::displayError(msg);
		state = false;
	}

	return state;
}

//-------------------------------------------------------------------------
//	Class atomBase
//-------------------------------------------------------------------------

// Tangent type words
//
const char *kWordTangentGlobal = "global";
const char *kWordTangentFixed = "fixed";
const char *kWordTangentLinear = "linear";
const char *kWordTangentFlat = "flat";
const char *kWordTangentSmooth = "spline";
const char *kWordTangentStep = "step";
const char *kWordTangentSlow = "slow";
const char *kWordTangentFast = "fast";
const char *kWordTangentClamped = "clamped";
const char *kWordTangentPlateau = "plateau";
const char *kWordTangentStepNext = "stepnext";
const char *kWordTangentAuto = "auto";

// Infinity type words
//
const char *kWordConstant = "constant";
const char *kWordLinear = "linear";
const char *kWordCycle = "cycle";
const char *kWordCycleRelative = "cycleRelative";
const char *kWordOscillate = "oscillate";

//	Param Curve types
//
const char *kWordTypeUnknown = "unknown";
const char *kWordTypeLinear = "linear";
const char *kWordTypeAngular = "angular";
const char *kWordTypeTime = "time";
const char *kWordTypeUnitless = "unitless";

//	Keywords
//
const char *kDagNode = "dagNode";
const char *kDependNode = "node";
const char *kShapeNode = "shape";
const char *kAnimLayer = "animLayer";
const char *kAnim = "anim";
const char *kAnimData = "animData";
const char *kMovData = "movData";
const char *kMayaVersion = "mayaVersion";
const char *kAtomVersion = "atomVersion";
const char *kMayaSceneFile = "mayaSceneFile";
const char *kStatic = "static";
const char *kCached = "cached";

const char *kTimeUnit = "timeUnit";
const char *kLinearUnit = "linearUnit";
const char *kAngularUnit = "angularUnit";
const char *kStartTime = "startTime";
const char *kEndTime = "endTime";
const char *kStartUnitless = "startUnitless";
const char *kEndUnitless = "endUnitless";

// atomVersions:
//
//const char *kAtomVersionString = "0.1";				//intitial implementation, mainly external edit files, most alpha and beta's for 2013
const char *kAtomVersionString = "1.0";				//2013

const char *kTwoSpace = "  ";
const char *kFourSpace = "    ";

//	animData keywords
//
const char *kInputString = "input";
const char *kOutputString = "output";
const char *kWeightedString = "weighted";
const char *kPreInfinityString = "preInfinity";
const char *kPostInfinityString = "postInfinity";
const char *kInputUnitString = "inputUnit";
const char *kOutputUnitString = "outputUnit";
const char *kTanAngleUnitString = "tangentAngleUnit";
const char *kKeysString = "keys";

//	special characters
//
const char kSemiColonChar	= ';';
const char kSpaceChar		= ' ';
//const char kTabChar			= '\t';
//const char kHashChar		= '#';
const char kNewLineChar		= '\n';
//const char kSlashChar		= '/';
const char kBraceLeftChar	= '{';
const char kBraceRightChar	= '}';
//const char kDoubleQuoteChar	= '"';

atomBase::atomBase ()
//
//	Description:
//		The constructor.
//
{
	resetUnits();
}
	
atomBase::~atomBase()
//
//	Description:
//		The destructor.
//
{
}

void atomBase::resetUnits()
//
//	Description:
//		Reset the units used by this class to the ui units.
//
{
	timeUnit = MTime::uiUnit();
	linearUnit = MDistance::uiUnit();
	angularUnit = MAngle::uiUnit();
}

const char *
atomBase::tangentTypeAsWord(MFnAnimCurve::TangentType type)
//
//	Description:
//		Returns a string with a test based desription of the passed
//		MFnAnimCurve::TangentType. 
//
{
	switch (type) {
		case MFnAnimCurve::kTangentGlobal:
			return (kWordTangentGlobal);
		case MFnAnimCurve::kTangentFixed:
			return (kWordTangentFixed);
		case MFnAnimCurve::kTangentLinear:
			return (kWordTangentLinear);
		case MFnAnimCurve::kTangentFlat:
			return (kWordTangentFlat);
		case MFnAnimCurve::kTangentSmooth:
			return (kWordTangentSmooth);
		case MFnAnimCurve::kTangentStep:
			return (kWordTangentStep);
		case MFnAnimCurve::kTangentStepNext:
			return (kWordTangentStepNext);
		case MFnAnimCurve::kTangentSlow:
			return (kWordTangentSlow);
		case MFnAnimCurve::kTangentFast:
			return (kWordTangentFast);
		case MFnAnimCurve::kTangentClamped:
			return (kWordTangentClamped);
		case MFnAnimCurve::kTangentPlateau:
			return (kWordTangentPlateau);
		case MFnAnimCurve::kTangentAuto:
			return (kWordTangentAuto);
		default:
			break;
	}
	return (kWordTangentGlobal);
}

MFnAnimCurve::TangentType
atomBase::wordAsTangentType (char *type)
//
//	Description:
//		Returns a MFnAnimCurve::TangentType based on the passed string.
//		If the string is not a recognized tangent type, tnen
//		MFnAnimCurve::kTangentGlobal is returned.
//
{
	if (strcmp(type, kWordTangentGlobal) == 0) {
		return (MFnAnimCurve::kTangentGlobal);
	}
	if (strcmp(type, kWordTangentFixed) == 0) {
		return (MFnAnimCurve::kTangentFixed);
	}
	if (strcmp(type, kWordTangentLinear) == 0) {
		return (MFnAnimCurve::kTangentLinear);
	}
	if (strcmp(type, kWordTangentFlat) == 0) {
		return (MFnAnimCurve::kTangentFlat);
	}
	if (strcmp(type, kWordTangentSmooth) == 0) {
		return (MFnAnimCurve::kTangentSmooth);
	}
	if (strcmp(type, kWordTangentStep) == 0) {
		return (MFnAnimCurve::kTangentStep);
	}
	if (strcmp(type, kWordTangentStepNext) == 0) {
		return (MFnAnimCurve::kTangentStepNext);
	}
	if (strcmp(type, kWordTangentSlow) == 0) {
		return (MFnAnimCurve::kTangentSlow);
	}
	if (strcmp(type, kWordTangentFast) == 0) {
		return (MFnAnimCurve::kTangentFast);
	}
	if (strcmp(type, kWordTangentClamped) == 0) {
		return (MFnAnimCurve::kTangentClamped);
	}
	if (strcmp(type, kWordTangentPlateau) == 0) {
		return (MFnAnimCurve::kTangentPlateau);
	}
	if (strcmp(type, kWordTangentAuto) == 0) {
		return (MFnAnimCurve::kTangentAuto);
	}
	return (MFnAnimCurve::kTangentGlobal);
}

const char *
atomBase::infinityTypeAsWord(MFnAnimCurve::InfinityType type)
//	
//	Description:
//		Returns a string containing the name of the passed 
//		MFnAnimCurve::InfinityType type.
//
{
	switch (type) {
		case MFnAnimCurve::kConstant:
			return (kWordConstant);
		case MFnAnimCurve::kLinear:
			return (kWordLinear);
		case MFnAnimCurve::kCycle:
			return (kWordCycle);
		case MFnAnimCurve::kCycleRelative:
			return (kWordCycleRelative);
		case MFnAnimCurve::kOscillate:
			return (kWordOscillate);
		default:
			break;
	}
	return (kWordConstant);
}

MFnAnimCurve::InfinityType
atomBase::wordAsInfinityType(const char *type)
//
//	Description:
//		Returns a MFnAnimCurve::InfinityType from the passed string.
//		If the string does not match a recognized infinity type,
//		MFnAnimCurve::kConstant is returned.
//
{
	if (strcmp(type, kWordConstant) == 0) {
		return(MFnAnimCurve::kConstant);
	} else if (strcmp(type, kWordLinear) == 0) {
		return (MFnAnimCurve::kLinear);
	} else if (strcmp(type, kWordCycle) == 0) {
		return (MFnAnimCurve::kCycle);
	} else if (strcmp(type, kWordCycleRelative) == 0) {
		return (MFnAnimCurve::kCycleRelative);
	} else if (strcmp(type, kWordOscillate) == 0) {
		return (MFnAnimCurve::kOscillate);
	}

	return (MFnAnimCurve::kConstant);
}

const char *
atomBase::outputTypeAsWord (MFnAnimCurve::AnimCurveType type)
//
//	Description:
//		Returns a string identifying the output type of the
//		passed MFnAnimCurve::AnimCurveType.
//
{
	switch (type) {
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			return (kWordTypeLinear);
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			return (kWordTypeAngular);
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			return (kWordTypeTime);
		case MFnAnimCurve::kAnimCurveTU:
		case MFnAnimCurve::kAnimCurveUU:
			return (kWordTypeUnitless);
		case MFnAnimCurve::kAnimCurveUnknown:
			return (kWordTypeUnitless);
	}
	return (kWordTypeUnknown);
}

atomBase::AnimBaseType
atomBase::wordAsInputType(const char *input)
//
//	Description:
//		Returns an input type based on the passed string.
//
{
	if (strcmp(input, kWordTypeTime) == 0) {
		return atomBase::kAnimBaseTime;
	} else {
		return atomBase::kAnimBaseUnitless;
	}
}

atomBase::AnimBaseType
atomBase::wordAsOutputType(const char *output) 
//
//	Description:
//		Returns a output type based on the passed string.
//
{
	if (strcmp(output, kWordTypeLinear) == 0) {
		return atomBase::kAnimBaseLinear;
	} else if (strcmp(output, kWordTypeAngular) == 0) {
		return atomBase::kAnimBaseAngular;
	} else if (strcmp(output, kWordTypeTime) == 0) {
		return atomBase::kAnimBaseTime;
	} else {
		return atomBase::kAnimBaseUnitless;
	}
}

const char *
atomBase::boolInputTypeAsWord(bool isUnitless) 
//
//	Description:
//		Returns a string based on a bool. 
//
{
	if (isUnitless) {
		return (kWordTypeUnitless);
	} else {
		return (kWordTypeTime);
	}
}


MFnAnimCurve::AnimCurveType
atomBase::typeAsAnimCurveType(	atomBase::AnimBaseType input, 
								atomBase::AnimBaseType output)
//
//	Description:
//		Returns a MFnAnimCurve::AnimCurveType based on the passed
//		input and output types. If the input and output types do
//		not create a valid MFnAnimCurve::AnimCurveType, then a
//		MFnAnimCurve::kAnimCurveUnknown is returned.
//
{
	MFnAnimCurve::AnimCurveType type = MFnAnimCurve::kAnimCurveUnknown;

	switch (output) {
		case kAnimBaseLinear:
			if (kAnimBaseUnitless == input) {
				type = MFnAnimCurve::kAnimCurveUL;
			} else {
				type = MFnAnimCurve::kAnimCurveTL;
			}
			break;
		case kAnimBaseAngular:
			if (kAnimBaseUnitless == input) {
				type = MFnAnimCurve::kAnimCurveUA;
			} else {
				type = MFnAnimCurve::kAnimCurveTA;
			}
			break;
		case kAnimBaseTime:
			if (kAnimBaseUnitless == input) {
				type = MFnAnimCurve::kAnimCurveUT;
			} else {
				type = MFnAnimCurve::kAnimCurveTT;
			}
			break;
		case kAnimBaseUnitless:
			if (kAnimBaseUnitless == input) {
				type = MFnAnimCurve::kAnimCurveUU;
			} else {
				type = MFnAnimCurve::kAnimCurveTU;
			}
			break;
		default:
			//	An unknown anim curve type.
			//
			break;
	}

	return type;
}

bool atomBase::isEquivalent(double a, double b)
//
//	Description:
//		Returns true if the doubles are within the tolerance.
//
{
	const double tolerance = 1.0e-10;
	return ((a > b) ? (a - b <= tolerance) : (b - a <= tolerance));
}

void atomBase::getAttrName(MPlug &plug, MString &attributeName)
{
	attributeName = plug.partialName(/*includeNodeName*/false,
		/*includeNonMandatoryIndices*/false,
		/*includeInstancedIndices=false*/false,
		/* useAlias=false*/ true,
		/* useFullAttributePath=false*/false,
		/* useLongNames*/true );
}

bool atomBase::getPlug(const MString &nodeName, const MString &attributeName, MPlug &plug)
{
	MSelectionList mList;
	MString nodeAndAttr = nodeName + MString(".") + attributeName;
	mList.add(nodeAndAttr);
	if(mList.length() != 1)
	{
		return false;
	}
	MStatus status = mList.getPlug( 0, plug);
	if(status != MS::kSuccess)
	{	
		return false;
	}
	return true;
	//mz to do delete if no good.

}


//-----------------------------------------------------------------------------
//	Class atomReader
//-----------------------------------------------------------------------------

atomReader::atomReader ()
//
//	Description:
//		Class constructor.
//
:	animVersion (1.0)
,	convertAnglesFromV2To3(false)
,	convertAnglesFromV3To2(false)
,   mImportStartFrame(0.0f)
,   mImportEndFrame(0.0f)
,	mImportCustomFrameRange(false)
{
}
	
atomReader::~atomReader()
//
//	Description:
//		Class destructor.
//
{
}

void atomReader::convertAnglesAndWeights3To2(MFnAnimCurve::AnimCurveType type, 
								bool isWeighted, MAngle &angle, double &weight)
//
//	Description:
//		Converts the tangent angles from Maya 3.0 to Maya2.* formats.
//
{
	double oldAngle = angle.as(MAngle::kRadians);
	double newAngle = oldAngle;

	//	Calculate the scale values for the conversion.
	//
	double xScale = 1.0;
	double yScale = 1.0;

	MTime tOne(1.0, MTime::kSeconds);
	if (type == MFnAnimCurve::kAnimCurveTT ||
		type == MFnAnimCurve::kAnimCurveTL ||
		type == MFnAnimCurve::kAnimCurveTA ||
		type == MFnAnimCurve::kAnimCurveTU) {

		xScale = tOne.as(MTime::uiUnit());
	}

	switch (type) {
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			yScale = tOne.as(MTime::uiUnit());
			break;
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			{
				MDistance dOne(1.0, MDistance::internalUnit());
				yScale = dOne.as(linearUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			{
				MAngle aOne(1.0, MAngle::internalUnit());
				yScale = aOne.as(angularUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTU:
		case MFnAnimCurve::kAnimCurveUU:
		default:
			break;
	}

	double tanAngle = tan(oldAngle);
	newAngle = atan((xScale*tanAngle)/yScale);

	if (isWeighted) {
		double sinAngle = sin(oldAngle);
		double cosAngle = cos(oldAngle);

		double denominator = (yScale*yScale*sinAngle*sinAngle) +
								(xScale*xScale*cosAngle*cosAngle);
		weight = sqrtl(weight/denominator);
	}

	MAngle finalAngle(newAngle, MAngle::kRadians);
	angle = finalAngle;
}

void atomReader::convertAnglesAndWeights2To3(MFnAnimCurve::AnimCurveType type, 
								bool isWeighted, MAngle &angle, double &weight)
//
//	Description:
//		Converts the tangent angles from Maya 2.* to Maya3.0+ formats.
//
{
	double oldAngle = angle.as(MAngle::kRadians);

	double newAngle = oldAngle;
	double newWeight = weight;

	//	Calculate the scale values for the conversion.
	//
	double xScale = 1.0;
	double yScale = 1.0;

	MTime tOne(1.0, MTime::kSeconds);
	if (type == MFnAnimCurve::kAnimCurveTT ||
		type == MFnAnimCurve::kAnimCurveTL ||
		type == MFnAnimCurve::kAnimCurveTA ||
		type == MFnAnimCurve::kAnimCurveTU) {

		xScale = tOne.as(MTime::uiUnit());
	}

	switch (type) {
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			yScale = tOne.as(MTime::uiUnit());
			break;
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			{
				MDistance dOne(1.0, MDistance::internalUnit());
				yScale = dOne.as(linearUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			{
				MAngle aOne(1.0, MAngle::internalUnit());
				yScale = aOne.as(angularUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTU:
		case MFnAnimCurve::kAnimCurveUU:
		default:
			break;
	}

	const double quarter = M_PI/2;
	if (isEquivalent(oldAngle, 0.0) ||
		isEquivalent(oldAngle, quarter) ||
		isEquivalent(oldAngle, -quarter)) {
		
		newAngle = oldAngle;

		if (isWeighted) {
			newWeight = yScale*oldAngle;
		}
	} else {
		double tanAngle = tan(oldAngle);
		newAngle = atan((yScale*tanAngle)/xScale);
			
		if (isWeighted) {
			double cosAngle = cos(oldAngle);
			double cosSq = cosAngle*cosAngle;

			double wSq = (weight*weight) * 
					(((xScale*xScale - yScale*yScale)*cosSq) + (yScale*yScale));

			newWeight = sqrtl(wSq);
		}
	}

	weight = newWeight;

	MAngle finalAngle(newAngle, MAngle::kRadians);
	angle = finalAngle;
}

bool atomReader::readAnimCurve(
ifstream &clipFile, MAnimCurveClipboardItem &item)
//
//	Description:
//		Read a block of the ifstream that should contain anim curve
//		data in the format determined by the animData keyword.
//
{
	MFnAnimCurve animCurve;
	MObject animCurveObj; 

	//	Anim curve defaults.
	//
	atomBase::AnimBaseType input = wordAsInputType(kWordTypeTime);
	atomBase::AnimBaseType output = wordAsOutputType(kWordTypeLinear);
	MFnAnimCurve::InfinityType preInf = wordAsInfinityType(kWordConstant);
	MFnAnimCurve::InfinityType postInf = wordAsInfinityType(kWordConstant);

	MString inputUnitName;
	atomUnitNames::setToShortName(timeUnit, inputUnitName);
	MString outputUnitName;
	MAngle::Unit tanAngleUnit = angularUnit;
	bool isWeighted (false);

	char *dataType;
	while (!clipFile.eof()) {
		advance(clipFile);

		dataType = asWord(clipFile);

		if (strcmp(dataType, kInputString) == 0) {
			input = wordAsInputType(asWord(clipFile));
		} else if (strcmp(dataType, kOutputString) == 0) {
			output = wordAsOutputType(asWord(clipFile));
		} else if (strcmp(dataType, kWeightedString) == 0) {
			isWeighted = (asDouble(clipFile) == 1.0);
		} else if (strcmp(dataType, kPreInfinityString) == 0) {
			preInf = wordAsInfinityType(asWord(clipFile));
		} else if (strcmp(dataType, kPostInfinityString) == 0) {
			postInf = wordAsInfinityType(asWord(clipFile));
		} else if (strcmp(dataType, kInputUnitString) == 0) {
			inputUnitName.set(asWord(clipFile));
		} else if (strcmp(dataType, kOutputUnitString) == 0) {
			outputUnitName.set(asWord(clipFile));
		} else if (strcmp(dataType, kTanAngleUnitString) == 0) {
			MString tUnit(asWord(clipFile));
			if (!atomUnitNames::setFromName(tUnit, tanAngleUnit)) {
				MString unitName;
				tanAngleUnit = angularUnit;
				atomUnitNames::setToShortName(tanAngleUnit, unitName);

				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kSettingTanAngleUnit, stat);
				msg.format(msgFmt, unitName);
				MGlobal::displayError(msg);
			}
		} else if (strcmp(dataType, kKeysString) == 0) {
			//	Ignore the rest of this line.
			//
			clipFile.ignore(INT_MAX, kNewLineChar);
			break;
		} else if (strcmp(dataType, "{") == 0) {
			//	Skipping the '{' character. Just ignore it.
			//
			continue;
		} else {
			//	An unrecogized keyword was found.
			//
			MString warnStr(dataType);
			MStatus stat;
			MString msg; 
			// Use format to place variable string into message
			MString msgFmt = MStringResource::getString(kUnknownKeyword,
														stat);
			msg.format(msgFmt, warnStr);
			MGlobal::displayError(msg);
			continue;
		}
	}

	// Read the animCurve
	//
	MStatus status;
	MFnAnimCurve::AnimCurveType type = typeAsAnimCurveType(input, output);
	animCurveObj = animCurve.create(type, NULL, &status);

	if (status != MS::kSuccess) {
		MString msg = MStringResource::getString(kCouldNotCreateAnim, status);
		MGlobal::displayError(msg);
		return false;
	}

	animCurve.setIsWeighted(isWeighted);
	animCurve.setPreInfinityType(preInf);
	animCurve.setPostInfinityType(postInf);

	//	Set the appropriate units.
	//
	MTime::Unit inputTimeUnit;
	if (input == kAnimBaseTime) {
		if (!atomUnitNames::setFromName(inputUnitName, inputTimeUnit)) {
			MString unitName;
			inputTimeUnit = timeUnit;
			atomUnitNames::setToShortName(inputTimeUnit, unitName);
			MStatus stat;
			MString msg; 
			// Use format to place variable string into message
			MString msgFmt = MStringResource::getString(kSettingToUnit,
														stat);
			msg.format(msgFmt, kInputUnitString, unitName);
			MGlobal::displayWarning(msg);
		}
	}

	MTime::Unit outputTimeUnit;
	if (output == kAnimBaseTime) {
		if (!atomUnitNames::setFromName(outputUnitName, outputTimeUnit)) {
			MString unitName;
			outputTimeUnit = timeUnit;
			atomUnitNames::setToShortName(outputTimeUnit, unitName);
			MStatus stat;
			MString msg; 
			// Use format to place variable string into message
			MString msgFmt = MStringResource::getString(kSettingToUnit,
														stat);
			msg.format(msgFmt, kOutputUnitString, unitName);
			MGlobal::displayWarning(msg);
		}
	}

	unsigned int index = 0;
	double conversion = 1.0;
	if (output == kAnimBaseLinear) {
		MDistance::Unit unit;
		if (outputUnitName.length() != 0) {
			if (!atomUnitNames::setFromName(outputUnitName, unit)) {
				MString unitName;
				unit = linearUnit;
				atomUnitNames::setToShortName(unit, unitName);
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kSettingToUnit,
															stat);
				msg.format(msgFmt, kOutputUnitString, unitName);
				MGlobal::displayWarning(msg);
			}
		} else {
			unit = linearUnit;
		}
		if (unit != MDistance::kCentimeters) {
			MDistance one(1.0, unit);
			conversion = one.as(MDistance::kCentimeters);
		}
	} else if (output == kAnimBaseAngular) {
		MAngle::Unit unit;
		if (outputUnitName.length() != 0) {
			if (!atomUnitNames::setFromName(outputUnitName, unit)) {
				MString unitName;
				unit = angularUnit;
				atomUnitNames::setToShortName(unit, unitName);
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kSettingToUnit,
															stat);
				msg.format(msgFmt, kOutputUnitString, unitName);
				MGlobal::displayWarning(msg);
			}
		} else {
			unit = angularUnit;
		}
		if (unit != MAngle::kRadians) {
			MAngle one(1.0, unit);
			conversion = one.as(MAngle::kRadians);
		}
	}

	// Now read each keyframe
	//
	bool lIsFirstFrame = true; 
	double lLowestFrame;
	double lHighestFrame;
	advance(clipFile);
	char c = clipFile.peek();
	while (clipFile && c != kBraceRightChar) {
		double t = asDouble (clipFile);
		double val = asDouble (clipFile);

		if(lIsFirstFrame)
		{
			lLowestFrame    = t;
			lHighestFrame   = t;
			lIsFirstFrame = false;
		}
		else
		{
			if(t < lLowestFrame)
				lLowestFrame = t;
			if(t > lHighestFrame)
				lHighestFrame = t;
		}
		MFnAnimCurve::TangentType tanIn = wordAsTangentType(asWord(clipFile));
		MFnAnimCurve::TangentType tanOut = wordAsTangentType(asWord(clipFile));

		switch (type) {
			case MFnAnimCurve::kAnimCurveTT:
				index = animCurve.addKey(	MTime(val, inputTimeUnit),
											MTime(val, outputTimeUnit),
											tanIn, tanOut, NULL, &status);
				break;
			case MFnAnimCurve::kAnimCurveTL:
			case MFnAnimCurve::kAnimCurveTA:
			case MFnAnimCurve::kAnimCurveTU:
				index = animCurve.addKey(	MTime(t, inputTimeUnit),
											val*conversion, tanIn, tanOut,
											NULL, &status);
				break;
			case MFnAnimCurve::kAnimCurveUL:
			case MFnAnimCurve::kAnimCurveUA:
			case MFnAnimCurve::kAnimCurveUU:
				index = animCurve.addKey(	t, val*conversion, 
											tanIn, tanOut,
											NULL, &status);
				break;
			case MFnAnimCurve::kAnimCurveUT:
				index = animCurve.addKey(	t, MTime(val, outputTimeUnit),
											tanIn, tanOut,
											NULL, &status);
				break;
			default:
				MString msg = MStringResource::getString(kUnknownNode, status);
				MGlobal::displayError(msg);
				return false;
		}

		if (status != MS::kSuccess) {
			MStatus stringStat;
			MString msg = MStringResource::getString(kCouldNotKey, stringStat);
			MGlobal::displayError(msg);
		}

		//	Tangent locking needs to be called after the weights and 
		//	angles are set for the fixed tangents.
		//
		bool tLocked = bool(asDouble(clipFile) == 1.0);
		bool swLocked = bool(asDouble(clipFile) == 1.0);
		bool isBreakdown =bool(asDouble(clipFile) == 1.0);

		//	Only fixed tangents need additional information.
		//
		if (tanIn == MFnAnimCurve::kTangentFixed) {
			MAngle inAngle(asDouble(clipFile), tanAngleUnit);
			double inWeight = asDouble(clipFile);

			//	If this is from a pre-Maya3.0 file, the tangent angles will 
			//	need to be converted.
			//
			if (convertAnglesFromV2To3) {
				convertAnglesAndWeights2To3(type,isWeighted,inAngle,inWeight);
			} else if (convertAnglesFromV3To2) {
				convertAnglesAndWeights3To2(type,isWeighted,inAngle,inWeight);
			}

			//  By default, the tangents are locked. When the tangents
			//	are locked, setting the angle and weight of a fixed in
			//	tangent may change the tangent type of the out tangent.
			//
			animCurve.setTangentsLocked(index, false);
			animCurve.setTangent(index, inAngle, inWeight, true);
		}

		//	Only fixed tangents need additional information.
		//
		if (tanOut == MFnAnimCurve::kTangentFixed) {
			MAngle outAngle(asDouble(clipFile), tanAngleUnit);
			double outWeight = asDouble(clipFile);

			//	If this is from a pre-Maya3.0 file, the tangent angles will 
			//	need to be converted.
			//
			if (convertAnglesFromV2To3) {
				convertAnglesAndWeights2To3(type,isWeighted,outAngle,outWeight);
			} else if (convertAnglesFromV3To2) {
				convertAnglesAndWeights3To2(type,isWeighted,outAngle,outWeight);
			}
			
			//  By default, the tangents are locked. When the tangents
			//	are locked, setting the angle and weight of a fixed out 
			//	tangent may change the tangent type of the in tangent.
			//
			animCurve.setTangentsLocked(index, false);
			animCurve.setTangent(index, outAngle, outWeight, false);
		}

		//	To prevent tangent types from unexpectedly changing, tangent 
		//	locking should be the last operation. See the above comments
		//	about fixed tangent types for more information.
		//
		animCurve.setWeightsLocked(index, swLocked);
		animCurve.setTangentsLocked(index, tLocked);
		animCurve.setIsBreakdown (index, isBreakdown);

		//	There should be no additional data on this line. Go to the
		//	next line of data.
		//
		clipFile.ignore(INT_MAX, kNewLineChar);

		//	Skip any comments.
		//
		advance(clipFile);
		c = clipFile.peek();
	}

	//	Ignore the brace that marks the end of the keys block.
	//
	if (c == kBraceRightChar) {
		clipFile.ignore(INT_MAX, kNewLineChar);
	}

	//	Ignore the brace that marks the end of the animData block.
	//
	advance(clipFile);
	if (clipFile.peek() == kBraceRightChar) {
		clipFile.ignore(INT_MAX, kNewLineChar);
	} else {
		//	Something is wrong.
		//
		MStatus stringStat;
		MString msg = MStringResource::getString(kMissingBrace, stringStat);
		MGlobal::displayError(msg);
	}

	//	Do not set the clipboard with an empty clipboard item.
	//
	if (!animCurveObj.isNull()) 
	{
		if( mImportCustomFrameRange )
		{
			double lFirstFrameToImport = mImportStartFrame < lLowestFrame ? lLowestFrame : mImportStartFrame;
			double lLastFrameToImport  = mImportEndFrame   > lHighestFrame ? lHighestFrame : mImportEndFrame;

			for(int i = 0; i < 2; i++)
			{
				double lCurrentFrame = i == 0 ? lFirstFrameToImport : lLastFrameToImport;

				unsigned int lFrameIndex = 0;
				bool lIsFrameFound = animCurve.find( MTime(lCurrentFrame, inputTimeUnit), lFrameIndex );
				
				//
				// If the keyframe is not found, we need to create a new one
				//
				if( !lIsFrameFound )
				{
					switch ( animCurve.animCurveType() ) 
					{
						case MFnAnimCurve::kAnimCurveTT:
							{
								MTime lTime;
								animCurve.evaluate( MTime(lCurrentFrame, inputTimeUnit), lTime );
								lFrameIndex = animCurve.addKey(	MTime(lCurrentFrame, inputTimeUnit), lTime, MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, NULL, &status);
							}
							break;
						case MFnAnimCurve::kAnimCurveTL:
						case MFnAnimCurve::kAnimCurveTA:
						case MFnAnimCurve::kAnimCurveTU:
							{
								double lValue = animCurve.evaluate( MTime(lCurrentFrame, inputTimeUnit) );
								lFrameIndex = animCurve.addKey(	MTime(lCurrentFrame, inputTimeUnit), lValue, MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, NULL, &status);
							}
							break;
						case MFnAnimCurve::kAnimCurveUL:
						case MFnAnimCurve::kAnimCurveUA:
						case MFnAnimCurve::kAnimCurveUU:
							{
								double lValue = 0;
								animCurve.evaluate( lCurrentFrame, lValue );
								lFrameIndex = animCurve.addKey(	lCurrentFrame, lValue,  MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, NULL, &status);
							}
							break;
						case MFnAnimCurve::kAnimCurveUT:
							{
								MTime lTime;
								animCurve.evaluate( lCurrentFrame, lTime );
								lFrameIndex = animCurve.addKey(	lCurrentFrame, lTime, MFnAnimCurve::kTangentGlobal, MFnAnimCurve::kTangentGlobal, NULL, &status);
							}
							break;
						default:
							MString msg = MStringResource::getString(kUnknownNode, status);
							MGlobal::displayError(msg);
							return false;
					}
				}


				if(i == 0)
				{
					// Remove all frame before the first keyframe
					for(unsigned int i = 0; i < lFrameIndex; i++)
					{
						animCurve.remove(0);
					}
				}
				else 
				{
					// Remove all frame after the last keyframe
					unsigned int lNumKeysToRemove = animCurve.numKeys()-1 - lFrameIndex;
					for(unsigned int i = 0; i < lNumKeysToRemove; i++)
					{
						animCurve.remove(lFrameIndex + 1);
					}
				}
			}
		}


		item.setAnimCurve(animCurveObj);
	}

	//	Delete the copy of the anim curve.
	//
	MGlobal::deleteNode(animCurveObj);

	return true;
}

char *
atomReader::skipToNextParenth(char *dataType, ifstream &readAnim,int parenthCount)
{
	bool firstParenthHit = (parenthCount > 0);
	while(readAnim && ! readAnim.eof() && dataType && (firstParenthHit==false || parenthCount != 0))
	{
		if(strcmp(dataType, "}") == 0)
			--parenthCount;
		else if(strcmp(dataType, "{") == 0)
		{
			firstParenthHit = true;
			++parenthCount; //get a { then add 1 to the count 
		}
		dataType = asWord(readAnim); 
	}
	return dataType;
}


void
atomReader::addDynamicAttributeIfNeeded(MString& nodeName,
										MString& attributeName)
{
	if (attributeName.length() > 0) {
		// check if the attribute exists on the node, and if
		// it doesn't then add it
		//
		MSelectionList list;
		list.add(nodeName);
		if (list.length() == 1) {
			MObject node;
			list.getDependNode(0,node);
			MFnDependencyNode fnNode(node);
			if (!fnNode.hasAttribute(attributeName)) {
				MFnNumericAttribute nAttr;
				MObject dynAdd = nAttr.create(attributeName,attributeName,MFnNumericData::kDouble);
				nAttr.setKeyable(true);
				
				MDGModifier mod;
				mod.addAttribute(node,dynAdd);
				mod.doIt();
			}
		}
	}
}

MStatus 
atomReader::readNodes(char *dataType, ifstream &readAnim,	
	  				    atomLayerClipboard &cb,
						MSelectionList &mList, atomNodeNameReplacer & replacer,
						atomTemplateReader &templateReader,
						bool replaceLayers,  // if true we remove the attribute from all existing anim layers, if false, we don't
						MString &exportEditsFile, //if we find an export edit file, return it here
						bool &removeExportEditsFile) //if the file we found was created us, return this to be true to remove it.
{
	MString nodeName;
	unsigned int depth =0, childCount =0;
	bool nodeIsValid = false;
	atomAnimLayers animLayers;
	while (!readAnim.eof())
	{
		if (NULL == dataType) {
			dataType = asWord(readAnim);
		}
		bool isDag = (strcmp(dataType, kDagNode) == 0);
		bool isDepend = (strcmp(dataType, kDependNode) == 0);
		bool isShape = (strcmp(dataType, kShapeNode) == 0);
		bool isAnimLayer = (strcmp(dataType, kAnimLayer) ==0);
		if (isDag ||isDepend||isShape||isAnimLayer) 
		{
			atomNodeNameReplacer::NodeType  type = atomNodeNameReplacer::eDepend;
			if(isDag)
				type = atomNodeNameReplacer::eDag;
			else if(isShape)
				type = atomNodeNameReplacer::eShape;
			else if(isAnimLayer)
				type = atomNodeNameReplacer::eAnimLayer;
			//depend is the default above
			dataType = asWord(readAnim);
			if (strcmp(dataType, "{") == 0)
			{
				dataType = asWord(readAnim);
				nodeName.set(dataType);
				depth = (unsigned)asDouble(readAnim);
				childCount = (unsigned)asDouble(readAnim);
				//now see if the node is valid if not, then just read to the last parenth,
				//otherwise the returned node name is the name we use
				//first check the template, if not in the template then not valid
				nodeIsValid = replacer.findNode(type,nodeName,depth,childCount);
				if(nodeIsValid)
					nodeIsValid = 	templateReader.findNode(nodeName);
				
				if(nodeIsValid == false)
				{
					dataType = skipToNextParenth(dataType,readAnim);
					continue;
				}
			}
		}
		else if (animLayers.readAnimLayers(readAnim,dataType,*this))
		{
			dataType = NULL;
			continue;
		}
		else if (strcmp(dataType, kAnim) == 0) {
			MString fullAttributeName, leafAttributeName;

			//	If this is from an unconnected anim curve, then there
			//	will not be an attribute name.
			//
			MString layerName;
			if (!isNextNumeric(readAnim)) {
				fullAttributeName.set(asWord(readAnim));

				//	If the node names were specified, then the next two
				//	words should be the leaf attribute and the node name.
				//
				if (!isNextNumeric(readAnim)) {
					leafAttributeName.set(asWord(readAnim));
				}
				
			}

			//attr and node not in template skip
			if(templateReader.findNodeAndAttr(nodeName,leafAttributeName)==false)
			{
				dataType = skipToNextParenth(dataType,readAnim,0); //0 parenth means exit on first set
					continue;

			}
			unsigned  attrCount;
			attrCount = (unsigned)asDouble(readAnim);

			addDynamicAttributeIfNeeded(nodeName,leafAttributeName);
			
			//peek to see if the next character is a ; if not it's a name
			//which means this attribute is the specified animation layer
			char next = readAnim.peek();
			if (next != kSemiColonChar) 
			{
				layerName.set(asWord(readAnim));
				//since this attribute has an anim layer we need to remove all of the previous layers the attribute is on if we
				//are 'replacing' and not 'inserting'. this function automatically makes sure we only remove it once.
				animLayers.removeLayersIfNeeded(replaceLayers, nodeName, leafAttributeName);
				if(atomAnimLayers::isAttrInAnimLayer(nodeName,leafAttributeName,layerName) == false)
					atomAnimLayers::addAttrToAnimLayer(nodeName,leafAttributeName,layerName);
			}

			MAnimCurveClipboardItemArray &clipboardArray = cb.getCBItemArray(layerName);
			//	If the next keyword is not an animData, then this is 
			//	a place holder for the API clipboard.
			//
			dataType = asWord(readAnim);
			if (strcmp(dataType, kAnimData) == 0) {
				MAnimCurveClipboardItem clipboardItem;
				if (readAnimCurve(readAnim, clipboardItem)) {
					clipboardItem.setAddressingInfo(depth, 
													childCount, attrCount);

					//change node name with the one the replacer gave us
					clipboardItem.setNameInfo(	nodeName, 
												fullAttributeName, 
												leafAttributeName);
					clipboardArray.append(clipboardItem);
				} else {
					//	Could not read the anim curve.
					//
					MStatus stringStat;
					MString msg = MStringResource::getString(kCouldNotReadAnim,
															 stringStat);
					MGlobal::displayError(msg);
				}
			} else {
				//	This must be a place holder object for the clipboard.
				//
				MAnimCurveClipboardItem clipboardItem;
				clipboardItem.setAddressingInfo(depth, 
												childCount, attrCount);

				//	Since there is no anim curve specified, what is 
				//	in the fullAttributeName string is really the node 
				//	name.
				// TODO, figure out these placeholders.
				clipboardItem.setNameInfo(	nodeName, 
											nodeName, 
											leafAttributeName);
				clipboardArray.append(clipboardItem);

				//	dataType already contains the next keyword. 
				//	
				continue;
			}
		}
		else if (strcmp(dataType, kCached) == 0)
		{
			dataType = readCachedValues(nodeName,dataType,depth,childCount,readAnim,
				cb,templateReader);
			continue;
		}
		else if (strcmp(dataType, kStatic) == 0)
		{
			dataType = readStaticValue(nodeName,dataType,depth, childCount,readAnim,
				cb,templateReader);
			continue;
		}
		else if (strcmp(dataType, "}")==0)
		{
			nodeIsValid = false;
			dataType = asWord(readAnim); //set up for the next set
			continue;
		}
		else if (strcmp(dataType, kExportEditsDataString) == 0)	{
			//if this is true then we put that data into the file named exportEditsFile
			dataType = readExportEditsFile(dataType, readAnim,exportEditsFile,removeExportEditsFile);
		}
		else
		{
			if (!readAnim.eof() && readAnim.fail() == false) {
				MString warnStr(dataType);
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kUnknownKeyword,
															stat);
				msg.format(msgFmt, warnStr);
				MGlobal::displayError(msg);

				//	Skip to the next line, this one is invalid.
				//
				readAnim.ignore(INT_MAX, kNewLineChar);
			} else {
				//	The end of the file was reached. 
				//
				break;
			}
		}

		//
		dataType = NULL;
	}
	//delete empty anims that we replaced.
	animLayers.deleteEmptyLayers(replaceLayers);

	return (MS::kSuccess);
}


char * atomReader::readCachedValues(MString &nodeName,char *dataType, 
	unsigned int depth, unsigned int childCount,ifstream &readAnim,
	atomLayerClipboard &cb,atomTemplateReader &templateReader)
{
	MString fullAttributeName, leafAttributeName;

	MAnimCurveClipboardItem item;
	MString layerName;
	if (!isNextNumeric(readAnim)) {
		fullAttributeName.set(asWord(readAnim));

		if (!isNextNumeric(readAnim)) {
			leafAttributeName.set(asWord(readAnim));
		}
	}

	//check with template
	if(templateReader.findNodeAndAttr(nodeName,leafAttributeName)==false)
	{
		dataType = skipToNextParenth(dataType,readAnim,0);
		return dataType;
	}

	addDynamicAttributeIfNeeded(nodeName,leafAttributeName);
	
	item.setNameInfo(	nodeName, 
						fullAttributeName, 
						leafAttributeName);
	
	unsigned attrCount;
	attrCount = (unsigned)asDouble(readAnim);
	item.setAddressingInfo(depth, 
												childCount, attrCount);

	//peek to see if the next character is a ; if not it's a name
	//which means this attribute is the specified animation layer
	char next = readAnim.peek();
	if (next != kSemiColonChar) 
	{
		layerName.set(asWord(readAnim));
		if(atomAnimLayers::isAttrInAnimLayer(nodeName,leafAttributeName,layerName) == false)
			atomAnimLayers::addAttrToAnimLayer(nodeName,leafAttributeName,layerName);
	}
	
MAnimCurveClipboardItemArray &clipboardArray = cb.getCBItemArray(layerName);
	
	dataType = asWord(readAnim);
	MString origName(nodeName);
	if (strcmp(dataType, "{") == 0)
	{

		MPlug plug;
		if(getPlug(nodeName,leafAttributeName,plug))
		{
			MFnAnimCurve curve;
			MFnAnimCurve::AnimCurveType type = curve.timedAnimCurveTypeForPlug (plug);
			MObject animCurveObj = curve.create(type, NULL);
			//just use defaults for weighted, set pre/post as constant
			curve.setPreInfinityType(MFnAnimCurve::kConstant);
			curve.setPostInfinityType(MFnAnimCurve::kConstant);

			dataType = putCachedOnAnimCurve(nodeName,leafAttributeName,dataType,readAnim,plug,curve);
			if (!animCurveObj.isNull()) {
				item.setAnimCurve(animCurveObj);
			}
			//	Delete the copy of the anim curve.
			MGlobal::deleteNode(animCurveObj);

			//  Set the item on the clipboard
			clipboardArray.append(item);
			return dataType;
		}
	}

	while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
	{
		dataType = asWord(readAnim); 
	}
	dataType = asWord(readAnim); //set up for the next set
	//if we get here we had an error
	MStatus stringStat;
	MString msgFmt = MStringResource::getString(kCouldNotReadCached,
												stringStat);
	origName += ".";
	origName += fullAttributeName;
	MString msg;
	msg.format(msgFmt,origName);
			
	MGlobal::displayError(msg);

	return dataType;
}

MStatus 
atomReader::readAtom(ifstream &readAnim, atomLayerClipboard &cb,MSelectionList &mList, atomNodeNameReplacer & replacer, 
				MString& exportEditsFile,bool &removeExportEditsFile,atomTemplateReader &templateReader, bool replaceLayers)
//
//	Description:
//		Read  the new updated animation transfer object model file.
//		Now contains 'static' values.           
{

	bool hasVersionString = false;
	double startTime = 1.0;
	double endTime = 0.0;
	double startUnitless = 1.0;
	double endUnitless = 0.0;
	char *dataType = readHeader(readAnim, hasVersionString, startTime,endTime,startUnitless, endUnitless);

	bool exportEditsPresent = false;
	dataType = readExportEditsFilePresent(dataType, readAnim,exportEditsPresent,exportEditsFile);
	if(exportEditsPresent) //if we have an edit file, make sure hierarchy is off.
		replacer.turnOffHierarchy();
	
	//	The atomVersion string is required.
	//

	if (!hasVersionString) {
		MStatus stat;
		MString msg; 
		// Use format to place variable string into message
		MString msgFmt = MStringResource::getString(kMissingKeyword, stat);
		msg.format(msgFmt, kAtomVersion);
		MGlobal::displayError(msg);
		return (MS::kFailure);
	}

	mStartTime = MTime(startTime,timeUnit);
	mEndTime = MTime(endTime,timeUnit);
	mStartUnitless = startUnitless;
	mEndUnitless = endUnitless;
	//	Set the linear and angular units to be the same as the file
	//	being read. This will allow fixed tangent data to be read
	//	in properly if the scene has different units than the .atom file.
	//
	mOldDistanceUnit = MDistance::uiUnit();
	mOldTimeUnit = MTime::uiUnit();

	MDistance::setUIUnit(linearUnit);
	MTime::setUIUnit(timeUnit);

	
	if (MS::kSuccess != readNodes(dataType,readAnim,cb,mList,replacer,templateReader,replaceLayers,exportEditsFile,removeExportEditsFile)) {

		MStatus stringStat;
		MString msg = MStringResource::getString(kCouldNotReadAnim,
												 stringStat);
		MGlobal::displayError(msg);
	}


	return (MS::kSuccess);
}

char*
atomReader::readExportEditsFile(char* dataType,
								ifstream& readAnim,
								MString& filename,
								bool &removeExportEditsFile)
{
	if (!readAnim.eof() && dataType != NULL && strcmp(dataType, kExportEditsDataString) == 0)
	{
		//put a temporary editMA file in the temp directory
		MString melCommand = MString("internalVar -utd") + ";";
		MStatus status = MS::kFailure;
		MString tempFile = MGlobal::executeCommandStringResult (melCommand, false, false, &status);
		//make these values false in case we fail for some reason
		filename = MString("");
		removeExportEditsFile = false;
		if(status == MS::kSuccess && tempFile.length())
		{
			tempFile = tempFile + MString("atomExportTmpFile.editMA");
#if defined (OSMac_)
			char fname[MAXPATHLEN];
			strcpy (fname, tempFile.asChar());
			ofstream editFile(fname);
#else
			ofstream editFile(tempFile.asChar());
#endif
			// if we created that file then put the data from readAnim into the editFile
			if(editFile.good()== true)
			{
				advance(readAnim);
				std::string s;
				while(getline(readAnim, s)) //faster ways to do this but it works since we keep the endlines(\n) needed by the editMA file.
				{
					editFile << s << "\n";
				}
				editFile.flush(); //make sure it's written to disk
				editFile.close();
				dataType = NULL;
				filename = tempFile;
				removeExportEditsFile = true;
			}
		}
	}
	return dataType;
}

char*
atomReader::readExportEditsFilePresent(char* dataType,
								ifstream& readAnim,
								bool &present,
								MString &filename)
{
	if (strcmp(dataType, kExportEditsString) == 0)
	{
		present = true;
		filename = MString("");
		char c = readAnim.peek();
		if (c != kSemiColonChar) //to support vs 0.1 files where the export edits are passed in as files,if the next character isn't a ;
								 //it's a file name so we use and set that. 
		{
			filename.set(asWord(readAnim));
		}
		advance(readAnim);
		dataType = asWord(readAnim);
	} 
	return dataType;
}

char * atomReader::readStaticValue(MString &nodeName,char *dataType, 
			unsigned int depth, unsigned int childCount,ifstream &readAnim,
			atomLayerClipboard &cb,atomTemplateReader &templateReader)
{

	MString fullAttributeName, leafAttributeName;

	MAnimCurveClipboardItem item;
	MString layerName;
	if (!isNextNumeric(readAnim)) {
		fullAttributeName.set(asWord(readAnim));

		if (!isNextNumeric(readAnim)) {
			leafAttributeName.set(asWord(readAnim));
		}
	}

		//check with template
	if(templateReader.findNodeAndAttr(nodeName,leafAttributeName)==false)
	{
		dataType = skipToNextParenth(dataType,readAnim,0);
		return dataType;
	}

	addDynamicAttributeIfNeeded(nodeName,leafAttributeName);
			
	item.setNameInfo(	nodeName, 
						fullAttributeName, 
						leafAttributeName);

	unsigned attrCount;
	attrCount = (unsigned)asDouble(readAnim);
	item.setAddressingInfo(depth, 
												childCount, attrCount);

	//peek to see if the next character is a ; if not it's a name
	//which means this attribute is the specified animation layer
	char next = readAnim.peek();
	if (next != kSemiColonChar) 
	{
		layerName.set(asWord(readAnim));
		if(atomAnimLayers::isAttrInAnimLayer(nodeName,leafAttributeName,layerName) == false)
			atomAnimLayers::addAttrToAnimLayer(nodeName,leafAttributeName,layerName);
	}
	MAnimCurveClipboardItemArray &clipboardArray = cb.getCBItemArray(layerName);

	dataType = asWord(readAnim);
	MString origName(nodeName);
	if (strcmp(dataType, "{") == 0)
	{
		MPlug plug;
		if(getPlug(nodeName,leafAttributeName,plug))
		{
			//if there is a curve we put the static value as a key using the cipboard
			//which handles the correct insert/replace flags
			MStatus status = MStatus::kSuccess;
			MFnAnimCurve curve(plug,&status);
			MFnAnimCurve::AnimCurveType type = curve.animCurveType();
			if((status== MStatus::kSuccess) && (
				(type == MFnAnimCurve::kAnimCurveTA) ||
				(type == MFnAnimCurve::kAnimCurveTL) ||
				(type == MFnAnimCurve::kAnimCurveTT) ||
				(type == MFnAnimCurve::kAnimCurveTU)))
			{
				MFnAnimCurve newCurve;
				MObject animCurveObj = newCurve.create(type, NULL);
				//just use defaults for weighted, set pre/post as constant
				curve.setPreInfinityType(MFnAnimCurve::kConstant);
				curve.setPostInfinityType(MFnAnimCurve::kConstant);
				dataType = putCachedOnAnimCurve(nodeName,leafAttributeName, dataType,readAnim,plug,newCurve);
				if (!animCurveObj.isNull()) {
					item.setAnimCurve(animCurveObj);
				}
				//	Delete the copy of the anim curve.
				MGlobal::deleteNode(animCurveObj);

				//  Set the item on the clipboard
				clipboardArray.append(item);
				return dataType;
			}
			else//no curve was present so we just set the value on theh plug directly
			{
				char c = readAnim.peek();
				if(readAnim && c != kBraceRightChar)
				{
					MObject attribute = plug.attribute();

					if( attribute.hasFn( MFn::kUnitAttribute ) )
					{
						MFnUnitAttribute fnAttrib(attribute);
						switch(fnAttrib.unitType())
						{
						case MFnUnitAttribute::kAngle:
						{
							double val = asDouble(readAnim);
							MAngle unit(1.0);
							val /= unit.as(angularUnit);
							MAngle angle(val);
							plug.setMAngle(angle);
							break;
						}
						case MFnUnitAttribute::kDistance:
						{
							double val = asDouble(readAnim);
							MDistance unit(1.0);
							val /= unit.as(linearUnit);
							MDistance dist(val);
							plug.setMDistance(dist);
							break;
						}
						case MFnUnitAttribute::kTime:
						{
							double val = asDouble(readAnim);
							MTime t(val);
							plug.setMTime(t);
							break;
						}
						default:
							break;
						}
					}
					else if ( attribute.hasFn( MFn::kNumericAttribute ) )
					{
						MFnNumericAttribute fnAttrib(attribute);
			
						switch(fnAttrib.unitType())
						{
						case MFnNumericData::kBoolean:
						{
							int val = asInt(readAnim);
							bool value = (val ==0) ? false : true;
							plug.setBool(value);
							break;
						}
						case MFnNumericData::kByte:
						case MFnNumericData::kChar:
						{
							char val = asChar(readAnim);
							plug.setChar(val);
							break;
						}
						case MFnNumericData::kShort:
						{
							short val = asShort(readAnim);
							plug.setShort(val);
							break;
						}
						case MFnNumericData::kLong:
						{
							int val = asInt(readAnim);
							plug.setInt(val);
							break;
						}
						case MFnNumericData::kFloat:
						{
							double val = asDouble(readAnim);
							plug.setFloat((float)val);
							break;
						}
						case MFnNumericData::kDouble:
						{
							double val = asDouble(readAnim);
							plug.setDouble(val);
							break;
						}
						//dont handle float3 since it's handled as 3 float plugs
						default:
							break;
						}
					}
					else if( attribute.hasFn( MFn::kEnumAttribute ) )
					{
						short val = asShort(readAnim);
						plug.setShort(val);
					}	
				}
				dataType = asWord(readAnim); //this should be the } bracket
				while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
				{
					dataType = asWord(readAnim); //set up for the next set
				}
			}
			dataType = asWord(readAnim); //set up for the next set
			
		}
		else //can't find the plug
		{
			while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
			{
				dataType = asWord(readAnim); 
			}
			dataType = asWord(readAnim); //set up for the next set
		}
		return dataType;
	}
	//if we get here we had an error
	MStatus stringStat;
	MString msgFmt = MStringResource::getString(kCouldNotReadStatic,
												stringStat);
	origName += ".";
	origName += fullAttributeName;
	MString msg;
	msg.format(msgFmt,origName);
			
	MGlobal::displayError(msg);

	return dataType;

}

char * atomReader::putCachedOnAnimCurve(MString &nodeName,MString &fullAttributeName,
	char *dataType, ifstream &readAnim, MPlug &plug, MFnAnimCurve &curve)
{
	MObject attribute = plug.attribute();
	char c = readAnim.peek();
	if(readAnim && c != kBraceRightChar)
	{
		MFnAnimCurve::AnimCurveType type = curve.animCurveType();
		if((type == MFnAnimCurve::kAnimCurveTA) ||
				(type == MFnAnimCurve::kAnimCurveTL) ||
				(type == MFnAnimCurve::kAnimCurveTT) ||
				(type == MFnAnimCurve::kAnimCurveTU) 
				)
		{
			MTime currentTime = mStartTime;
			MTime stepTime(1,timeUnit);
			if( attribute.hasFn( MFn::kUnitAttribute ) )
			{
				MFnUnitAttribute fnAttrib(attribute);
				switch(fnAttrib.unitType())
				{
				case MFnUnitAttribute::kAngle:
				{
					MAngle unit(1.0);
					double conv = 1.0/unit.as(angularUnit);
					while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
					{
						if(isNextNumeric(readAnim))
						{
							double val = asDouble(readAnim);
							val *= conv;
							MAngle angle(val);
							curve.addKeyframe(currentTime,val);
							currentTime += stepTime;
						}
						else
						{
							dataType = asWord(readAnim); //this should be the } bracket
						}
					}
					dataType = asWord(readAnim); //set up for the next set
					return dataType;
				}
				case MFnUnitAttribute::kDistance:
				{
					MDistance unit(1.0);
					double conv = 1.0/unit.as(linearUnit);
					while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
					{
						if(isNextNumeric(readAnim))
						{
							double val = asDouble(readAnim);
							val *= conv;
							MDistance dist(val);
							curve.addKeyframe(currentTime,val);
							currentTime += stepTime;
						}
						else
						{
							dataType = asWord(readAnim); //this should be the } bracket
						}
					}
					dataType = asWord(readAnim); //set up for the next set
					return dataType;

				}
				case MFnUnitAttribute::kTime:
				{
					while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
					{
						if(isNextNumeric(readAnim))
						{
							double val = asDouble(readAnim);
							MTime t(val,timeUnit);
							curve.addKey(currentTime,t);
							currentTime += stepTime;
						}
						else
						{
							dataType = asWord(readAnim); //this should be the } bracket
						}
					}
					dataType = asWord(readAnim); //set up for the next set
					return dataType;
				}
				default:
					break;
				}
			}
			else if ( attribute.hasFn( MFn::kNumericAttribute ) )
			{
				MFnNumericAttribute fnAttrib(attribute);
			
				switch(fnAttrib.unitType())
				{
					case MFnNumericData::kByte:
					case MFnNumericData::kChar:
					case MFnNumericData::kBoolean:
					case MFnNumericData::kShort:
					case MFnNumericData::kLong:
					case MFnNumericData::kFloat:
					case MFnNumericData::kDouble:
					{
						
						while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
						{
							if(isNextNumeric(readAnim))
							{
								double val = asDouble(readAnim);
								curve.addKeyframe(currentTime,val);
								currentTime += stepTime;
							}
							else
							{
								dataType = asWord(readAnim); //this should be the } bracket
							}
						}
						dataType = asWord(readAnim); //set up for the next set
						return dataType;
					}
					default:
						break;
				}
			}
			else if( attribute.hasFn( MFn::kEnumAttribute ) )
			{
				//just read as a double
				while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
				{
					if(isNextNumeric(readAnim))
					{
						double val = asDouble(readAnim);
						curve.addKeyframe(currentTime,val);
						currentTime += stepTime;
					}
					else
					{
						dataType = asWord(readAnim); //this should be the } bracket
					}
				}
				dataType = asWord(readAnim); //set up for the next set
				return dataType;	
			}	
		}
	}

	while(readAnim && ! readAnim.eof() && dataType && strcmp(dataType, "}") !=0)
	{
		dataType = asWord(readAnim); 
	}
	dataType = asWord(readAnim); //set up for the next set
	//if we get here we had an error
	MStatus stringStat;
	MString msgFmt = MStringResource::getString(kCouldNotReadCached,
												stringStat);
	MString origName(nodeName);
	origName += ".";
	origName += fullAttributeName;
	MString msg;
	msg.format(msgFmt,origName);
	MGlobal::displayError(msg);

	return dataType;
}

char * 
atomReader::readHeader(ifstream &readAnim, bool &hasVersionString,double &startTime, double &endTime, double &startUnitless, double &endUnitless)
//
//	Description:
//	Read the header.
//  Returns: Will return NULL or the current read character after the header is read.
//  Even if NULL the file might not be at the end, so ehck eof() if needed.
{
	

	resetUnits();
	convertAnglesFromV2To3 = false;
	convertAnglesFromV3To2 = false;

	//	Read the header. The header officially ends when the first non-header
	//	keyword is found. The header contains clipboard specific information
	//	where the body is anim curve specific.
	//
	char *dataType = NULL;
	while (!readAnim.eof()) {
		advance(readAnim);
		dataType = asWord(readAnim);

		if (strcmp(dataType, kAtomVersion) == 0) {
			MString version(asWord(readAnim));
			animVersion = version.asDouble();
			MString thisVersion(kAtomVersionString);

			hasVersionString = true;

			//	Add versioning control here.
			//
			if (version != thisVersion) {
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kInvalidVersion,
															stat);
				msg.format(msgFmt, version, thisVersion);
				MGlobal::displayWarning(msg);
			} 
		} else if (strcmp(dataType, kMayaVersion) == 0) {
			MString version(asWord(readAnim, true));

			MString currentVersion = MGlobal::mayaVersion();
			if (currentVersion.substring(0,1) == "2.") {
				MString vCheck = version.substring(0, 1);
				if (vCheck != "2.") {
					convertAnglesFromV3To2 = true;
				}
			} else {
				//	If this is a pre-Maya 3.0 file, then the tangent angles 
				//	will need to be converted to work in Maya 3.0+
				//
				MString vCheck = version.substring(0, 1);
				if (vCheck == "2.") {
					convertAnglesFromV2To3 = true;
				}
			}
		} else if (strcmp(dataType, kTimeUnit) == 0) {
			MString timeUnitString(asWord(readAnim));
			if (!atomUnitNames::setFromName(timeUnitString, timeUnit)) {
				MString unitName;
				timeUnit = MTime::uiUnit();
				atomUnitNames::setToShortName(timeUnit, unitName);
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kSettingToUnit,
															stat);
				msg.format(msgFmt, kTimeUnit, unitName);
				MGlobal::displayWarning(msg);
			}
		} else if (strcmp(dataType, kLinearUnit) == 0) {
			MString linearUnitString(asWord(readAnim));
			if (!atomUnitNames::setFromName(linearUnitString, linearUnit)) {
				MString unitName;
				linearUnit = MDistance::uiUnit();
				atomUnitNames::setToShortName(linearUnit, unitName);
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kSettingToUnit,
															stat);
				msg.format(msgFmt, kLinearUnit, unitName);
				MGlobal::displayWarning(msg);
			}
		} else if (strcmp(dataType, kAngularUnit) == 0) {
			MString angularUnitString(asWord(readAnim));
			if (!atomUnitNames::setFromName(angularUnitString, angularUnit)) {
				MString unitName;
				angularUnit = MAngle::uiUnit();
				atomUnitNames::setToShortName(angularUnit, unitName);
				MStatus stat;
				MString msg; 
				// Use format to place variable string into message
				MString msgFmt = MStringResource::getString(kSettingToUnit,
															stat);
				msg.format(msgFmt, kAngularUnit, unitName);
				MGlobal::displayWarning(msg);
			}
		} else if (strcmp(dataType, kStartTime) == 0) {
			startTime = asDouble(readAnim);
		} else if (strcmp(dataType, kEndTime) == 0) {
			endTime = asDouble(readAnim);
		} else if (strcmp(dataType, kStartUnitless) == 0) {
			startUnitless = asDouble(readAnim);
		} else if (strcmp(dataType, kEndUnitless) == 0) {
			endUnitless = asDouble(readAnim);
		} 
		else if(strcmp(dataType,kMayaSceneFile) ==0){
			//just make sure we read this;
			//and skip whitespace
			asWord(readAnim, true);
		}
		else {	
			//	The header should be finished. Begin to parse the body.
			//
			break;
		}
	}
	return dataType;

}
//-------------------------------------------------------------------------
//	Class atomWriter
//-------------------------------------------------------------------------

atomWriter::atomWriter()
//
//	Description:
//		Class constructor.
//
{
}

atomWriter::~atomWriter()
//
//	Description:
//		Class destructor.
//
{
}

MStatus 
atomWriter::writeClipboard(	ofstream& animFile, 
							const MAnimCurveClipboard &cb,
							atomCachedPlugs *cachedPlugs,
							const MString &layerName)
//
//	Description:
//		Write the contents of the clipboard to the ofstream.
//
{
	MStatus status = MS::kFailure;

	// Check to see if there is anything on the clipboard at all
	//
	if (cb.isEmpty()) {
		return status;
	}
	
	// Now write out each animCurve
	//
	const MAnimCurveClipboardItemArray 	&clipboardArray = 
										cb.clipboardItems(&status);
	if (MS::kSuccess != status) {
		return status;
	}

	for (unsigned int i = 0; i < clipboardArray.length(); i++) {
		const MAnimCurveClipboardItem &clipboardItem = clipboardArray[i];

		MStatus statusInLoop = MS::kSuccess;
		const MObject animCurveObj = clipboardItem.animCurve(&statusInLoop);

		//to check with caching we need the name of the plug, not just the attrname
		MPlug plug;
		if(getPlug(clipboardItem.nodeName(),clipboardItem.fullAttributeName(), plug)==false)
			continue;
		MString attrName;
		getAttrName(plug,attrName);
		//if we are caching don't output the curve for a cached plug.
		if(cachedPlugs!=NULL && cachedPlugs->isAttrCached(attrName,layerName))
			continue;

		//	The clipboard may contain Null anim curves. If a Null anim 
		//	is hit we skip it.
		if (MS::kSuccess != statusInLoop || animCurveObj.isNull()) {
			continue;
		}

		// Write out animCurve information
		//
		if (!writeAnim(animFile, clipboardItem,layerName)) {
			return (MS::kFailure);
		}


		//	Write out each curve in its specified format.
		//	For now, only the anim curve format.
		//
		if (!writeAnimCurve(animFile, &animCurveObj, 
							clipboardItem.animCurveType())) {
			return (MS::kFailure);
		}
	}

	return (MS::kSuccess);
}


void atomWriter::writeStaticValues(	ofstream &animFile,
	const MPlugArray &animatablePlugs,
	std::set<std::string> &attrStrings,
	const MString &nodeName,
	unsigned int depth,
	unsigned int childCount,
	atomTemplateReader &templateReader
)
{
	// Walk through each animtable plug and write out the static value on that plug
	// If the plug is animated (or TODO is a constraint or set driven key), or
	// is not found in the attrString set and that set isn't empty
	// Then don't write it.
	std::set<std::string>::const_iterator constIter = attrStrings.end();
	unsigned int numPlugs = animatablePlugs.length();
	for (unsigned int i = 0; i < numPlugs; i++)
	{
		MPlugArray dstPlugArray;
		MPlug plug = animatablePlugs[i];
		//if a plug isn't connected then it's static, or if it's not getting driven
		//later on with cached values we may revert them to a static if the plug
		//is connected but the value doesn't change over the time range.
		bool connected = plug.connectedTo (dstPlugArray, true,false);
		if (connected == false || dstPlugArray.length() ==0) 
		{
			
			MPlug attrPlug (plug);
			MObject attrObj = attrPlug.attribute();
			MFnAttribute fnLeafAttr (attrObj);
			MString attrName;
			getAttrName(plug,attrName);
			//template filter check first
			if(templateReader.findNodeAndAttr(nodeName,attrName) == false)
				continue; 
			//and if we have specified attr strings then don't save it out if it's not found
			//must use shortName since the channelBox command will always return a short name
			//the long name flag there is only used to turn long name (or nice name) display on
			if(attrStrings.size() == 0 || attrStrings.find(std::string(fnLeafAttr.shortName().asChar())) != constIter)
			{
				// Write out the plugs' static statement
				animFile <<kTwoSpace << "static ";
				// build up the full attribute name
				MFnAttribute fnAttr (attrObj);
				MString fullAttrName (fnLeafAttr.shortName());
				attrPlug = attrPlug.parent();
				while (!attrPlug.isNull()) {
					attrObj = attrPlug.attribute();
					MFnAttribute fnAttr2 (attrObj);
					fullAttrName = fnAttr2.name() + "." + fullAttrName;
					attrPlug = attrPlug.parent();
				}
				animFile << fullAttrName.asChar() << " " << attrName.asChar() << " " << i << ";\n";
				MDGContext context = MDGContext::fsNormal;
				animFile <<kTwoSpace << "{ ";
				writeValue(animFile,plug,context);
				animFile << " }\n";
			}
		}
	}
}


void atomWriter::writeCachedValues(	ofstream &animFile,
	atomCachedPlugs *cachedPlugs,
	std::set<std::string> &attrStrings,
	const MString &nodeName,
	unsigned int depth,
	unsigned int childCount,
	atomTemplateReader &templateReader
)
{
	if(cachedPlugs)
	{
		std::set<std::string>::const_iterator constIter = attrStrings.end();
		unsigned int numPlugs = cachedPlugs->getNumPlugs();
		for (unsigned int i = 0; i < numPlugs; i++)
		{
			MPlug &plug = cachedPlugs->getPlug(i);
			MPlug attrPlug (plug);
			MObject attrObj = attrPlug.attribute();
			MFnAttribute fnLeafAttr (attrObj);
			MString attrName;
			getAttrName(plug,attrName);
			//template filter check first
			if(templateReader.findNodeAndAttr(nodeName,fnLeafAttr.name()) == false)
				continue; 
			//then if we have specified attr strings then don't save it out if it's not found
			//must use shortName since the channelBox command will always return a short name
			//the long name flag there is only used to turn long name (or nice name) display on
			if(attrStrings.size() == 0 || attrStrings.find(std::string(fnLeafAttr.shortName().asChar())) != constIter)
			{
				// Write out the plugs' cached statement
				animFile <<kTwoSpace << "cached ";
				// build up the full attribute name
				MFnAttribute fnAttr (attrObj);
				MString fullAttrName (fnLeafAttr.shortName());
				attrPlug = attrPlug.parent();
				while (!attrPlug.isNull()) {
					attrObj = attrPlug.attribute();
					MFnAttribute fnAttr2 (attrObj);
					fullAttrName = fnAttr2.name() + "." + fullAttrName;
					attrPlug = attrPlug.parent();
				}
				animFile << fullAttrName.asChar() << " " << attrName.asChar() << " " << i << ";\n";
				animFile <<kTwoSpace << "{ ";
				cachedPlugs->writeValues(animFile,i);
				animFile << " }\n";
			}
		}
	}
}

void	
atomWriter::writeNodeStart(ofstream& animFile,atomNodeNameReplacer::NodeType nodeType, MString &nodeName, unsigned int depth, unsigned int childCount)
{
	if(nodeType == atomNodeNameReplacer::eDag)
		animFile << kDagNode;
	else if(nodeType == atomNodeNameReplacer::eShape)
		animFile << kShapeNode;
	else if(nodeType == atomNodeNameReplacer::eAnimLayer)
		animFile << kAnimLayer;
	else
		animFile << kDependNode;
	animFile << " {\n";
	animFile  << kTwoSpace << nodeName.asChar() << " " << depth << " " << childCount << ";\n";

}
void	
atomWriter::writeNodeEnd(ofstream& animFile)
{
	animFile << "}\n";
}


bool atomWriter::writeHeader(ofstream& clip,bool useSpecifiedRange, MTime &defaultStartTime, MTime &defaultEndTime)
//
//	Description:
//		Writes the header for the file. The header contains the clipboard
//		specific data. 
//
{
	if (!clip) {
		return false;
	}

	resetUnits(); //reset the units first.

	//always write out the version info and the units in case we only have statics in the file and no curves.
	clip << kAtomVersion << kSpaceChar << kAtomVersionString << kSemiColonChar << endl;
	clip << kMayaVersion << kSpaceChar << MGlobal::mayaVersion() << kSemiColonChar << endl;
	MString melCommand = MString("file -q -sn") + ";";
	MStatus status = MS::kFailure;
	MString sceneName = MGlobal::executeCommandStringResult (melCommand, false, false, &status);
	if(status == MS::kSuccess && sceneName.length() >0) //if no name then don't save it
		clip << kMayaSceneFile << kSpaceChar << sceneName << kSemiColonChar << endl;
	MString unit;
	atomUnitNames::setToShortName(timeUnit, unit);
	clip << kTimeUnit << kSpaceChar << unit << kSemiColonChar << endl;
	atomUnitNames::setToShortName(linearUnit, unit);
	clip << kLinearUnit << kSpaceChar << unit << kSemiColonChar << endl;
	atomUnitNames::setToShortName(angularUnit, unit);
	clip << kAngularUnit << kSpaceChar << unit << kSemiColonChar << endl;


	static MAnimCurveClipboard &clipboard = 
	MAnimCurveClipboard::theAPIClipboard();

	//always use the default times since we may also have cached times too


	if(useSpecifiedRange == true)
	{
		double startTime = defaultStartTime.as(timeUnit);
		double endTime = defaultEndTime.as(timeUnit);

		mStartTime = MTime(startTime,timeUnit);
		mEndTime = MTime(endTime,timeUnit);
		clip << kStartTime << kSpaceChar << startTime << kSemiColonChar << endl;
		clip << kEndTime << kSpaceChar << endTime << kSemiColonChar << endl;
		if(clipboard.isEmpty() == false) //we have curves so write out the unitless min/max
		{

			float startUnitless = clipboard.startUnitlessInput();
			float endUnitless = clipboard.endUnitlessInput();
			if (startUnitless != endUnitless) {
				clip << kStartUnitless << kSpaceChar << startUnitless << 
						kSemiColonChar << endl;
				clip << kEndUnitless << kSpaceChar << endUnitless << 
						kSemiColonChar << endl;
			}
		}
	}
	else
	{
		if(clipboard.isEmpty() == false) //we have curves so write them out
		{
		
			double startTime = clipboard.startTime().as(timeUnit);
			double endTime = clipboard.endTime().as(timeUnit);

			mStartTime = MTime(startTime,timeUnit);
			mEndTime = MTime(endTime,timeUnit);
			clip << kStartTime << kSpaceChar << startTime << kSemiColonChar << endl;
			clip << kEndTime << kSpaceChar << endTime << kSemiColonChar << endl;

			float startUnitless = clipboard.startUnitlessInput();
			float endUnitless = clipboard.endUnitlessInput();
			
			if (startUnitless != endUnitless) {
				clip << kStartUnitless << kSpaceChar << startUnitless << 
						kSemiColonChar << endl;
				clip << kEndUnitless << kSpaceChar << endUnitless << 
						kSemiColonChar << endl;
			}
			

		}
		else //no anim data so set the default times to the playback options
		{
			double startTime =  MAnimControl::animationStartTime().as(timeUnit);
			double endTime = MAnimControl::animationEndTime().as(timeUnit);
			mStartTime = MTime(startTime,timeUnit);
			mEndTime = MTime(endTime,timeUnit);

			clip << kStartTime << kSpaceChar << startTime << kSemiColonChar << endl;
			clip << kEndTime << kSpaceChar << endTime << kSemiColonChar << endl;
		}
	}
	defaultStartTime = mStartTime;
	defaultEndTime = mEndTime;
	return true;
}

bool atomWriter::writeExportEditsFilePresent(ofstream& clip)
//
//	Description:
//		Says that an export edits file will be present embedded at the end of the file
//
{
	clip << kExportEditsString << kSpaceChar  << kSemiColonChar << endl;
	return true;
}

bool atomWriter::writeExportEditsFile(ofstream &clip, const MString& filename)
//
//	Description:
//		Writes the exportEdits (offline) file embedded in the atom file.
//
{
	clip << kExportEditsDataString << kSpaceChar;

	std::string line;
	std::ifstream exportFile (filename.asChar());
	if (exportFile.is_open())
	{
		while ( exportFile.good() )
		{
			getline (exportFile,line);
			clip << line <<endl;
		}
		exportFile.close();
		clip  << kSemiColonChar << endl;
		//remove the file
		remove(filename.asChar());
	}
	return true;

}

bool atomWriter::writeAnim(	ofstream &clip, 
							const MAnimCurveClipboardItem &clipboardItem,
							const MString &layerName)
//	
//	Description:
//		Write out the anim curve from the clipboard item into the 
//		ofstream. The position of the anim curve in the clipboard
//		and the attribute to which it is attached is written out in this
//		method.
//
//		This method returns true if the write was successful.
//
{
	if (!clip) {
		return false;
	}

	clip << kTwoSpace <<kAnim;

	//	If this is a clipboard place holder then there will be no full
	//	or leaf attribute names.
	
	//want multi[index].leafAttr not just leafAttr here
	MPlug plug;
	if(getPlug(clipboardItem.nodeName(),clipboardItem.fullAttributeName(), plug)==false)
		return false;
	MString attrName;
	getAttrName(plug,attrName);

	clip << kSpaceChar << clipboardItem.fullAttributeName().asChar();
	clip << kSpaceChar << attrName.asChar();

	unsigned int rowCount, childCount, attrCount;
	clipboardItem.getAddressingInfo(rowCount, childCount, attrCount);

	clip << kSpaceChar << attrCount;
	if(layerName.length())
		clip << kSpaceChar << layerName.asChar();
	clip << kSemiColonChar << endl;

	return true;
}

bool atomWriter::writeAnimCurve(ofstream &clip, 
								const MObject *animCurveObj,
								MFnAnimCurve::AnimCurveType type,
								bool verboseUnits)
//
//	Description:
//		Write out the anim curve from the clipboard item into the
//		ofstream. The actual anim curve data is written out.
//
//		This method returns true if the write was successful.
//
{
	if (NULL == animCurveObj || animCurveObj->isNull() || !clip) {
		return true;
	}

	MStatus status = MS::kSuccess;
	MFnAnimCurve animCurve(*animCurveObj, &status);
	if (MS::kSuccess != status) {
		MString msg = MStringResource::getString(kCouldNotExport, status);
		MGlobal::displayError(msg);
		return false;
	}

	clip << kTwoSpace<< kAnimData << kSpaceChar << kBraceLeftChar << endl;

	clip << kFourSpace << kInputString << kSpaceChar <<
			boolInputTypeAsWord(animCurve.isUnitlessInput()) << 
			kSemiColonChar << endl;

	clip << kFourSpace << kOutputString << kSpaceChar <<
			outputTypeAsWord(type) << kSemiColonChar << endl;

	clip << kFourSpace << kWeightedString << kSpaceChar <<
			(animCurve.isWeighted() ? 1 : 0) << kSemiColonChar << endl;

	//	These units default to the units in the header of the file.
	//	
	if (verboseUnits) {
		clip << kFourSpace << kInputUnitString << kSpaceChar;
		if (animCurve.isTimeInput()) {
			MString unitName;
			atomUnitNames::setToShortName(timeUnit, unitName);
			clip << unitName;
		} else {
			//	The anim curve has unitless input.
			//
			clip << kUnitlessString;
		}
		clip << kSemiColonChar << endl;

		clip << kFourSpace << kOutputUnitString << kSpaceChar;
	}

	double conversion = 1.0;
	MString unitName;
	switch (type) {
		case MFnAnimCurve::kAnimCurveTA:
		case MFnAnimCurve::kAnimCurveUA:
			atomUnitNames::setToShortName(angularUnit, unitName);
			if (verboseUnits) clip << unitName;
			{
				MAngle angle(1.0);
				conversion = angle.as(angularUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTL:
		case MFnAnimCurve::kAnimCurveUL:
			atomUnitNames::setToShortName(linearUnit, unitName);
			if (verboseUnits) clip << unitName;
			{
				MDistance distance(1.0);
				conversion = distance.as(linearUnit);
			}
			break;
		case MFnAnimCurve::kAnimCurveTT:
		case MFnAnimCurve::kAnimCurveUT:
			atomUnitNames::setToShortName(timeUnit, unitName);
			if (verboseUnits) clip << unitName;
			break;
		default:
			if (verboseUnits) clip << kUnitlessString;
			break;
	}
	if (verboseUnits) clip << kSemiColonChar << endl;

	if (verboseUnits) {
		MString angleUnitName;
		atomUnitNames::setToShortName(angularUnit, angleUnitName);
		clip << kFourSpace << kTanAngleUnitString << 
				kSpaceChar << angleUnitName << kSemiColonChar << endl;
	}

	clip << kFourSpace << kPreInfinityString << kSpaceChar <<
			infinityTypeAsWord(animCurve.preInfinityType()) << 
			kSemiColonChar << endl;

	clip << kFourSpace << kPostInfinityString << kSpaceChar <<
			infinityTypeAsWord(animCurve.postInfinityType()) << 
			kSemiColonChar << endl;

	clip << kFourSpace << kKeysString << kSpaceChar << kBraceLeftChar << endl;

	// And then write out each keyframe
	//
	unsigned numKeys = animCurve.numKeyframes();
	for (unsigned i = 0; i < numKeys; i++) {
		clip << kFourSpace << kTwoSpace;
		if (animCurve.isUnitlessInput()) {
			clip << animCurve.unitlessInput(i);
		}
		else {
			clip << animCurve.time(i).value();
		}

		// clamp tiny values so that it isn't so small it can't be read in
		//
		double animValue = (conversion*animCurve.value(i));
		if (atomBase::isEquivalent(animValue,0.0)) animValue = 0.0;
		clip << kSpaceChar << animValue;

		clip << kSpaceChar << tangentTypeAsWord(animCurve.inTangentType(i));
		clip << kSpaceChar << tangentTypeAsWord(animCurve.outTangentType(i));

		clip << kSpaceChar << (animCurve.tangentsLocked(i) ? 1 : 0);
		clip << kSpaceChar << (animCurve.weightsLocked(i) ? 1 : 0);
		clip << kSpaceChar << (animCurve.isBreakdown(i) ? 1 : 0);

		if (animCurve.inTangentType(i) == MFnAnimCurve::kTangentFixed) {
			MAngle angle;
			double weight;
			animCurve.getTangent(i, angle, weight, true);

			clip << kSpaceChar << angle.as(angularUnit);
			clip << kSpaceChar << weight;
		}
		if (animCurve.outTangentType(i) == MFnAnimCurve::kTangentFixed) {
			MAngle angle;
			double weight;
			animCurve.getTangent(i, angle, weight, false);

			clip << kSpaceChar << angle.as(angularUnit);
			clip << kSpaceChar << weight;
		}

		clip << kSemiColonChar << endl;
	}
	clip << kFourSpace << kBraceRightChar << endl;

	clip << kTwoSpace << kBraceRightChar << endl;

	return true;
}

void atomWriter::writeValue(ofstream & clip,MPlug &plug,MDGContext &context)
{
	MObject attribute = plug.attribute();

	if ( attribute.hasFn( MFn::kNumericAttribute ) )
	{
		MFnNumericAttribute fnAttrib(attribute);

		switch(fnAttrib.unitType())
		{
			case MFnNumericData::kBoolean:
			{
				bool value;
				plug.getValue(value,context);
				int ival = value == true ? 1 : 0;
				clip << ival;
				break;
			}
			case MFnNumericData::kByte:
			case MFnNumericData::kChar:
			{
				char value;
				plug.getValue(value,context);
				clip << value;
				break;
			}
			case MFnNumericData::kShort:
			{
				short value;
				plug.getValue(value,context);
				clip << value;
				break;
			}
			case MFnNumericData::kLong:
			{
				int value;
				plug.getValue(value,context);
				clip << value;
				break;
			}
			case MFnNumericData::kFloat:
			{
				float value;
				plug.getValue(value,context);
				clip << value;
				break;
			}
			case MFnNumericData::kDouble:
			{
				double value;
				plug.getValue(value,context);
				clip << value;
				break;
			}
			case MFnNumericData::k3Float:
			{
				MVector float3;
				plug.child(0).getValue(float3.x,context);
				plug.child(1).getValue(float3.y,context);
				plug.child(2).getValue(float3.z,context);
				clip << float3.x <<kSpaceChar<< float3.y << kSpaceChar<< float3.z;
				break;
			}
			default:
				break;
		}
	}
	else if( attribute.hasFn( MFn::kUnitAttribute ) )
	{
		MFnUnitAttribute fnAttrib(attribute);
		switch(fnAttrib.unitType())
		{
			case MFnUnitAttribute::kAngle:
			{
				double value;
				plug.getValue(value,context);
				MAngle angle(1.0);
				value *= angle.as(angularUnit);
				clip << value;
				break;
			}
			case MFnUnitAttribute::kDistance:
			{
				double value;
				plug.getValue(value,context);
				MDistance distance(1.0);
				value *= distance.as(linearUnit);
				clip << value;
				break;
			}
			case MFnUnitAttribute::kTime:
			{
				double value;
				plug.getValue(value,context);
				clip << value;
				break;
			}
			default:
			{
				break;
			}
		}
	}
	else if( attribute.hasFn( MFn::kEnumAttribute ) )
	{
		short value;
		plug.getValue(value,context);
		clip << value;
	}
}

//this function returns a sorted list of items that we should save or import onto
//The problem is that we don't know the order of the objects returned by
//MGlobal::getActiveSelectionList, we also may need to include children, and we need to remove any
//shape nodes
//So we go through that selection list, add children, remove shapes, and save out a 
//std::set of all of the dag node names we find.
//then we do a depth sort of the scene and if that dag node is in the set,
//we add it to the list. That way we get the dag orders always sorted in the same
//way no matter if we load or save. The depend ndoes
void SelectionGetter::getSelectedObjects(bool includeChildren,MSelectionList &list, std::vector<unsigned int> &depths)
{

	MGlobal::getActiveSelectionList(list);
		
	std::set<std::string> nodeNames;

	MDagPath path;
	unsigned int numObjects = list.length();
	std::set<std::string>::const_iterator constIter;
	for (int i = numObjects -1; i > -1; --i) 
	{
		if (list.getDagPath (i, path) == MS::kSuccess)
		{
			MObject object = path.node();
			std::string str(path.fullPathName().asChar());
			constIter = nodeNames.end();
			if(nodeNames.find(str)== constIter)
				nodeNames.insert(str);
			if(includeChildren) //add children, if not in the nodenames already
			{
				MItDag dagIt (MItDag::kDepthFirst);
				dagIt.reset(path,MItDag::kDepthFirst);
				for (dagIt.next(); !dagIt.isDone(); dagIt.next()) 
				{
					MDagPath path;
					if (dagIt.getPath (path) == MS::kSuccess) 
					{
						object = path.node();
						std::string str(path.fullPathName().asChar());
						constIter = nodeNames.end();
						if(nodeNames.find(str)== constIter)
							nodeNames.insert(str);
					}
				}
			}
			list.remove(i); 
		}//else not a dag node, leave it in  in the list.
	}

    constIter = nodeNames.end();
	numObjects = list.length(); //this is just leftover depend nodes now
	unsigned int size = ((unsigned int)nodeNames.size()) + numObjects;
	depths.resize(size);

	//depths for all depend nodes is 0
	for(unsigned int k =0; k<numObjects;++k)
		depths[k] =0;
	unsigned int count = numObjects;
	MItDag dagIt (MItDag::kDepthFirst);
	for (dagIt.next(); !dagIt.isDone(); dagIt.next())
	{
		if (dagIt.getPath (path) == MS::kSuccess)
		{
			MObject object = path.node();
			std::string str(path.fullPathName().asChar());

			if(nodeNames.find(str) != constIter ) 
			{
				list.add(path);
				if(count < size)
					depths[count++] = dagIt.depth();
			}
		}
	}

	return;
}

//atom template reader class
void atomTemplateReader::setTemplate(const MString &templateName, MString &viewName)
{
	fTemplateSet = false;
	MStringArray result;
	MStatus status = MS::kFailure;
	if(viewName.length()==0) //no view specified so query template
	{
		MString melCommand = MString("containerTemplate -query -attributeList ") + templateName + ";";
		status = MGlobal::executeCommand(melCommand, result, false, false );
	}
	else
	{
		MString melCommand = MString("baseView -query -viewName ") + viewName + " -itemList -itemInfo itemName "
		+ templateName + ";";
		status = MGlobal::executeCommand(melCommand, result, false, false );
		if(status == MS::kSuccess)
		{
			//okay get the list but everything may not be an attribute so we need to query the array of attributes
			MStringArray isAttribute;
			melCommand = MString("baseView -query -viewName ") + viewName + " -itemList -itemInfo itemIsAttribute "
		+ templateName + ";";
			status = MGlobal::executeCommand(melCommand, isAttribute, false, false );
			if(status ==MS::kSuccess)
			{
				//remove non attributes
				if(isAttribute.length()==result.length())
				{
					unsigned int z = result.length();
					do
					{
						--z;
						if(isAttribute[z] == "0") //not an attribute
							result.remove(z); //remove it
					}while( z > 0);
				}
				//else not sure what todo if it's not the same size, should be an error with baseView, for now just leave the results alone.
			}
		}
	}

	if (status ==MS::kSuccess) 
	{
		fTemplateSet = true;
		for(unsigned int i =0;i < result.length();++i)
		{
			//use _ as the delimiter between the node name and the attribute, e.g. pSphere1_translateX.
			//since split doesn't work well with Name__name_attr we find the first _ by going backwards
			//use const char* since node names are always chars.
			const char * str = result[i].asChar();
			int len = (int)result[i].numChars();
			int z = len -1;
			for(; z >=0; --z)
			{
				if(str[z] == '_')
					break;
			}
			if(z < 1 || z == (len -1))
				continue; //should never happen but dont want to crash
			MString nodeName = result[i].substring(0,z-1);
			MString attribute = result[i].substring(z+1,len-1);

			if(nodeName.length() >0) //we have a node name
			{
				AttrMap::iterator p;
				std::string stdNodeName(nodeName.asChar());
				std::string stdAttrName(attribute.asChar());
				p = fNodeAttrs.find(stdNodeName);
				if(p != fNodeAttrs.end())
				{
					Attrs val = p->second;
					val.fAttrStrings.insert(stdAttrName);
					fNodeAttrs[stdNodeName] = val;
				}
				else
				{
					Attrs val;
					val.fAttrStrings.insert(stdAttrName);
					fNodeAttrs[stdNodeName] =val;
				}
			}
		}
	}
}


bool atomTemplateReader::findNode(const MString &nodeName)
{
	if(fTemplateSet)
	{
		AttrMap::const_iterator p;
		std::string stdNodeName(nodeName.asChar());
		p = fNodeAttrs.find(stdNodeName);
		return (p != fNodeAttrs.end());
	}
	return true; //not set so it's valid
}

bool atomTemplateReader::findNodeAndAttr(const MString &nodeName,const MString &attribute)
{
	if(fTemplateSet)
	{
		std::string stdNodeName(nodeName.asChar());
		std::string stdAttrName(attribute.asChar());
		AttrMap::iterator p;
		p = fNodeAttrs.find(stdNodeName);
		if(p != fNodeAttrs.end())
		{
			Attrs val = p->second;
			std::set<std::string>::iterator iter = val.fAttrStrings.find(stdAttrName);
			return (iter != val.fAttrStrings.end());
		}
		return false;
	}
	return true; //not set so it's valid
}

MString atomTemplateReader::attributesForNode(const MString  &nodeName)
{
	MString attributes("");
	if(fTemplateSet)
	{
		std::string stdNodeName(nodeName.asChar());
		AttrMap::iterator p;
		p = fNodeAttrs.find(stdNodeName);
		if(p != fNodeAttrs.end())
		{
			Attrs val = p->second;
			for(std::set<std::string>::iterator iter = val.fAttrStrings.begin(); iter !=
				val.fAttrStrings.end(); ++iter)
			{
				std::string val = *iter;
				attributes += MString (" -at ") + MString(val.c_str()) + MString(" ");
			}
		}
	}
	return attributes;
}

void atomTemplateReader::selectNodes()
{
	if(fTemplateSet)
	{
		bool first = true;
		AttrMap::iterator iter;
		for(iter = fNodeAttrs.begin(); iter != fNodeAttrs.end();++iter)
		{
			std::string nodeName = iter->first;
			MString mName(nodeName.c_str());
			if(first)
			{
				first = false;
				MGlobal::selectByName( mName, MGlobal::kReplaceList );
			}
			else
				MGlobal::selectByName( mName, MGlobal::kAddToList );
		}
	}
}
atomTemplateReader::~atomTemplateReader()
{

	//now we can clear the map
	fNodeAttrs.clear();

}



