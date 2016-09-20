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

SRCDIR := $(TOP)/testNucleusNode
DSTDIR := $(TOP)/testNucleusNode

testNucleusNode_SOURCES  := $(TOP)/testNucleusNode/testNucleusNode.cpp
testNucleusNode_OBJECTS  := $(TOP)/testNucleusNode/testNucleusNode.o
testNucleusNode_PLUGIN   := $(DSTDIR)/testNucleusNode.$(EXT)
testNucleusNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(testNucleusNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(testNucleusNode_EXTRA_CFLAGS)
$(testNucleusNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(testNucleusNode_EXTRA_C++FLAGS)
$(testNucleusNode_OBJECTS): INCLUDES := $(INCLUDES) $(testNucleusNode_EXTRA_INCLUDES)

depend_testNucleusNode:     INCLUDES := $(INCLUDES) $(testNucleusNode_EXTRA_INCLUDES)

$(testNucleusNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(testNucleusNode_EXTRA_LFLAGS) 
$(testNucleusNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(testNucleusNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_testNucleusNode clean_testNucleusNode Clean_testNucleusNode


$(testNucleusNode_PLUGIN): $(testNucleusNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_testNucleusNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(testNucleusNode_SOURCES)

clean_testNucleusNode:
	-rm -f $(testNucleusNode_OBJECTS)

Clean_testNucleusNode:
	-rm -f $(testNucleusNode_MAKEFILE).bak $(testNucleusNode_OBJECTS) $(testNucleusNode_PLUGIN)


plugins: $(testNucleusNode_PLUGIN)
depend:	 depend_testNucleusNode
clean:	 clean_testNucleusNode
Clean:	 Clean_testNucleusNode

