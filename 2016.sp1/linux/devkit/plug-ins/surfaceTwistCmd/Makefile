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

SRCDIR := $(TOP)/surfaceTwistCmd
DSTDIR := $(TOP)/surfaceTwistCmd

surfaceTwistCmd_SOURCES  := $(TOP)/surfaceTwistCmd/surfaceTwistCmd.cpp
surfaceTwistCmd_OBJECTS  := $(TOP)/surfaceTwistCmd/surfaceTwistCmd.o
surfaceTwistCmd_PLUGIN   := $(DSTDIR)/surfaceTwistCmd.$(EXT)
surfaceTwistCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(surfaceTwistCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(surfaceTwistCmd_EXTRA_CFLAGS)
$(surfaceTwistCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(surfaceTwistCmd_EXTRA_C++FLAGS)
$(surfaceTwistCmd_OBJECTS): INCLUDES := $(INCLUDES) $(surfaceTwistCmd_EXTRA_INCLUDES)

depend_surfaceTwistCmd:     INCLUDES := $(INCLUDES) $(surfaceTwistCmd_EXTRA_INCLUDES)

$(surfaceTwistCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(surfaceTwistCmd_EXTRA_LFLAGS) 
$(surfaceTwistCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(surfaceTwistCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_surfaceTwistCmd clean_surfaceTwistCmd Clean_surfaceTwistCmd


$(surfaceTwistCmd_PLUGIN): $(surfaceTwistCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_surfaceTwistCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(surfaceTwistCmd_SOURCES)

clean_surfaceTwistCmd:
	-rm -f $(surfaceTwistCmd_OBJECTS)

Clean_surfaceTwistCmd:
	-rm -f $(surfaceTwistCmd_MAKEFILE).bak $(surfaceTwistCmd_OBJECTS) $(surfaceTwistCmd_PLUGIN)


plugins: $(surfaceTwistCmd_PLUGIN)
depend:	 depend_surfaceTwistCmd
clean:	 clean_surfaceTwistCmd
Clean:	 Clean_surfaceTwistCmd

