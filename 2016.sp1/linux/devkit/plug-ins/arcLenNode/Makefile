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

SRCDIR := $(TOP)/arcLenNode
DSTDIR := $(TOP)/arcLenNode

arcLenNode_SOURCES  := $(TOP)/arcLenNode/arcLenNode.cpp
arcLenNode_OBJECTS  := $(TOP)/arcLenNode/arcLenNode.o
arcLenNode_PLUGIN   := $(DSTDIR)/arcLenNode.$(EXT)
arcLenNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(arcLenNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(arcLenNode_EXTRA_CFLAGS)
$(arcLenNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(arcLenNode_EXTRA_C++FLAGS)
$(arcLenNode_OBJECTS): INCLUDES := $(INCLUDES) $(arcLenNode_EXTRA_INCLUDES)

depend_arcLenNode:     INCLUDES := $(INCLUDES) $(arcLenNode_EXTRA_INCLUDES)

$(arcLenNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(arcLenNode_EXTRA_LFLAGS) 
$(arcLenNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(arcLenNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_arcLenNode clean_arcLenNode Clean_arcLenNode


$(arcLenNode_PLUGIN): $(arcLenNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_arcLenNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(arcLenNode_SOURCES)

clean_arcLenNode:
	-rm -f $(arcLenNode_OBJECTS)

Clean_arcLenNode:
	-rm -f $(arcLenNode_MAKEFILE).bak $(arcLenNode_OBJECTS) $(arcLenNode_PLUGIN)


plugins: $(arcLenNode_PLUGIN)
depend:	 depend_arcLenNode
clean:	 clean_arcLenNode
Clean:	 Clean_arcLenNode

