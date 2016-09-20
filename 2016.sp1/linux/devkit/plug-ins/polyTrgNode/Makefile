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

SRCDIR := $(TOP)/polyTrgNode
DSTDIR := $(TOP)/polyTrgNode

polyTrgNode_SOURCES  := $(TOP)/polyTrgNode/polyTrgNode.cpp
polyTrgNode_OBJECTS  := $(TOP)/polyTrgNode/polyTrgNode.o
polyTrgNode_PLUGIN   := $(DSTDIR)/polyTrgNode.$(EXT)
polyTrgNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(polyTrgNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(polyTrgNode_EXTRA_CFLAGS)
$(polyTrgNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(polyTrgNode_EXTRA_C++FLAGS)
$(polyTrgNode_OBJECTS): INCLUDES := $(INCLUDES) $(polyTrgNode_EXTRA_INCLUDES)

depend_polyTrgNode:     INCLUDES := $(INCLUDES) $(polyTrgNode_EXTRA_INCLUDES)

$(polyTrgNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(polyTrgNode_EXTRA_LFLAGS) 
$(polyTrgNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(polyTrgNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_polyTrgNode clean_polyTrgNode Clean_polyTrgNode


$(polyTrgNode_PLUGIN): $(polyTrgNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_polyTrgNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(polyTrgNode_SOURCES)

clean_polyTrgNode:
	-rm -f $(polyTrgNode_OBJECTS)

Clean_polyTrgNode:
	-rm -f $(polyTrgNode_MAKEFILE).bak $(polyTrgNode_OBJECTS) $(polyTrgNode_PLUGIN)


plugins: $(polyTrgNode_PLUGIN)
depend:	 depend_polyTrgNode
clean:	 clean_polyTrgNode
Clean:	 Clean_polyTrgNode

