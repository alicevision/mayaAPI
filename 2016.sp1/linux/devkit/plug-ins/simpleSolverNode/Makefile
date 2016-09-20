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

SRCDIR := $(TOP)/simpleSolverNode
DSTDIR := $(TOP)/simpleSolverNode

simpleSolverNode_SOURCES  := $(TOP)/simpleSolverNode/simpleSolverNode.cpp
simpleSolverNode_OBJECTS  := $(TOP)/simpleSolverNode/simpleSolverNode.o
simpleSolverNode_PLUGIN   := $(DSTDIR)/simpleSolverNode.$(EXT)
simpleSolverNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleSolverNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleSolverNode_EXTRA_CFLAGS)
$(simpleSolverNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleSolverNode_EXTRA_C++FLAGS)
$(simpleSolverNode_OBJECTS): INCLUDES := $(INCLUDES) $(simpleSolverNode_EXTRA_INCLUDES)

depend_simpleSolverNode:     INCLUDES := $(INCLUDES) $(simpleSolverNode_EXTRA_INCLUDES)

$(simpleSolverNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleSolverNode_EXTRA_LFLAGS) 
$(simpleSolverNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(simpleSolverNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleSolverNode clean_simpleSolverNode Clean_simpleSolverNode


$(simpleSolverNode_PLUGIN): $(simpleSolverNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleSolverNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleSolverNode_SOURCES)

clean_simpleSolverNode:
	-rm -f $(simpleSolverNode_OBJECTS)

Clean_simpleSolverNode:
	-rm -f $(simpleSolverNode_MAKEFILE).bak $(simpleSolverNode_OBJECTS) $(simpleSolverNode_PLUGIN)


plugins: $(simpleSolverNode_PLUGIN)
depend:	 depend_simpleSolverNode
clean:	 clean_simpleSolverNode
Clean:	 Clean_simpleSolverNode

