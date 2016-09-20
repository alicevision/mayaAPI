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

SRCDIR := $(TOP)/curvedArrowsNode
DSTDIR := $(TOP)/curvedArrowsNode

curvedArrowsNode_SOURCES  := $(TOP)/curvedArrowsNode/curvedArrowsNode.cpp
curvedArrowsNode_OBJECTS  := $(TOP)/curvedArrowsNode/curvedArrowsNode.o
curvedArrowsNode_PLUGIN   := $(DSTDIR)/curvedArrowsNode.$(EXT)
curvedArrowsNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(curvedArrowsNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(curvedArrowsNode_EXTRA_CFLAGS)
$(curvedArrowsNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(curvedArrowsNode_EXTRA_C++FLAGS)
$(curvedArrowsNode_OBJECTS): INCLUDES := $(INCLUDES) $(curvedArrowsNode_EXTRA_INCLUDES)

depend_curvedArrowsNode:     INCLUDES := $(INCLUDES) $(curvedArrowsNode_EXTRA_INCLUDES)

$(curvedArrowsNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(curvedArrowsNode_EXTRA_LFLAGS) 
$(curvedArrowsNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL $(curvedArrowsNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_curvedArrowsNode clean_curvedArrowsNode Clean_curvedArrowsNode


$(curvedArrowsNode_PLUGIN): $(curvedArrowsNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_curvedArrowsNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(curvedArrowsNode_SOURCES)

clean_curvedArrowsNode:
	-rm -f $(curvedArrowsNode_OBJECTS)

Clean_curvedArrowsNode:
	-rm -f $(curvedArrowsNode_MAKEFILE).bak $(curvedArrowsNode_OBJECTS) $(curvedArrowsNode_PLUGIN)


plugins: $(curvedArrowsNode_PLUGIN)
depend:	 depend_curvedArrowsNode
clean:	 clean_curvedArrowsNode
Clean:	 Clean_curvedArrowsNode

