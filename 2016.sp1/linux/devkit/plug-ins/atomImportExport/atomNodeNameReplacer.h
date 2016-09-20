/** Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license 
agreement provided at the time of installation or download,  or 
which otherwise accompanies this software in either electronic 
or hard copy form.*/ 
//	File Name:	atomNodeNameReplacer.h
//
//   The atomeNodeNameReplacer is a class that returns a new node name based upon how we are replacing it ,either by string replacment
//  or by finding it in a map file.


#ifndef _atomNodeNameReplacer
#define _atomNodeNameReplacer

#include <vector>
#include <maya/MString.h> 
#include <maya/MFStream.h>
#include <maya/MStringArray.h>
#include <maya/MSelectionList.h>

//base class for reader text files
class streamIO
{
public:
	streamIO();
	virtual ~streamIO(){};

	double						asDouble(ifstream &);
	char *						asString(ifstream &);
	char *						asWord(ifstream &, bool = false);
	char 						asChar(ifstream &);
	int							asInt(ifstream &);
	short						asShort(ifstream &);

	bool						isNextNumeric(ifstream &);

	void						advance(ifstream &);

	bool						doesFileExist(const MString &fName);

};


// Class to modify the string of a node based upon either strings or some specified map file
class atomNodeNameReplacer : public streamIO
{

public:
	enum NodeType {eDag = 0, eShape, eDepend, eAnimLayer};
	enum ReplaceType {eHierarchy = 0, eSearchReplace, eMapFile};

	atomNodeNameReplacer(ReplaceType type, MSelectionList &list,std::vector<unsigned int> &depths, MString &prefix, MString &suffix,
		MString &search, MString &replace, MString &mapFile);
	virtual ~atomNodeNameReplacer(){};

	//return node in the scene that matches it.
	bool				findNode(atomNodeNameReplacer::NodeType nodeType, MString &nodeName, unsigned int depth, unsigned int childCount);
	void				setAddMainPlaceholderNamespace(bool);

	void				turnOffHierarchy() {if(fReplaceType == eHierarchy) fReplaceType = eSearchReplace;};
private:

	MSelectionList		fSelectionList; //this is the original list of objects
	std::vector<unsigned int> fDepths;
	ReplaceType			fReplaceType;
	bool				fAddMainPlaceholderNamespace;
	MString				fPrefix;
	MString				fSuffix;
	MString				fSearch;
	MString				fReplace;
	MString				fMapFile;

	//these two arrays should be in sync
	MStringArray		fCurrentNames;
	MStringArray		fNewNames;
protected:
	MString			replacedNameFromMapFileStrings(MString &current);
	bool			matchByName(); //do we match by name or not?
	//we call this now to change the name of the incoming
	//node based upon any search/replace or mapping parameters.
	MString				replacedName(MString &name);
};

#endif 

