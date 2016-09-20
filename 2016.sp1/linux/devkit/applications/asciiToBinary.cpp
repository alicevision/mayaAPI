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

#include <maya/MStatus.h>
#include <maya/MString.h> 
#include <maya/MFileIO.h>
#include <maya/MLibrary.h>
#include <maya/MIOStream.h>
#include <string.h>

const char* usage = "usage: [-h/help] asciiToBinary fileName1 fileName2 ...\n\
       each file will be loaded, the filename will be checked for an\n\
       extension.  If one is found it will be change to .mb, otherwise a\n\
       .mb will be appended to the file name.  The scene will then be\n\
       written out to this new filename in Maya Binary format.\n";

int main(int argc, char **argv)
{
	MStatus stat;

	argc--, argv++;

	if (argc == 0) {
		cerr << usage;
		return(1);
	}

	for (; argc && argv[0][0] == '-'; argc--, argv++) {
		if (!strcmp(argv[0], "-h") || !strcmp(argv[0], "-help")) {
			cerr << usage;
			return(1);
		}
		// Check for other valid flags

		if (argv[0][0] == '-') {
			// Unknown flag
			cerr << usage;
			return(1);
		}
	}

	stat = MLibrary::initialize (argv[0]);
	if (!stat) {
		stat.perror("MLibrary::initialize");
		return 1;
	}

	for (; argc; argc--, argv++) {
		MString	fileName(argv[0]);
		MString fileType;

		MFileIO::newFile(true);

		// Load the file into Maya
		stat = MFileIO::open(fileName);
		if ( !stat ) {
			stat.perror(fileName.asChar());
			continue;
		}

		// Check to see if file is already in binary format
		fileType = MFileIO::fileType();
		if (fileType == MString("mayaBinary")) {
			cerr << fileName << ": already in mayaBinary format\n";
			continue;
		}

		// Check for a file extension, and if one exists, change it
		// to .mb.  If an extension does not exist, append a .mb to
		// the fileName.
		MString newFile;

		int loc = fileName.rindex('.');
		if (loc == -1) {
			newFile = fileName;
		} else {
			newFile = fileName.substring(0, loc-1);
		}
		newFile += ".mb";

		stat = MFileIO::saveAs(newFile, "mayaBinary");
		if (stat)
			cerr << fileName
				 << ": resaved as "
			     << MFileIO::currentFile()
				 << endl;
		else
			stat.perror(newFile.asChar());

	}

	MLibrary::cleanup();
	return 0;
}
