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

SRCDIR := $(TOP)/offsetNode
DSTDIR := $(TOP)/offsetNode

offsetNode_SOURCES  := $(TOP)/offsetNode/offsetNode.cpp
offsetNode_OBJECTS  := $(TOP)/offsetNode/offsetNode.o
offsetNode_PLUGIN   := $(DSTDIR)/offsetNode.$(EXT)
offsetNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(offsetNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(offsetNode_EXTRA_CFLAGS)
$(offsetNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(offsetNode_EXTRA_C++FLAGS)
$(offsetNode_OBJECTS): INCLUDES := $(INCLUDES) $(offsetNode_EXTRA_INCLUDES)

depend_offsetNode:     INCLUDES := $(INCLUDES) $(offsetNode_EXTRA_INCLUDES)

$(offsetNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(offsetNode_EXTRA_LFLAGS) 
$(offsetNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation -lclew $(offsetNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_offsetNode clean_offsetNode Clean_offsetNode


$(offsetNode_PLUGIN): $(offsetNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_offsetNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(offsetNode_SOURCES)

clean_offsetNode:
	-rm -f $(offsetNode_OBJECTS)

Clean_offsetNode:
	-rm -f $(offsetNode_MAKEFILE).bak $(offsetNode_OBJECTS) $(offsetNode_PLUGIN)


plugins: $(offsetNode_PLUGIN)
depend:	 depend_offsetNode
clean:	 clean_offsetNode
Clean:	 Clean_offsetNode

