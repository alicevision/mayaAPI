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

PLUGIN_DEFINES= -DXP_UNIX -DMDCPUCFG=\"_linux.cfg\"

CC= gcc
OPTIMIZER= -g
CFLAGS=  $(OPTIMIZER) $(PLUGIN_DEFINES) -I. -I/usr/include

SRC= main.c mcp.c PluginObject.c 
OBJ= main.o mcp.o PluginObject.o

SHAREDTARGET=libmcpplugin.so

default all: $(SHAREDTARGET)

$(SHAREDTARGET): $(OBJ)
	$(CC) -shared -o $(SHAREDTARGET) $(OBJ) $(LDFLAGS)

mcp.o: mcp.c
	$(CC) -c $(CFLAGS) mcp.c

PluginObject.o: PluginObject.c
	$(CC) -c $(CFLAGS) PluginObject.c

main.o: main.c
	$(CC) -c $(CFLAGS) main.c

clean:
	$(RM) $(OBJ) $(SHAREDTARGET)
