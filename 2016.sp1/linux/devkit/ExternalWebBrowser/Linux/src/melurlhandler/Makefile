#!make

#-
# ==========================================================================
# Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.
# ==========================================================================
#+

CC= gcc
OPTIMIZER= -g
CFLAGS=  $(OPTIMIZER) -I. -I/usr/include

SRC= main.c mcp.c 
OBJ= main.o mcp.o

BINTARGET=melurlhandler

default all: $(BINTARGET)

$(BINTARGET): $(OBJ)
	$(CC) -o $(BINTARGET) $(OBJ) $(LDFLAGS) -lgnomevfs-2

mcp.o: mcp.c
	$(CC) -c $(CFLAGS) mcp.c

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

clean:
	$(RM) $(OBJ) $(SHAREDTARGET) $(BINTARGET)
