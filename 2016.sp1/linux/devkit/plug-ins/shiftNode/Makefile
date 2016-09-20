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

SRCDIR := $(TOP)/shiftNode
DSTDIR := $(TOP)/shiftNode

shiftNode_SOURCES  := $(TOP)/shiftNode/shiftNode.cpp
shiftNode_OBJECTS  := $(TOP)/shiftNode/shiftNode.o
shiftNode_PLUGIN   := $(DSTDIR)/shiftNode.$(EXT)
shiftNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(shiftNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(shiftNode_EXTRA_CFLAGS)
$(shiftNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(shiftNode_EXTRA_C++FLAGS)
$(shiftNode_OBJECTS): INCLUDES := $(INCLUDES) $(shiftNode_EXTRA_INCLUDES)

depend_shiftNode:     INCLUDES := $(INCLUDES) $(shiftNode_EXTRA_INCLUDES)

$(shiftNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(shiftNode_EXTRA_LFLAGS) 
$(shiftNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(shiftNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_shiftNode clean_shiftNode Clean_shiftNode


$(shiftNode_PLUGIN): $(shiftNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_shiftNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(shiftNode_SOURCES)

clean_shiftNode:
	-rm -f $(shiftNode_OBJECTS)

Clean_shiftNode:
	-rm -f $(shiftNode_MAKEFILE).bak $(shiftNode_OBJECTS) $(shiftNode_PLUGIN)


plugins: $(shiftNode_PLUGIN)
depend:	 depend_shiftNode
clean:	 clean_shiftNode
Clean:	 Clean_shiftNode

