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

SRCDIR := $(TOP)/skinClusterWeights
DSTDIR := $(TOP)/skinClusterWeights

skinClusterWeights_SOURCES  := $(TOP)/skinClusterWeights/skinClusterWeights.cpp
skinClusterWeights_OBJECTS  := $(TOP)/skinClusterWeights/skinClusterWeights.o
skinClusterWeights_PLUGIN   := $(DSTDIR)/skinClusterWeights.$(EXT)
skinClusterWeights_MAKEFILE := $(DSTDIR)/Makefile

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

$(skinClusterWeights_OBJECTS): CFLAGS   := $(CFLAGS)   $(skinClusterWeights_EXTRA_CFLAGS)
$(skinClusterWeights_OBJECTS): C++FLAGS := $(C++FLAGS) $(skinClusterWeights_EXTRA_C++FLAGS)
$(skinClusterWeights_OBJECTS): INCLUDES := $(INCLUDES) $(skinClusterWeights_EXTRA_INCLUDES)

depend_skinClusterWeights:     INCLUDES := $(INCLUDES) $(skinClusterWeights_EXTRA_INCLUDES)

$(skinClusterWeights_PLUGIN):  LFLAGS   := $(LFLAGS) $(skinClusterWeights_EXTRA_LFLAGS) 
$(skinClusterWeights_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(skinClusterWeights_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_skinClusterWeights clean_skinClusterWeights Clean_skinClusterWeights


$(skinClusterWeights_PLUGIN): $(skinClusterWeights_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_skinClusterWeights :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(skinClusterWeights_SOURCES)

clean_skinClusterWeights:
	-rm -f $(skinClusterWeights_OBJECTS)

Clean_skinClusterWeights:
	-rm -f $(skinClusterWeights_MAKEFILE).bak $(skinClusterWeights_OBJECTS) $(skinClusterWeights_PLUGIN)


plugins: $(skinClusterWeights_PLUGIN)
depend:	 depend_skinClusterWeights
clean:	 clean_skinClusterWeights
Clean:	 Clean_skinClusterWeights

