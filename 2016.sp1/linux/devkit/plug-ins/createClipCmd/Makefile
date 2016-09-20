#-
# ==========================================================================
# Copyright (c) 2011 Autodesk, Inc.
# All rights reserved.
# 
# These coded instructions, statements, and computer programs contain
# unpublished proprietary information written by Autodesk, Inc., and are
# protected by Federal copyright law. They may not be disclosed to third
# parties or copied or duplicated in any form, in whole or in part, without
# the prior written consent of Autodesk, Inc.
# ==========================================================================
#+

ifndef INCL_BUILDRULES

#
# Include platform specific build settings
#
TOP := ..
include $(TOP)/buildrules


#
# Always build the local plug-in when make is invoked from the
# directory.
#
all : plugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/createClipCmd
DSTDIR := $(TOP)/createClipCmd

createClipCmd_SOURCES  := $(TOP)/createClipCmd/createClipCmd.cpp
createClipCmd_OBJECTS  := $(TOP)/createClipCmd/createClipCmd.o
createClipCmd_PLUGIN   := $(DSTDIR)/createClipCmd.$(EXT)
createClipCmd_MAKEFILE := $(DSTDIR)/Makefile

#
# Include the optional per-plugin Makefile.inc
#
#    The file can contain macro definitions such as:
#       {pluginName}_EXTRA_CFLAGS
#       {pluginName}_EXTRA_C++FLAGS
#       {pluginName}_EXTRA_INCLUDES
#       {pluginName}_EXTRA_LIBS
-include $(SRCDIR)/Makefile.inc


#
# Set target specific flags.
#

$(createClipCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(createClipCmd_EXTRA_CFLAGS)
$(createClipCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(createClipCmd_EXTRA_C++FLAGS)
$(createClipCmd_OBJECTS): INCLUDES := $(INCLUDES) $(createClipCmd_EXTRA_INCLUDES)

depend_createClipCmd:     INCLUDES := $(INCLUDES) $(createClipCmd_EXTRA_INCLUDES)

$(createClipCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(createClipCmd_EXTRA_LFLAGS) 
$(createClipCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(createClipCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_createClipCmd clean_createClipCmd Clean_createClipCmd


$(createClipCmd_PLUGIN): $(createClipCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_createClipCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(createClipCmd_SOURCES)

clean_createClipCmd:
	-rm -f $(createClipCmd_OBJECTS)

Clean_createClipCmd:
	-rm -f $(createClipCmd_MAKEFILE).bak $(createClipCmd_OBJECTS) $(createClipCmd_PLUGIN)


plugins: $(createClipCmd_PLUGIN)
depend:	 depend_createClipCmd
clean:	 clean_createClipCmd
Clean:	 Clean_createClipCmd

