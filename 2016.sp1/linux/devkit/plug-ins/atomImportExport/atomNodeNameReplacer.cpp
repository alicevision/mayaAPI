/** Copyright 2015 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 

//
//	File Name:	atomNodeNameReplacer.cpp
//   The atomeNodeNameReplacer is a class that returns a new node name based upon how we are replacing it ,either by string replacment
//  or by finding it in a map file.

#include <maya/MGlobal.h>
#include <maya/MCommandResult.h>
#include <maya/MIOStream.h>
#include <maya/MDagPath.h>
#include <maya/MItDag.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "atomNodeNameReplacer.h"
#include <vector>
#include <map>


const char kSemiColonChar	= ';';
const char kSpaceChar		= ' ';
const char kTabChar			= '\t';
const char kHashChar		= '#';
const char kNewLineChar		= '\n';
//const char kSlashChar		= '/';    // not used, keep for now
const char kBraceLeftChar	= '{';
const char kBraceRightChar	= '}';
const char kDoubleQuoteChar	= '"';


streamIO::streamIO()
//
//	Description:
//		The constructor.
//
{
}

double streamIO::asDouble (ifstream &clipFile)
//
//	Description:
//		Reads the next bit of valid data as a double.
//
{
	advance(clipFile);

	double value;
	clipFile >> value;

	return (value);
}

bool streamIO::isNextNumeric(ifstream &clipFile)
//
//	Description:
//		The method skips past whitespace and comments and checks if
//		the next character is numeric.
//		
//		true is returned if the character is numeric.
//
{
	bool numeric = false;
	advance(clipFile);

	char next = clipFile.peek();
	if ((next >= '0' && next <= '9')
		||(next == '.') || (next == '-')) {
		numeric = true;
	}

	return numeric;
}

void streamIO::advance (ifstream &clipFile) 
//
//	Description:
//		The method skips past all of the whitespace and commented lines
//		in the ifstream. It will also ignore semi-colons.
//
{
	while (clipFile) {
		clipFile >> ws;

		char next = clipFile.peek();

		if (next == kSemiColonChar) {
			clipFile.ignore(1, kSemiColonChar);
			continue;
		}

		if (next == kHashChar) {
			clipFile.ignore(INT_MAX, kNewLineChar);
			continue;
		}

		break;
	}
}

char* streamIO::asWord (ifstream &clipFile, bool includeWS /* false */)
//
//	Description:
//		Returns the next string of characters in an ifstream. The string
//		ends when whitespace or a semi-colon is encountered. If the 
//		includeWS argument is true, the string will not end if a white
//		space character is encountered.
//
//		If a double quote is detected '"', then verything up to the next 
//		double quote will be returned.
//
//		This method returns a pointer to a static variable, so its contents
//		should be used immediately.
//		
{
	static const int kBufLength = 1024;
	static char string[kBufLength];

	advance(clipFile);

	char *c = string;
	clipFile.read (c, 1);

	if (*c == kDoubleQuoteChar) {
		clipFile.read(c, 1);
		while(!clipFile.eof() && (*c != kDoubleQuoteChar)) {
			c++;
			if (c-string >= kBufLength) {
				break;
			}
			clipFile.read(c, 1);
		}
	} else {
		
		//	Get the case of the '{' or '}' character
		//
		if (*c == kBraceLeftChar || *c == kBraceRightChar) {
			c++;
		} else {
			while(!clipFile.eof() && (*c != kSemiColonChar)) {
				if (!includeWS && ((*c == kSpaceChar) || (*c == kTabChar))) {
					break;
				}
				c++;
				if (c-string >= kBufLength) {
					break;
				}
				clipFile.read(c, 1);
			}
		}
	}
	*c = 0x00;

	return (string);
}

char streamIO::asChar (ifstream &clipFile)
//
//	Description:
//		Returns the next character of interest in the ifstream. All 
//		whitespace and commented lines are ignored.
//
{
	advance(clipFile);
	return clipFile.get();
}


int streamIO::asInt (ifstream &clipFile)
//
//	Description:
//		Reads the next bit of valid data as a int.
//
{
	advance(clipFile);

	int value;
	clipFile >> value;

	return (value);
}


short streamIO::asShort (ifstream &clipFile)
//
//	Description:
//		Reads the next bit of valid data as a short.
//
{
	advance(clipFile);

	short value;
	clipFile >> value;

	return (value);
}

//use the filetest command to see if the file is really a file
bool streamIO::doesFileExist(const MString &fileName)
{
	if(fileName.length() > 0)
	{
		MCommandResult result;
		int isFile;
		MString melCommand = "filetest -f " + MString("\"")  + fileName + MString("\"");
		if (MS::kSuccess == MGlobal::executeCommand(melCommand, result, false, false ))
		{
			if (MCommandResult::kInt == result.resultType()) 
			{ 
				result.getResult(isFile);
				return (isFile  > 0);
			}
		}
	}
	return false;
}

atomNodeNameReplacer::atomNodeNameReplacer(ReplaceType type,MSelectionList &sList, std::vector<unsigned int> &depths,
	MString &prefix, MString &suffix, MString &search,
	MString &replace, MString &mapFile)
	:fReplaceType(type), fPrefix(prefix), fSuffix(suffix), fSearch(search), fReplace(replace), fMapFile(mapFile),
	 fSelectionList(sList),
	 fDepths(depths),
	 fAddMainPlaceholderNamespace(0)
{
	
	if(fReplaceType == eMapFile && fMapFile.length() >0)
	{
		MString fileName = fMapFile;
		if(doesFileExist(fileName) == false)
		{
			fReplaceType = eHierarchy;
			return;
		}
		ifstream mapAnim(fileName.asChar());

		if(mapAnim.is_open() == false) 
		{
			fReplaceType = eHierarchy;
			return;
		}
		else if(mapAnim.good() == false)
		{
			fReplaceType = eHierarchy;
			//the fle is open so close it
			mapAnim.close();
			return;
		}
		char *dataType;
		bool current = true;
		while (!mapAnim.eof())
		{
			advance(mapAnim);
			dataType = asWord(mapAnim);
			if(dataType != NULL)
			{
				MString string(dataType);
				if(string.length() >0)
				{
					if(current)
						fCurrentNames.append(string);
					else fNewNames.append(string);
					current =  !current;
				}
			}
		}
		if(fNewNames.length() != fCurrentNames.length() && fCurrentNames.length() > 0)
			fCurrentNames.remove(fCurrentNames.length() -1);

		mapAnim.close();
	}

//	0 , 0,bob 0,dude,  1, none, 2, what, 
//	find 0,1
/*	
	typedef multimap<unsigned int, MString> ChildNodeName;
	typedef multimap<unsigned int, ChildNodeName> StringVec;

	StringMap map;
	MapOfNodeNames nodeNames;
	
	unsigned int numObjects = sList.length();
	for (unsigned int i = 0; i < numObjects; i++) 
	{
		MDagPath path;
		MObject node;
		if (sList.getDagPath (i, path) == MS::kSuccess) {
			MItDag dagIt (MItDag::kDepthFirst);
			dagIt.reset (path, MItDag::kDepthFirst);
			for (; !dagIt.isDone(); dagIt.next()) {
				MDagPath thisPath;
				if (dagIt.getPath (thisPath) != MS::kSuccess) {
					continue;
				}
				MString name = thisPath.partialPathName;
				unsigned int depth = dagIt.depth();
				unsigned int child = thisPath.childCount();
			}
		}
		else if (sList.getDependNode (i, node) == MS::kSuccess) {
			//write (animFile, node,haveAnimatableChannels);
		}
	}
	*/
}

void
atomNodeNameReplacer::setAddMainPlaceholderNamespace(bool val)
{
	fAddMainPlaceholderNamespace = val;
}

bool atomNodeNameReplacer::matchByName() //do we match by name or not?
{
	return fReplaceType == eHierarchy ? false : true;
}

static MString stringSearchAndReplace(
	const MString& searchString, const MString& replaceString, const MString& originalString)
{
	MString newString; // The result to be returned
	MString inputString(originalString); // The string we are going to do the work on
	bool notDone = true;
	while ( notDone )
	{
		int iw = inputString.indexW( searchString );
		if ( iw < 0 )
		{
			newString += inputString;
			notDone = false;
		}
		else
		{
			MString firstPart, secondPart;
			if ( iw > 0 )
				firstPart = inputString.substringW( 0, iw-1);
			int lastIndex = inputString.numChars() - 1;
			int firstIndex = iw + searchString.numChars();
			if ( firstIndex <= lastIndex )
				secondPart = inputString.substringW( firstIndex, lastIndex );
			newString = newString + firstPart + replaceString;
			inputString = secondPart;
		}
	}
	return newString;
}

MString	 atomNodeNameReplacer::replacedName(MString &name)
{
	if(fReplaceType == eSearchReplace)
	{
		MString newString(name);
		if(fSearch.length() > 0)
		{
			MString inputString(name);
			MString replaceString(fReplace);
			MString searchString(fSearch);
			newString = stringSearchAndReplace( searchString, replaceString, inputString );
		}
		if(fPrefix.length() >0)
			newString = fPrefix + newString;
		if(fSuffix.length() > 0)
			newString = newString + fSuffix;
		if(newString.length() >0)
			return newString;
		else
			return name;
	}
	else if(fReplaceType == eMapFile)
	{
		return replacedNameFromMapFileStrings(name);
	}
	return name;
}


MString	atomNodeNameReplacer::replacedNameFromMapFileStrings(MString &current)
{

	for(unsigned int i=0;i<fCurrentNames.length();++i)
	{
		if(fCurrentNames[i] == current && i < fNewNames.length())
			return fNewNames[i];
	}
	return current;
}


//this finds the node for a current passed in nodeName
//we assume that we only pass this name in once per load
//since with hierarchies once we resolve a node we remove it so that it doesn't get resolved more than once
bool atomNodeNameReplacer::findNode(atomNodeNameReplacer::NodeType nodeType, MString &nodeName, unsigned int depth, unsigned int childCount)
{

	MSelectionList list;
	bool selected = false;
	bool isDag =  (nodeType == atomNodeNameReplacer::eDag);
	bool isShape = (nodeType == atomNodeNameReplacer::eShape);
	bool isAnimLayer = (nodeType == atomNodeNameReplacer::eAnimLayer);
	if(matchByName() || nodeType == atomNodeNameReplacer::eDepend || isAnimLayer) //if depend node or layer  we still match by name
	{
		nodeName = replacedName(nodeName);
		if (fAddMainPlaceholderNamespace) {
			MString tmpName(":");			
			tmpName += nodeName;
			nodeName = tmpName;
		}
		list.add(nodeName); //add the node to the selection list
		if(list.length() != 1)
		{
			//it may be that the passed in node name has a name space, so split it and try again...
			MStringArray nameSpaces;
			nodeName.split(':',nameSpaces);
			unsigned int len = nameSpaces.length();
			if(len > 1)
			{
				nodeName = nameSpaces[len -1];
				list.add(nodeName);
				if(list.length() == 1)
					selected = true;
			}
		}
		else
			selected = true;
		if(selected)
		{
			//if animlayer don't worry about selection
			if(isAnimLayer)
				return true;
			//node is in scene but is it selected?
			if(nodeType != atomNodeNameReplacer::eDepend)
			{
				MDagPath dagPath;
				MStatus localStat = list.getDagPath(0,dagPath);
				if(localStat != MStatus::kFailure)
				{
					selected = fSelectionList.hasItem(dagPath);
				}
			}
			else
			{
				MObject depNode;
				MStatus localStat = list.getDependNode( 0, depNode);
				if(localStat != MStatus::kFailure)
				{
					selected = fSelectionList.hasItem(depNode);
				}
			}
			return selected;
		}
		return false;
	}
	else if( isDag || isShape)//do a hierarchy search
	{
		//after some attempts a straight forward algorithm works the best
		//for a given depth we look for the next object off that depth. yes
		//that's it. works really well if the selections match up
		//and we get the same pruning effect that's in pastekeys that works
		int i =0;
		int lastOne = -1;
		int length = (int)fSelectionList.length();
		for(i = 0; i < (int)length;++i)
		{
			MDagPath dagPath;
			MStatus localStat = fSelectionList.getDagPath(i,dagPath);
			MObject object = dagPath.node();
			bool selectedIsShape =  object.hasFn(MFn::kShape); //if the current dag is actually a shape
			if(localStat != MStatus::kFailure)
			{
				//first we handle shapes
				unsigned int sceneDepth = fDepths[i];
				if((isDag && selectedIsShape) || (isShape && (selectedIsShape ==false))) //the saved is a dag but we have a shape, in this case we skip it, or also skip if the opposite
				{
					selected = false;
					lastOne = i;
					continue;
				}
				else if(isShape && selectedIsShape == true) //both are shapes then they match
				{
					//TODO need to do a better check here than just shapes
					selected = true;
					nodeName = dagPath.partialPathName();
					lastOne =i;
					break;
				}
				//this algorithm here let's us skip over depths lower then us
				//until we get back up to a depth that we are at.
				else if(sceneDepth < depth)
				{
					selected = false;
					break;
				}
				else if(sceneDepth == depth)
				{
					selected = true;
					nodeName = dagPath.partialPathName();
					lastOne =i;
					break;
				}
				//else continue
				lastOne =i;
			}
		}
		//remove all of the ones we skipped or used 
		for(int k = lastOne;  k >=0 ;--k)
		{
			fSelectionList.remove(k);
			fDepths.erase( fDepths.begin( ) + k );
		}

	}

	return selected;
}
		
