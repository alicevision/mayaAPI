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

SRCDIR := $(TOP)/sineNode
DSTDIR := $(TOP)/sineNode

sineNode_SOURCES  := $(TOP)/sineNode/sineNode.cpp
sineNode_OBJECTS  := $(TOP)/sineNode/sineNode.o
sineNode_PLUGIN   := $(DSTDIR)/sineNode.$(EXT)
sineNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(sineNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(sineNode_EXTRA_CFLAGS)
$(sineNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(sineNode_EXTRA_C++FLAGS)
$(sineNode_OBJECTS): INCLUDES := $(INCLUDES) $(sineNode_EXTRA_INCLUDES)

depend_sineNode:     INCLUDES := $(INCLUDES) $(sineNode_EXTRA_INCLUDES)

$(sineNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(sineNode_EXTRA_LFLAGS) 
$(sineNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(sineNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_sineNode clean_sineNode Clean_sineNode


$(sineNode_PLUGIN): $(sineNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_sineNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(sineNode_SOURCES)

clean_sineNode:
	-rm -f $(sineNode_OBJECTS)

Clean_sineNode:
	-rm -f $(sineNode_MAKEFILE).bak $(sineNode_OBJECTS) $(sineNode_PLUGIN)


plugins: $(sineNode_PLUGIN)
depend:	 depend_sineNode
clean:	 clean_sineNode
Clean:	 Clean_sineNode

