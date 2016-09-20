/*
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
*/

// To use this standalone:
// 1. Start Maya
// 2. In a MEL window, execute: commandPort
// 3. Execute mcp.exe from the command line: .\mcp.exe
// 4. Enter text such as: createNode "transform"  
// 5. Press the enter key and the node should be created in Maya
//

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#ifndef _WIN32
# include <sys/socket.h>
# include <unistd.h>
#else
# include <winsock2.h>
# include <windows.h>
# include <io.h>
#endif
#include <stdlib.h>
#ifdef LINUX
#include <string.h>
#endif

extern "C" {
#ifndef _WIN32
#include <maya/mocapserver.h>
#include <maya/mocaptcp.h>
#else
#include <maya/mocapserver.h>
#include <maya/mocaptcp.h>
#endif
}


#define DEVEL
int  show_usage = 0;
int  debug_mode = 0;
int  verbose    = 0;
int  readStdIn  = 0;
int  perLine = 0;
int  numPerLine = 0;
int  interactive = 0;

#ifdef LINUX
#define GETOPTHUH '?'
#endif

#ifdef _WIN32

char *   optarg = NULL;
int	    optind = 1;
int	    opterr = 0;
int     optlast = 0;
#define GETOPTHUH 0

int getopt(int argc, char **argv, char *pargs)
{
	if (optind >= argc) return EOF;

	if (optarg==NULL || optlast==':')
	{
		optarg = argv[optind];
		if (*optarg!='-' && *optarg!='/')
			return EOF;
	}

	if (*optarg=='-' || *optarg=='/') optarg++;
	pargs = strchr(pargs, *optarg);
	if (*optarg) optarg++;
	if (*optarg=='\0')
	{
		optind++;
		optarg = NULL;
	}
	if (pargs == NULL) return 0;  //error
	if (*(pargs+1)==':')
	{
		if (optarg==NULL)
		{
			if (optind >= argc) return EOF;
			// we want a second paramter
			optarg = argv[optind];
		}
		optind++;
	}
	optlast = *(pargs+1);

	return *pargs;
}
#else
#define closesocket close
#endif

void main(int argc, char **argv)
{
	char *server_name = "mayaCommand";
	int  opt;

	char *program = new char[strlen(argv[0]) + 1];
	char *cptr;
	/*
	 * Grab a copy of the program name
	 */
#ifdef _WIN32
		_splitpath (argv[0], NULL, NULL, program, NULL);
#else
	cptr = strrchr(argv[0], '/');
	if (cptr)
	{
		strcpy(program, (cptr + 1));
	}
	else
	{
		strcpy(program, argv[0]);
	}
#endif

#ifdef DEVEL
	while ((opt = getopt(argc, argv, "Dhvi1w:n:")) != -1)
#else /* DEVEL */
	while ((opt = getopt(argc, argv, "hi1w:n:")) != -1)
#endif /* DEVEL */
	{	  
		switch (opt)
		{
		case 'h':
			show_usage++;
			break;

		case 'D':
			debug_mode++;
			break;

		case 'n':
			server_name = optarg;
			break;

		case 'v':
			verbose++;
			break;

		case '1':
			perLine++;
			numPerLine = 1;
			break;

		case 'i':
			interactive++;
			break;

		case 'w':
			perLine++;
			numPerLine = atoi(optarg);
			break;

		case GETOPTHUH:
			show_usage++;
		}
	}

	if (show_usage)
	{
		fprintf(stderr, "Usage:\n");
#ifdef DEVEL
		fprintf(stderr, "    %s [-Dvhi1] [-w num ] [-n name]\n", argv[0]);
#else /* DEVEL */
		fprintf(stderr, "    %s [-hi1] [-w num ] [-n name]\n", argv[0]);
#endif /* DEVEL */
		fprintf(stderr, "\n");
		fprintf(stderr, "        -h        Print this help message\n");
#ifdef DEVEL
		fprintf(stderr, "        -D        Set the debug flag\n");
		fprintf(stderr, "        -v        Set the vebose flag\n");
#endif /* DEVEL */
		fprintf(stderr, "        -n name   The server's UNIX socket name\n");
		fprintf(stderr, "        -1 	   Format the results one field per line\n");
		fprintf(stderr, "        -w num	   Format the results num fields per line\n");
		fprintf(stderr, "        -i 	   Interactive. Prompt each line with the server name.\n");

		fprintf(stderr, "\n");

		exit(1);
	}

	char command[5000] = {'\0' };

	int i;

    for ( i = optind ; i < argc; i++) {
		strcat(command,argv[i]);
		strcat(command," ");
	}
	if ( strlen(command) == 0 )
		readStdIn = 1;

	if ( verbose ) {
		fprintf(stderr,"// debug_mode  = %d\n", debug_mode);
		fprintf(stderr,"// readStdIn   = %d\n", readStdIn);
		fprintf(stderr,"// perLine     = %d\n", perLine);
		fprintf(stderr,"// numPerLine  = %d\n", numPerLine);
		fprintf(stderr,"// interactive = %d\n", interactive);
		fprintf(stderr,"// verbose     = %d\n", verbose);
	}

	if ( verbose ) {
		fprintf(stderr,"// %s: contacting server %s\n",program, server_name);
		fflush(stderr);
	}


	
	// Connect to the new server
	int fd = CapTcpOpen(server_name);

	if ( fd < 0 ) {
		fprintf(stderr,"// %s: couldn't connect to server %s\n"
				,program, server_name);
		exit(-1);
	} else if ( verbose ) {
		fprintf(stderr,"// %s: connected to server %s\n",program, server_name);
	}
		

	do {
		if ( readStdIn ) {
			if ( interactive ) {
				printf("%s %% ", server_name);
				fflush(stdout);
			}

			if ( NULL == fgets (command, 5000, stdin) ) 
				break;
		}

		if ( verbose )
			fprintf(stderr,"// %s: sending command %s\n",program, command);

		send(fd,command,strlen(command),0);

		if ( verbose )
			fprintf(stderr,"// %s: awaiting reply...",program);

		char reply[5000];
		int red = recv(fd,reply,4096,0);

		if ( verbose )
			fprintf(stderr,"// %s: recieved %d bytes\n",program,red);

		if ( red > 0)
		{
			if ( perLine ) {
				char *tabby = reply;
				int   count = 0;
				// replace \t with \n in reply
				while ( tabby = strchr ( tabby, '\t' ) ) {
					if ( (++count % numPerLine) == 0 )
						*tabby = '\n';
					tabby++;
				}
			}


			printf("%s",reply);
		}
		else {
			if ( verbose )
				printf("READ FAILED\n");
			break;
		}
	} while (readStdIn);

	if ( verbose )
		fprintf(stderr,"// %s: closing connection.\n",program);

	closesocket(fd);
}
