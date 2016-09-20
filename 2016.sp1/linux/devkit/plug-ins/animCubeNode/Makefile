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

SRCDIR := $(TOP)/animCubeNode
DSTDIR := $(TOP)/animCubeNode

animCubeNode_SOURCES  := $(TOP)/animCubeNode/animCubeNode.cpp
animCubeNode_OBJECTS  := $(TOP)/animCubeNode/animCubeNode.o
animCubeNode_PLUGIN   := $(DSTDIR)/animCubeNode.$(EXT)
animCubeNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(animCubeNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(animCubeNode_EXTRA_CFLAGS)
$(animCubeNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(animCubeNode_EXTRA_C++FLAGS)
$(animCubeNode_OBJECTS): INCLUDES := $(INCLUDES) $(animCubeNode_EXTRA_INCLUDES)

depend_animCubeNode:     INCLUDES := $(INCLUDES) $(animCubeNode_EXTRA_INCLUDES)

$(animCubeNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(animCubeNode_EXTRA_LFLAGS) 
$(animCubeNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(animCubeNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_animCubeNode clean_animCubeNode Clean_animCubeNode


$(animCubeNode_PLUGIN): $(animCubeNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_animCubeNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(animCubeNode_SOURCES)

clean_animCubeNode:
	-rm -f $(animCubeNode_OBJECTS)

Clean_animCubeNode:
	-rm -f $(animCubeNode_MAKEFILE).bak $(animCubeNode_OBJECTS) $(animCubeNode_PLUGIN)


plugins: $(animCubeNode_PLUGIN)
depend:	 depend_animCubeNode
clean:	 clean_animCubeNode
Clean:	 Clean_animCubeNode

