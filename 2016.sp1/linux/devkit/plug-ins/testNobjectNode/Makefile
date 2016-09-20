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

SRCDIR := $(TOP)/testNobjectNode
DSTDIR := $(TOP)/testNobjectNode

testNobjectNode_SOURCES  := $(TOP)/testNobjectNode/testNobjectNode.cpp
testNobjectNode_OBJECTS  := $(TOP)/testNobjectNode/testNobjectNode.o
testNobjectNode_PLUGIN   := $(DSTDIR)/testNobjectNode.$(EXT)
testNobjectNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(testNobjectNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(testNobjectNode_EXTRA_CFLAGS)
$(testNobjectNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(testNobjectNode_EXTRA_C++FLAGS)
$(testNobjectNode_OBJECTS): INCLUDES := $(INCLUDES) $(testNobjectNode_EXTRA_INCLUDES)

depend_testNobjectNode:     INCLUDES := $(INCLUDES) $(testNobjectNode_EXTRA_INCLUDES)

$(testNobjectNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(testNobjectNode_EXTRA_LFLAGS) 
$(testNobjectNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(testNobjectNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_testNobjectNode clean_testNobjectNode Clean_testNobjectNode


$(testNobjectNode_PLUGIN): $(testNobjectNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_testNobjectNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(testNobjectNode_SOURCES)

clean_testNobjectNode:
	-rm -f $(testNobjectNode_OBJECTS)

Clean_testNobjectNode:
	-rm -f $(testNobjectNode_MAKEFILE).bak $(testNobjectNode_OBJECTS) $(testNobjectNode_PLUGIN)


plugins: $(testNobjectNode_PLUGIN)
depend:	 depend_testNobjectNode
clean:	 clean_testNobjectNode
Clean:	 Clean_testNobjectNode

