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

#include <stdio.h>
#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#else
#include <winsock2.h>
#include <windows.h>
#include <io.h>
#endif
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#ifdef  LINUX
#define GETOPTHUH '?'
#endif  /* LINUX */

#ifdef _WIN32

#define PATH_MAX _MAX_PATH

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

int gettimeofday(struct timeval* tp)
{
	unsigned int dw = timeGetTime();
	tp->tv_sec = dw/1000;
	tp->tv_usec = dw%1000;

	return 0;
}

#endif

size_t	gBufferSize = 4096;
float	gPlayFreq   = 30;
int		gShowUsage  = 0;
char gProgramName[PATH_MAX] = { 0 };
char gDataPath[PATH_MAX] = { 0 };
FILE *gDataFile = NULL;
char *gBuffer;

double toDouble(const struct timeval t) {
	return (double)(t.tv_sec) + (double)(t.tv_usec) / 1000000.0;
}

struct timeval	toTimeval(double sec)
{
	struct timeval tim;

	tim.tv_sec  = sec;
	tim.tv_usec = (sec - tim.tv_sec) * 1000000;

	return tim;
}


void printUsage()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    %s [-H] [-b size] -f file\n", gProgramName);
    fprintf(stderr, "\n");
	fprintf(stderr, "        -f file   file to read\n");
	fprintf(stderr, "        -b	size   max line length for input file\n");
    fprintf(stderr, "        -h        Print this help message\n");
	fprintf(stderr, "        -H        default record frequency in Hz\n");
    fprintf(stderr, "\n");

    exit(1);
}

void parseArgs(int argc, char **argv)
{
	int opt;
	char *cptr;
	float optFloat;
	/*
	 * Grab a copy of the program name
	 */
#ifdef _WIN32
		_splitpath (argv[0], NULL, NULL, gProgramName, NULL);
#else
	cptr = strrchr(argv[0], '/');
	if (cptr)
	{
		strcpy(gProgramName, (cptr + 1));
	}
	else
	{
		strcpy(gProgramName, argv[0]);
	}
#endif

	/*
	 * Set default values
	 */
	gBufferSize = 4096;
	gPlayFreq   = 30;
	gShowUsage  = 0;

	while ((opt = getopt(argc, argv, "b:H:hf:")) != -1) {
		switch (opt) {
		case 'f' :
			strcpy(gDataPath, optarg);
			break;

		case 'h':
			gShowUsage++;
			break;
		case 'b' :
			gBufferSize = atoi(optarg);
			break;

		case 'H':
			if ( 1 == sscanf( optarg ,"%f", &optFloat) ) {
				gPlayFreq = optFloat;
			} else {
				gShowUsage++;
			}
			break;
		
		case GETOPTHUH:
		default :
			gShowUsage++;
		}
	}

	if ( gPlayFreq <= 0. )
		gShowUsage++;

	if ( gBufferSize < 1 )
		gShowUsage++;

	if ( !gDataPath[0] )
		gShowUsage++;

	if (gShowUsage) 
		printUsage();


}



main( int argc, char *argv[] ) {
	
	double playNow    = 0;
	double playNext   = 0;
	double playPeriod = 1./30.;
	FILE   *dataFile  = NULL;

	parseArgs( argc, argv);

	playPeriod = 1./ gPlayFreq;

	dataFile = fopen(gDataPath, "r");
	if ( NULL == dataFile ) {
		fprintf(stderr,
				"%s: could not open file %s", gProgramName, gDataPath);
		exit(1);
	}

	gBuffer = malloc(gBufferSize);
	if ( NULL == gBuffer ) {
		fprintf(stderr,
				"%s: out of memory, -b %d failed", gProgramName, gBufferSize);
		exit(1);
	}

	strcpy(gBuffer,"");



	while (1) {
		struct timeval timeout, now;
#ifdef LINUX
	    gettimeofday(&now,NULL);
#else
		gettimeofday(&now);
#endif            
		playNow   = toDouble(now);
		if ( playNext == 0.0 ) /* The first record */
			playNext = playNow;

		while ( playNext <= playNow ) {
			if ( NULL == fgets(gBuffer, gBufferSize, dataFile ) ) {
				rewind( dataFile); 
				if ( NULL == fgets(gBuffer, gBufferSize, dataFile ) ) {
					fprintf(stderr, "%s: file read failed", 
							gProgramName);
					exit(1);
				}
			}
				
			printf( "%s", gBuffer );				
			playNext = playNext + playPeriod;
		}
		fflush(stdout);
			
		/*
		 * wait for commands or a timeout
		 */
		timeout = toTimeval( playNext - playNow );
		select(FD_SETSIZE, NULL, NULL, NULL, &timeout);

	}

}
