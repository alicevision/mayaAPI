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

const char* usage = "usage: [-h/help] readAndWrite fileName1 fileName2 ...\n\
       each file will be loaded, the string \".updated\" will be added\n\
       either at the end of just before the extension, and the file will\n\
       be written back out again.  If the specified file was created by\n\
       an old version of Maya, the \"updated\" version will contain the\n\
       same scene but updated to the current file format.\n";

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

		cerr << "Loading \"" << fileName << "\" ... ";
		// Load the file into Maya
		stat = MFileIO::open(fileName);
		if ( !stat ) {
			stat.perror(fileName.asChar());
			continue;
		}
		cerr << " done.\n";

		// Get the file type
		fileType = MFileIO::fileType();

		// Don't overwrite the existing file
		MString newFile;

		// Find the extension if one exists
		int loc = fileName.rindex('.');
		if (loc == -1) {
			newFile = fileName + ".updated";
		} else {
			newFile = fileName.substring(0, loc-1);
			newFile += ".updated";
			newFile += fileName.substring(loc, fileName.length()-1);
		}

		stat = MFileIO::saveAs(newFile, fileType.asChar());
		cerr << "    ";
		if (stat) {
			cerr << "resaved as "
			     << MFileIO::currentFile()
				 << endl;
		} else {
			stat.perror(newFile.asChar());
		}
	}

	MLibrary::cleanup();
	return 0;
}
