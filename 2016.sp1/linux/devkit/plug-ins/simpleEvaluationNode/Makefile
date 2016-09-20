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

SRCDIR := $(TOP)/simpleEvaluationNode
DSTDIR := $(TOP)/simpleEvaluationNode

simpleEvaluationNode_SOURCES  := $(TOP)/simpleEvaluationNode/simpleEvaluationNode.cpp
simpleEvaluationNode_OBJECTS  := $(TOP)/simpleEvaluationNode/simpleEvaluationNode.o
simpleEvaluationNode_PLUGIN   := $(DSTDIR)/simpleEvaluationNode.$(EXT)
simpleEvaluationNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleEvaluationNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleEvaluationNode_EXTRA_CFLAGS)
$(simpleEvaluationNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleEvaluationNode_EXTRA_C++FLAGS)
$(simpleEvaluationNode_OBJECTS): INCLUDES := $(INCLUDES) $(simpleEvaluationNode_EXTRA_INCLUDES)

depend_simpleEvaluationNode:     INCLUDES := $(INCLUDES) $(simpleEvaluationNode_EXTRA_INCLUDES)

$(simpleEvaluationNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleEvaluationNode_EXTRA_LFLAGS) 
$(simpleEvaluationNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(simpleEvaluationNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleEvaluationNode clean_simpleEvaluationNode Clean_simpleEvaluationNode


$(simpleEvaluationNode_PLUGIN): $(simpleEvaluationNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleEvaluationNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleEvaluationNode_SOURCES)

clean_simpleEvaluationNode:
	-rm -f $(simpleEvaluationNode_OBJECTS)

Clean_simpleEvaluationNode:
	-rm -f $(simpleEvaluationNode_MAKEFILE).bak $(simpleEvaluationNode_OBJECTS) $(simpleEvaluationNode_PLUGIN)


plugins: $(simpleEvaluationNode_PLUGIN)
depend:	 depend_simpleEvaluationNode
clean:	 clean_simpleEvaluationNode
Clean:	 Clean_simpleEvaluationNode

