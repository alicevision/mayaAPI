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

SRCDIR := $(TOP)/weightListNode
DSTDIR := $(TOP)/weightListNode

weightListNode_SOURCES  := $(TOP)/weightListNode/weightListNode.cpp
weightListNode_OBJECTS  := $(TOP)/weightListNode/weightListNode.o
weightListNode_PLUGIN   := $(DSTDIR)/weightListNode.$(EXT)
weightListNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(weightListNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(weightListNode_EXTRA_CFLAGS)
$(weightListNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(weightListNode_EXTRA_C++FLAGS)
$(weightListNode_OBJECTS): INCLUDES := $(INCLUDES) $(weightListNode_EXTRA_INCLUDES)

depend_weightListNode:     INCLUDES := $(INCLUDES) $(weightListNode_EXTRA_INCLUDES)

$(weightListNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(weightListNode_EXTRA_LFLAGS) 
$(weightListNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(weightListNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_weightListNode clean_weightListNode Clean_weightListNode


$(weightListNode_PLUGIN): $(weightListNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_weightListNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(weightListNode_SOURCES)

clean_weightListNode:
	-rm -f $(weightListNode_OBJECTS)

Clean_weightListNode:
	-rm -f $(weightListNode_MAKEFILE).bak $(weightListNode_OBJECTS) $(weightListNode_PLUGIN)


plugins: $(weightListNode_PLUGIN)
depend:	 depend_weightListNode
clean:	 clean_weightListNode
Clean:	 Clean_weightListNode

