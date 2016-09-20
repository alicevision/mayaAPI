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

SRCDIR := $(TOP)/surfaceCreateCmd
DSTDIR := $(TOP)/surfaceCreateCmd

surfaceCreateCmd_SOURCES  := $(TOP)/surfaceCreateCmd/surfaceCreateCmd.cpp
surfaceCreateCmd_OBJECTS  := $(TOP)/surfaceCreateCmd/surfaceCreateCmd.o
surfaceCreateCmd_PLUGIN   := $(DSTDIR)/surfaceCreateCmd.$(EXT)
surfaceCreateCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(surfaceCreateCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(surfaceCreateCmd_EXTRA_CFLAGS)
$(surfaceCreateCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(surfaceCreateCmd_EXTRA_C++FLAGS)
$(surfaceCreateCmd_OBJECTS): INCLUDES := $(INCLUDES) $(surfaceCreateCmd_EXTRA_INCLUDES)

depend_surfaceCreateCmd:     INCLUDES := $(INCLUDES) $(surfaceCreateCmd_EXTRA_INCLUDES)

$(surfaceCreateCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(surfaceCreateCmd_EXTRA_LFLAGS) 
$(surfaceCreateCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(surfaceCreateCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_surfaceCreateCmd clean_surfaceCreateCmd Clean_surfaceCreateCmd


$(surfaceCreateCmd_PLUGIN): $(surfaceCreateCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_surfaceCreateCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(surfaceCreateCmd_SOURCES)

clean_surfaceCreateCmd:
	-rm -f $(surfaceCreateCmd_OBJECTS)

Clean_surfaceCreateCmd:
	-rm -f $(surfaceCreateCmd_MAKEFILE).bak $(surfaceCreateCmd_OBJECTS) $(surfaceCreateCmd_PLUGIN)


plugins: $(surfaceCreateCmd_PLUGIN)
depend:	 depend_surfaceCreateCmd
clean:	 clean_surfaceCreateCmd
Clean:	 Clean_surfaceCreateCmd

