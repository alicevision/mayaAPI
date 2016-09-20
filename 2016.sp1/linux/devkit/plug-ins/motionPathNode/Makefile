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

SRCDIR := $(TOP)/motionPathNode
DSTDIR := $(TOP)/motionPathNode

motionPathNode_SOURCES  := $(TOP)/motionPathNode/motionPathNode.cpp
motionPathNode_OBJECTS  := $(TOP)/motionPathNode/motionPathNode.o
motionPathNode_PLUGIN   := $(DSTDIR)/motionPathNode.$(EXT)
motionPathNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(motionPathNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(motionPathNode_EXTRA_CFLAGS)
$(motionPathNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(motionPathNode_EXTRA_C++FLAGS)
$(motionPathNode_OBJECTS): INCLUDES := $(INCLUDES) $(motionPathNode_EXTRA_INCLUDES)

depend_motionPathNode:     INCLUDES := $(INCLUDES) $(motionPathNode_EXTRA_INCLUDES)

$(motionPathNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(motionPathNode_EXTRA_LFLAGS) 
$(motionPathNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaAnim -lOpenMaya -lFoundation $(motionPathNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_motionPathNode clean_motionPathNode Clean_motionPathNode


$(motionPathNode_PLUGIN): $(motionPathNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_motionPathNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(motionPathNode_SOURCES)

clean_motionPathNode:
	-rm -f $(motionPathNode_OBJECTS)

Clean_motionPathNode:
	-rm -f $(motionPathNode_MAKEFILE).bak $(motionPathNode_OBJECTS) $(motionPathNode_PLUGIN)


plugins: $(motionPathNode_PLUGIN)
depend:	 depend_motionPathNode
clean:	 clean_motionPathNode
Clean:	 Clean_motionPathNode

