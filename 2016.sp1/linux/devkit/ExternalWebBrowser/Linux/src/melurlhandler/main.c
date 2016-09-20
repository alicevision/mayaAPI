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

#include "mcp.h"

#include <string.h>

// We are using this function from gnome vfs utils to unescape URL
// It is declared in gnome-vfs-utils.h and defined in libgnomevfs.so
// Since we use only single function, it's much easier just to forward declare it
// instead of including the header.
extern char * gnome_vfs_unescape_string(const char *escaped_string, const char *illegal_characters);

#define kBufferSize 5000

int main(int argc, char **argv)
{
	int i;
	char buffer[kBufferSize];
	char *unescapedBuffer = NULL;
	int mcpSocket = -1;

	// expect at least one parameter after binary name
	if(argc > 1) {
		// catenate all the parameters into single string separated by spaces
		buffer[0] = '\0';
		for(i = 1; i < argc; i++) {
			strcat(buffer, argv[i]);
			strcat(buffer, " ");
		}

		// deal with url header
		if(strstr(buffer, "mel://") == buffer) { 
			strcpy(buffer, buffer+6);
		} else if(strstr(buffer, "mel:") == buffer) { 
			strcpy(buffer, buffer+4);
		}

		// get rid of trailing slash
		if(buffer[strlen(buffer)-2] == '/') {
			buffer[strlen(buffer)-2] = '\0';
		}

		// unescape the URL
		unescapedBuffer = gnome_vfs_unescape_string(buffer, "");

		if(NULL != unescapedBuffer) {
			// connect to default command port
			mcpSocket = mcpOpen("commandportDefault");
			if(mcpSocket != -1) {
				// send command to Maya	
				mcpWrite(mcpSocket, unescapedBuffer, strlen(unescapedBuffer));
				// clear the socket
				mcpRead(mcpSocket, buffer, kBufferSize);
				// close the socket
				mcpClose(mcpSocket);
			}
			free(unescapedBuffer);
		}
	}
	return 0;
}
