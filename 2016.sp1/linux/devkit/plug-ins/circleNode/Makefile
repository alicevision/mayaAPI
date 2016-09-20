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

SRCDIR := $(TOP)/circleNode
DSTDIR := $(TOP)/circleNode

circleNode_SOURCES  := $(TOP)/circleNode/circleNode.cpp
circleNode_OBJECTS  := $(TOP)/circleNode/circleNode.o
circleNode_PLUGIN   := $(DSTDIR)/circleNode.$(EXT)
circleNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(circleNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(circleNode_EXTRA_CFLAGS)
$(circleNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(circleNode_EXTRA_C++FLAGS)
$(circleNode_OBJECTS): INCLUDES := $(INCLUDES) $(circleNode_EXTRA_INCLUDES)

depend_circleNode:     INCLUDES := $(INCLUDES) $(circleNode_EXTRA_INCLUDES)

$(circleNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(circleNode_EXTRA_LFLAGS) 
$(circleNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(circleNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_circleNode clean_circleNode Clean_circleNode


$(circleNode_PLUGIN): $(circleNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_circleNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(circleNode_SOURCES)

clean_circleNode:
	-rm -f $(circleNode_OBJECTS)

Clean_circleNode:
	-rm -f $(circleNode_MAKEFILE).bak $(circleNode_OBJECTS) $(circleNode_PLUGIN)


plugins: $(circleNode_PLUGIN)
depend:	 depend_circleNode
clean:	 clean_circleNode
Clean:	 Clean_circleNode

