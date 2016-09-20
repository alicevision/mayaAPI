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

SRCDIR := $(TOP)/testNsolverNode
DSTDIR := $(TOP)/testNsolverNode

testNsolverNode_SOURCES  := $(TOP)/testNsolverNode/testNsolverNode.cpp
testNsolverNode_OBJECTS  := $(TOP)/testNsolverNode/testNsolverNode.o
testNsolverNode_PLUGIN   := $(DSTDIR)/testNsolverNode.$(EXT)
testNsolverNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(testNsolverNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(testNsolverNode_EXTRA_CFLAGS)
$(testNsolverNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(testNsolverNode_EXTRA_C++FLAGS)
$(testNsolverNode_OBJECTS): INCLUDES := $(INCLUDES) $(testNsolverNode_EXTRA_INCLUDES)

depend_testNsolverNode:     INCLUDES := $(INCLUDES) $(testNsolverNode_EXTRA_INCLUDES)

$(testNsolverNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(testNsolverNode_EXTRA_LFLAGS) 
$(testNsolverNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(testNsolverNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_testNsolverNode clean_testNsolverNode Clean_testNsolverNode


$(testNsolverNode_PLUGIN): $(testNsolverNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_testNsolverNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(testNsolverNode_SOURCES)

clean_testNsolverNode:
	-rm -f $(testNsolverNode_OBJECTS)

Clean_testNsolverNode:
	-rm -f $(testNsolverNode_MAKEFILE).bak $(testNsolverNode_OBJECTS) $(testNsolverNode_PLUGIN)


plugins: $(testNsolverNode_PLUGIN)
depend:	 depend_testNsolverNode
clean:	 clean_testNsolverNode
Clean:	 Clean_testNsolverNode

