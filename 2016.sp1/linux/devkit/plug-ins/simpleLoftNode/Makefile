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

SRCDIR := $(TOP)/simpleLoftNode
DSTDIR := $(TOP)/simpleLoftNode

simpleLoftNode_SOURCES  := $(TOP)/simpleLoftNode/simpleLoftNode.cpp
simpleLoftNode_OBJECTS  := $(TOP)/simpleLoftNode/simpleLoftNode.o
simpleLoftNode_PLUGIN   := $(DSTDIR)/simpleLoftNode.$(EXT)
simpleLoftNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleLoftNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleLoftNode_EXTRA_CFLAGS)
$(simpleLoftNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleLoftNode_EXTRA_C++FLAGS)
$(simpleLoftNode_OBJECTS): INCLUDES := $(INCLUDES) $(simpleLoftNode_EXTRA_INCLUDES)

depend_simpleLoftNode:     INCLUDES := $(INCLUDES) $(simpleLoftNode_EXTRA_INCLUDES)

$(simpleLoftNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleLoftNode_EXTRA_LFLAGS) 
$(simpleLoftNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(simpleLoftNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleLoftNode clean_simpleLoftNode Clean_simpleLoftNode


$(simpleLoftNode_PLUGIN): $(simpleLoftNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleLoftNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleLoftNode_SOURCES)

clean_simpleLoftNode:
	-rm -f $(simpleLoftNode_OBJECTS)

Clean_simpleLoftNode:
	-rm -f $(simpleLoftNode_MAKEFILE).bak $(simpleLoftNode_OBJECTS) $(simpleLoftNode_PLUGIN)


plugins: $(simpleLoftNode_PLUGIN)
depend:	 depend_simpleLoftNode
clean:	 clean_simpleLoftNode
Clean:	 Clean_simpleLoftNode

