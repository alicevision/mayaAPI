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

SRCDIR := $(TOP)/closestPointOnCurve
DSTDIR := $(TOP)/closestPointOnCurve

closestPointOnCurve_SOURCES  := $(TOP)/closestPointOnCurve/closestPointOnCurveCmd.cpp $(TOP)/closestPointOnCurve/closestPointOnCurveNode.cpp $(TOP)/closestPointOnCurve/closestPointOnCurvePlugin.cpp $(TOP)/closestPointOnCurve/closestTangentUAndDistance.cpp
closestPointOnCurve_OBJECTS  := $(TOP)/closestPointOnCurve/closestPointOnCurveCmd.o $(TOP)/closestPointOnCurve/closestPointOnCurveNode.o $(TOP)/closestPointOnCurve/closestPointOnCurvePlugin.o $(TOP)/closestPointOnCurve/closestTangentUAndDistance.o
closestPointOnCurve_PLUGIN   := $(DSTDIR)/closestPointOnCurve.$(EXT)
closestPointOnCurve_MAKEFILE := $(DSTDIR)/Makefile

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

$(closestPointOnCurve_OBJECTS): CFLAGS   := $(CFLAGS)   $(closestPointOnCurve_EXTRA_CFLAGS)
$(closestPointOnCurve_OBJECTS): C++FLAGS := $(C++FLAGS) $(closestPointOnCurve_EXTRA_C++FLAGS)
$(closestPointOnCurve_OBJECTS): INCLUDES := $(INCLUDES) $(closestPointOnCurve_EXTRA_INCLUDES)

depend_closestPointOnCurve:     INCLUDES := $(INCLUDES) $(closestPointOnCurve_EXTRA_INCLUDES)

$(closestPointOnCurve_PLUGIN):  LFLAGS   := $(LFLAGS) $(closestPointOnCurve_EXTRA_LFLAGS) 
$(closestPointOnCurve_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL $(closestPointOnCurve_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_closestPointOnCurve clean_closestPointOnCurve Clean_closestPointOnCurve


$(closestPointOnCurve_PLUGIN): $(closestPointOnCurve_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_closestPointOnCurve :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(closestPointOnCurve_SOURCES)

clean_closestPointOnCurve:
	-rm -f $(closestPointOnCurve_OBJECTS)

Clean_closestPointOnCurve:
	-rm -f $(closestPointOnCurve_MAKEFILE).bak $(closestPointOnCurve_OBJECTS) $(closestPointOnCurve_PLUGIN)


plugins: $(closestPointOnCurve_PLUGIN)
depend:	 depend_closestPointOnCurve
clean:	 clean_closestPointOnCurve
Clean:	 Clean_closestPointOnCurve

