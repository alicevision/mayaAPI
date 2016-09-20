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

SRCDIR := $(TOP)/pointOnSubdNode
DSTDIR := $(TOP)/pointOnSubdNode

pointOnSubdNode_SOURCES  := $(TOP)/pointOnSubdNode/pointOnSubdNode.cpp
pointOnSubdNode_OBJECTS  := $(TOP)/pointOnSubdNode/pointOnSubdNode.o
pointOnSubdNode_PLUGIN   := $(DSTDIR)/pointOnSubdNode.$(EXT)
pointOnSubdNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(pointOnSubdNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(pointOnSubdNode_EXTRA_CFLAGS)
$(pointOnSubdNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(pointOnSubdNode_EXTRA_C++FLAGS)
$(pointOnSubdNode_OBJECTS): INCLUDES := $(INCLUDES) $(pointOnSubdNode_EXTRA_INCLUDES)

depend_pointOnSubdNode:     INCLUDES := $(INCLUDES) $(pointOnSubdNode_EXTRA_INCLUDES)

$(pointOnSubdNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(pointOnSubdNode_EXTRA_LFLAGS) 
$(pointOnSubdNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(pointOnSubdNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_pointOnSubdNode clean_pointOnSubdNode Clean_pointOnSubdNode


$(pointOnSubdNode_PLUGIN): $(pointOnSubdNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_pointOnSubdNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(pointOnSubdNode_SOURCES)

clean_pointOnSubdNode:
	-rm -f $(pointOnSubdNode_OBJECTS)

Clean_pointOnSubdNode:
	-rm -f $(pointOnSubdNode_MAKEFILE).bak $(pointOnSubdNode_OBJECTS) $(pointOnSubdNode_PLUGIN)


plugins: $(pointOnSubdNode_PLUGIN)
depend:	 depend_pointOnSubdNode
clean:	 clean_pointOnSubdNode
Clean:	 Clean_pointOnSubdNode

