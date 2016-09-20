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

SRCDIR := $(TOP)/buildRotationNode
DSTDIR := $(TOP)/buildRotationNode

buildRotationNode_SOURCES  := $(TOP)/buildRotationNode/buildRotationNode.cpp
buildRotationNode_OBJECTS  := $(TOP)/buildRotationNode/buildRotationNode.o
buildRotationNode_PLUGIN   := $(DSTDIR)/buildRotationNode.$(EXT)
buildRotationNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(buildRotationNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(buildRotationNode_EXTRA_CFLAGS)
$(buildRotationNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(buildRotationNode_EXTRA_C++FLAGS)
$(buildRotationNode_OBJECTS): INCLUDES := $(INCLUDES) $(buildRotationNode_EXTRA_INCLUDES)

depend_buildRotationNode:     INCLUDES := $(INCLUDES) $(buildRotationNode_EXTRA_INCLUDES)

$(buildRotationNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(buildRotationNode_EXTRA_LFLAGS) 
$(buildRotationNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(buildRotationNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_buildRotationNode clean_buildRotationNode Clean_buildRotationNode


$(buildRotationNode_PLUGIN): $(buildRotationNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_buildRotationNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(buildRotationNode_SOURCES)

clean_buildRotationNode:
	-rm -f $(buildRotationNode_OBJECTS)

Clean_buildRotationNode:
	-rm -f $(buildRotationNode_MAKEFILE).bak $(buildRotationNode_OBJECTS) $(buildRotationNode_PLUGIN)


plugins: $(buildRotationNode_PLUGIN)
depend:	 depend_buildRotationNode
clean:	 clean_buildRotationNode
Clean:	 Clean_buildRotationNode

