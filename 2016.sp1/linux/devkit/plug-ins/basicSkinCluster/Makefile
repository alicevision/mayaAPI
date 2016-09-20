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

SRCDIR := $(TOP)/basicSkinCluster
DSTDIR := $(TOP)/basicSkinCluster

basicSkinCluster_SOURCES  := $(TOP)/basicSkinCluster/basicSkinCluster.cpp
basicSkinCluster_OBJECTS  := $(TOP)/basicSkinCluster/basicSkinCluster.o
basicSkinCluster_PLUGIN   := $(DSTDIR)/basicSkinCluster.$(EXT)
basicSkinCluster_MAKEFILE := $(DSTDIR)/Makefile

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

$(basicSkinCluster_OBJECTS): CFLAGS   := $(CFLAGS)   $(basicSkinCluster_EXTRA_CFLAGS)
$(basicSkinCluster_OBJECTS): C++FLAGS := $(C++FLAGS) $(basicSkinCluster_EXTRA_C++FLAGS)
$(basicSkinCluster_OBJECTS): INCLUDES := $(INCLUDES) $(basicSkinCluster_EXTRA_INCLUDES)

depend_basicSkinCluster:     INCLUDES := $(INCLUDES) $(basicSkinCluster_EXTRA_INCLUDES)

$(basicSkinCluster_PLUGIN):  LFLAGS   := $(LFLAGS) $(basicSkinCluster_EXTRA_LFLAGS) 
$(basicSkinCluster_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(basicSkinCluster_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_basicSkinCluster clean_basicSkinCluster Clean_basicSkinCluster


$(basicSkinCluster_PLUGIN): $(basicSkinCluster_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_basicSkinCluster :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(basicSkinCluster_SOURCES)

clean_basicSkinCluster:
	-rm -f $(basicSkinCluster_OBJECTS)

Clean_basicSkinCluster:
	-rm -f $(basicSkinCluster_MAKEFILE).bak $(basicSkinCluster_OBJECTS) $(basicSkinCluster_PLUGIN)


plugins: $(basicSkinCluster_PLUGIN)
depend:	 depend_basicSkinCluster
clean:	 clean_basicSkinCluster
Clean:	 Clean_basicSkinCluster

