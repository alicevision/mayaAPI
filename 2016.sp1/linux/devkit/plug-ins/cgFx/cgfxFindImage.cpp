//
// Copyright (C) 2002-2003 NVIDIA 
// 
// File: cgfxFindImage.cpp

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

#include "cgfxShaderCommon.h"
#include "cgfxFindImage.h"

#include <maya/MStringArray.h>
#include <maya/MGlobal.h>
#include <maya/MFileObject.h>

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#else
#	include <string.h>
#	define  MAX_PATH 1024
#endif

MString cgfxFindFile(const MString& name, const MString &searchpath)
{
	struct stat statBuf;
    char path[MAX_PATH];

    const char * psearchpath = searchpath.asChar();

	OutputDebugString("File = ");
	OutputDebugString(name.asChar());
	OutputDebugString("\n");

#ifdef _WIN32
    const bool fullyQualified = name.index('/') == 0 || name.index('\\') == 0 || name.index(':') == 1;
#else
    const bool fullyQualified = name.index('/') == 0 || name.index('\\') == 0;
#endif

    // First we check if it is a fully qualified path...
    if (fullyQualified && stat(name.asChar(), &statBuf) != -1) {
        OutputDebugString("Returning fully qualified: ");
        OutputDebugString(name.asChar());
        OutputDebugString("\n");

        return name;
    }
    else {
        // Strip out any leading '/' at this point since the file has
        // not been found using a fully qualified path.
        MString resolvedName;
        if(name.index('/') == 0 || name.index('\\') == 0)
            resolvedName = name.substring(1, name.length() - 1);
        else
            resolvedName = name;
        
        while (psearchpath < searchpath.asChar() + searchpath.length())
	    {
            const char * endpath = strchr(psearchpath,';');
            if (endpath)
            {
                strncpy(path,psearchpath, endpath - psearchpath);
                path[endpath - psearchpath] = '\0';
            }
            else
            {
                strcpy(path,psearchpath);
            }
                
            psearchpath += strlen(path)+1;

	        bool fullPath = (path[0] == '/'	||
					         path[0] == '\\');

	        if (strlen(path) > 2)
	        {
		        fullPath = fullPath ||
				           (path[1] == ':' &&
					        (path[2] == '/' ||
					         path[2] == '\\'));
	        }

            // Add the path and the filename together to get the full path
            MString file;
            if(path[strlen(path) - 1] != '/')
	            file = MString(path) + "/" + resolvedName;
            else
                file = MString(path) + resolvedName;

            OutputDebugString("Try File = ");
            OutputDebugString(file.asChar());
            OutputDebugString("\n");

            if (stat(file.asChar(), &statBuf) != -1) {
                OutputDebugString("Returning: ");
                OutputDebugString(file.asChar());
                OutputDebugString("\n");
    
                return file;
            }
	    }

        OutputDebugString("Not found!\n");
    
        return MString();
    }
}

MString cgfxFindFile(const MString& name, bool projectRelative)
{
	// Our result
	MString fileName;

	// Do we have an image to look for?
    if (name.length() > 0)
    {
		// Build a list of places we'll look for textures
		MString texturePath;

		// Add the standard Maya project paths
		MString workspace;
		MStatus status = MGlobal::executeCommand(MString("workspace -q -rd;"),
										workspace);
        if ( status == MS::kSuccess)
		{
			texturePath += workspace;
			texturePath += ";";
			texturePath += workspace;
			texturePath += "/textures;";
			texturePath += workspace;
			texturePath += "/images;";
			texturePath += workspace;
			texturePath += "/sourceimages";
		}

		// Finally, see if any CgFX environment variable paths are set
		char * cgfxPath = getenv("CGFX_TEXTURE_PATH");
		if (cgfxPath)
		{
            if (texturePath.length() > 0) {
                texturePath += ";";
            }
			texturePath += cgfxPath;
		}
		else
		{
			char * cgfxRoot = getenv("CGFX_ROOT");
			if (cgfxRoot)
			{
                if (texturePath.length() > 0) {
                    texturePath += ";";
                }
				texturePath += cgfxRoot;
				texturePath += "/textures/2D;";
				texturePath += cgfxRoot;
				texturePath += "/textures/cubemaps;";
				texturePath += cgfxRoot;
				texturePath += "/textures/3D;";
				texturePath += cgfxRoot;
				texturePath += "/textures/rectangles;";
				texturePath += cgfxRoot;
				texturePath += "/CgFX_Textures;";
				texturePath += cgfxRoot;
				texturePath += "/CgFX";
			}
		}

		OutputDebugString("CgFX texture path is: ");
		OutputDebugString(texturePath.asChar());
		OutputDebugString("\n");

	    fileName = cgfxFindFile(name, texturePath);

        int hasFile = fileName.length() > 0;

	    if (hasFile == 0)
        {
            // lets extract the filename and try it again...
            int idx = name.rindex('/');
            if (idx == -1)
                idx = name.rindex('\\');
            if (idx != -1)
            {
                MString filename = name.substring(idx+1,name.length()-1);
                fileName = cgfxFindFile(filename, texturePath);
                hasFile = fileName.length() > 0;
            }
        }
            
		// If we found the file and the user wants project relative, try
		// to strip the project directory off the front of our result
		if( hasFile && projectRelative)
		{
			if( fileName.length() > workspace.length() && 
				workspace.length() > 0 &&
				fileName.substring( 0, workspace.length() - 1) == workspace)
				// Strip the project path off the front INCLUDING the
				// separating '/' (otherwise we'd create an absolute path)
				fileName = fileName.substring( workspace.length(), fileName.length() - 1);
		}

        if (hasFile == 0)
		    OutputDebugString("Error: file not found.\n");
    }
    return fileName;
}

void    
cgfxGetFxIncludePath( const MString &fxFile, MStringArray &pathOptions )
{
	// Append the path of the cgfx file as a possible include search path
	//
	MString option;
	if (fxFile.length())
	{
		MFileObject fobject;
		fobject.setRawFullName( fxFile );
		option = MString("-I") + fobject.resolvedPath();		
		pathOptions.append( option );
	}

	// Add in "standard" cgfx search for cgfx files as a possible include
	// search path
	//
	char * cgfxRoot = getenv("CGFX_ROOT");
	if (cgfxRoot)
	{
		option = MString("-I") + MString(cgfxRoot);
		pathOptions.append( option );
		option = MString("-I") + MString(cgfxRoot) + MString("/CgFX");
		pathOptions.append( option );
	}

	// Add in Maya's Cg directory
	char * mayaLocation = getenv("MAYA_LOCATION");
	if (mayaLocation)
	{
		MString mayaCgLocation(MString(mayaLocation) + MString("/bin/Cg/"));
		option = MString("-I") + mayaCgLocation;
		pathOptions.append( option );
	}
}
