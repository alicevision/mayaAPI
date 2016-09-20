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

SRCDIR := $(TOP)/testNpassiveNode
DSTDIR := $(TOP)/testNpassiveNode

testNpassiveNode_SOURCES  := $(TOP)/testNpassiveNode/testNpassiveNode.cpp
testNpassiveNode_OBJECTS  := $(TOP)/testNpassiveNode/testNpassiveNode.o
testNpassiveNode_PLUGIN   := $(DSTDIR)/testNpassiveNode.$(EXT)
testNpassiveNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(testNpassiveNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(testNpassiveNode_EXTRA_CFLAGS)
$(testNpassiveNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(testNpassiveNode_EXTRA_C++FLAGS)
$(testNpassiveNode_OBJECTS): INCLUDES := $(INCLUDES) $(testNpassiveNode_EXTRA_INCLUDES)

depend_testNpassiveNode:     INCLUDES := $(INCLUDES) $(testNpassiveNode_EXTRA_INCLUDES)

$(testNpassiveNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(testNpassiveNode_EXTRA_LFLAGS) 
$(testNpassiveNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(testNpassiveNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_testNpassiveNode clean_testNpassiveNode Clean_testNpassiveNode


$(testNpassiveNode_PLUGIN): $(testNpassiveNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_testNpassiveNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(testNpassiveNode_SOURCES)

clean_testNpassiveNode:
	-rm -f $(testNpassiveNode_OBJECTS)

Clean_testNpassiveNode:
	-rm -f $(testNpassiveNode_MAKEFILE).bak $(testNpassiveNode_OBJECTS) $(testNpassiveNode_PLUGIN)


plugins: $(testNpassiveNode_PLUGIN)
depend:	 depend_testNpassiveNode
clean:	 clean_testNpassiveNode
Clean:	 Clean_testNpassiveNode

