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

SRCDIR := $(TOP)/closestPointOnNurbsSurfaceCmd
DSTDIR := $(TOP)/closestPointOnNurbsSurfaceCmd

closestPointOnNurbsSurfaceCmd_SOURCES  := $(TOP)/closestPointOnNurbsSurfaceCmd/closestPointOnNurbsSurfaceCmd.cpp
closestPointOnNurbsSurfaceCmd_OBJECTS  := $(TOP)/closestPointOnNurbsSurfaceCmd/closestPointOnNurbsSurfaceCmd.o
closestPointOnNurbsSurfaceCmd_PLUGIN   := $(DSTDIR)/closestPointOnNurbsSurfaceCmd.$(EXT)
closestPointOnNurbsSurfaceCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(closestPointOnNurbsSurfaceCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(closestPointOnNurbsSurfaceCmd_EXTRA_CFLAGS)
$(closestPointOnNurbsSurfaceCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(closestPointOnNurbsSurfaceCmd_EXTRA_C++FLAGS)
$(closestPointOnNurbsSurfaceCmd_OBJECTS): INCLUDES := $(INCLUDES) $(closestPointOnNurbsSurfaceCmd_EXTRA_INCLUDES)

depend_closestPointOnNurbsSurfaceCmd:     INCLUDES := $(INCLUDES) $(closestPointOnNurbsSurfaceCmd_EXTRA_INCLUDES)

$(closestPointOnNurbsSurfaceCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(closestPointOnNurbsSurfaceCmd_EXTRA_LFLAGS) 
$(closestPointOnNurbsSurfaceCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(closestPointOnNurbsSurfaceCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_closestPointOnNurbsSurfaceCmd clean_closestPointOnNurbsSurfaceCmd Clean_closestPointOnNurbsSurfaceCmd


$(closestPointOnNurbsSurfaceCmd_PLUGIN): $(closestPointOnNurbsSurfaceCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_closestPointOnNurbsSurfaceCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(closestPointOnNurbsSurfaceCmd_SOURCES)

clean_closestPointOnNurbsSurfaceCmd:
	-rm -f $(closestPointOnNurbsSurfaceCmd_OBJECTS)

Clean_closestPointOnNurbsSurfaceCmd:
	-rm -f $(closestPointOnNurbsSurfaceCmd_MAKEFILE).bak $(closestPointOnNurbsSurfaceCmd_OBJECTS) $(closestPointOnNurbsSurfaceCmd_PLUGIN)


plugins: $(closestPointOnNurbsSurfaceCmd_PLUGIN)
depend:	 depend_closestPointOnNurbsSurfaceCmd
clean:	 clean_closestPointOnNurbsSurfaceCmd
Clean:	 Clean_closestPointOnNurbsSurfaceCmd

