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

SRCDIR := $(TOP)/multiCurveNode
DSTDIR := $(TOP)/multiCurveNode

multiCurveNode_SOURCES  := $(TOP)/multiCurveNode/multiCurveNode.cpp
multiCurveNode_OBJECTS  := $(TOP)/multiCurveNode/multiCurveNode.o
multiCurveNode_PLUGIN   := $(DSTDIR)/multiCurveNode.$(EXT)
multiCurveNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(multiCurveNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(multiCurveNode_EXTRA_CFLAGS)
$(multiCurveNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(multiCurveNode_EXTRA_C++FLAGS)
$(multiCurveNode_OBJECTS): INCLUDES := $(INCLUDES) $(multiCurveNode_EXTRA_INCLUDES)

depend_multiCurveNode:     INCLUDES := $(INCLUDES) $(multiCurveNode_EXTRA_INCLUDES)

$(multiCurveNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(multiCurveNode_EXTRA_LFLAGS) 
$(multiCurveNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(multiCurveNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_multiCurveNode clean_multiCurveNode Clean_multiCurveNode


$(multiCurveNode_PLUGIN): $(multiCurveNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_multiCurveNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(multiCurveNode_SOURCES)

clean_multiCurveNode:
	-rm -f $(multiCurveNode_OBJECTS)

Clean_multiCurveNode:
	-rm -f $(multiCurveNode_MAKEFILE).bak $(multiCurveNode_OBJECTS) $(multiCurveNode_PLUGIN)


plugins: $(multiCurveNode_PLUGIN)
depend:	 depend_multiCurveNode
clean:	 clean_multiCurveNode
Clean:	 Clean_multiCurveNode

