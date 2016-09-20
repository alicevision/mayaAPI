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

SRCDIR := $(TOP)/identityNode
DSTDIR := $(TOP)/identityNode

identityNode_SOURCES  := $(TOP)/identityNode/identityNode.cpp
identityNode_OBJECTS  := $(TOP)/identityNode/identityNode.o
identityNode_PLUGIN   := $(DSTDIR)/identityNode.$(EXT)
identityNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(identityNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(identityNode_EXTRA_CFLAGS)
$(identityNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(identityNode_EXTRA_C++FLAGS)
$(identityNode_OBJECTS): INCLUDES := $(INCLUDES) $(identityNode_EXTRA_INCLUDES)

depend_identityNode:     INCLUDES := $(INCLUDES) $(identityNode_EXTRA_INCLUDES)

$(identityNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(identityNode_EXTRA_LFLAGS) 
$(identityNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation -lclew $(identityNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_identityNode clean_identityNode Clean_identityNode


$(identityNode_PLUGIN): $(identityNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_identityNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(identityNode_SOURCES)

clean_identityNode:
	-rm -f $(identityNode_OBJECTS)

Clean_identityNode:
	-rm -f $(identityNode_MAKEFILE).bak $(identityNode_OBJECTS) $(identityNode_PLUGIN)


plugins: $(identityNode_PLUGIN)
depend:	 depend_identityNode
clean:	 clean_identityNode
Clean:	 Clean_identityNode

