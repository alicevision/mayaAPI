#                       ****************
#************************  Makefile    ***************************
#                       ****************
#
#  Makes the mocap devkit examples
# 
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
#
#   $Id: Makefile.devkit.Linux /main/maya3.0_linux_port/1 2000/06/05 20:22:18 rjones Exp $
#

# The location of the Maya libraries and header files must be set in the
# environment or uncommented here
#
#MAYA_LOCATION=/usr/autodesk/maya

BUILD_ROOT = $(MAYA_LOCATION)

INC_DIR = $(BUILD_ROOT)/include
LIB_DIR = $(BUILD_ROOT)/lib
LDLIBS  = -lm

.SUFFIXES: $(.SUFFIXES) .cpp

########################################################################
#
#  Definitions for common commands
#
########################################################################
CC  = cc
C++ = c++
RM  = rm -f

########################################################################
#
#  Basic Flags for compilers
#	Notes:
#		The order of rotation composition has reversed from
#		kinemation to maya.  Rotations specified XYZ for kin are in
#		fact ZYX for maya.  If your server was written in the kinemation
#		order sense use:
#			-DCAP_ROTATION_ORDER=KIN
#
#		All maya mocap servers should also define:
#			-DFUNCPROTO -DMAYA_MOCAP_DEVKIT
#
########################################################################
CDEBUGFLAGS =
#CDEBUGFLAGS = -g

INCLUDES_ALL = $(INCLUDES) -I$(INC_DIR)

DEFINES_ALL  = $(DEFINES) -DFUNCPROTO -DMAYA_MOCAP_DEVKIT

CFLAGS_ALL   = -mpentiumpro -fno-gnu-keywords $(CDEBUGFLAGS) $(CFLAGS)	\
		       $(INCLUDES_ALL) $(DEFINES_ALL) -DMOTIF \
			   -DXT_CODE -DCONTEXT_MACRO_ACCESS -DLINUX -DUNIX

LDFLAGS_ALL  = $(LDFLAGS)  $(CDEBUGFLAGS) -L$(LIB_DIR) $(LDOPTS) \
			  -lmocap $(LDLIBS)


C++FLAGS    = $(CFLAGS_ALL)



########################################################################
#
#  Convert a C source file (.c) to an object file (.o)
#  Convert a C++ source file (.cpp) to an object file (.o)
#
########################################################################
.c.o:
	$(CC) -c $(CFLAGS_ALL) -o $@ $<

.cpp.o:
	$(C++) -c $(C++FLAGS) -o $@ $<

########################################################################
#
#  Define the components of the various targets
#
########################################################################
SRCS_MCP    =	mcp.cpp
OBJS_MCP    =  $(SRCS_MCP:.cpp=.o)

SRCS_CLK    =	mayaClockServer.c
OBJS_CLK    =  $(SRCS_CLK:.c=.o)

SRCS_EXT	=	externServer.c channelParse.c
OBJS_EXT    =  $(SRCS_EXT:.c=.o)

SRCS_LOOP	=	loopfile.c 
OBJS_LOOP   =  $(SRCS_EXT:.c=.o)

OBJS    	=  $(OBJS_MCP) $(OBJS_CLK) $(OBJS_EXT) $(OBJS_LOOP)
SERVERS 	=  mayaClockServer mcp externServer loopfile


########################################################################
#
#  Useful top level targets
#
########################################################################
all:   $(SERVERS)

clean:	
	-$(RM) $(OBJS)
	-$(RM) $(SERVERS)

########################################################################
#
#  The build rules for the servers
#
########################################################################
mayaClockServer: $(OBJS_CLK)
	$(CC) $(CFLAGS_ALL) $(OBJS_CLK) $(LDFLAGS_ALL) -o $@

mcp: $(OBJS_MCP)
	$(C++) $(CFLAGS_ALL) $(OBJS_MCP) $(LDFLAGS_ALL) -o $@

externServer: $(OBJS_EXT)
	$(CC) $(CFLAGS_ALL) $(OBJS_EXT) $(LDFLAGS_ALL) -o $@

loopfile: $(OBJS_LOOP)
	$(CC) $(CFLAGS_ALL) $(OBJS_LOOP) $(LDFLAGS_ALL) -o $@





