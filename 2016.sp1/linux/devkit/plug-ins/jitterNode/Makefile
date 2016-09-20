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

SRCDIR := $(TOP)/jitterNode
DSTDIR := $(TOP)/jitterNode

jitterNode_SOURCES  := $(TOP)/jitterNode/jitterNode.cpp
jitterNode_OBJECTS  := $(TOP)/jitterNode/jitterNode.o
jitterNode_PLUGIN   := $(DSTDIR)/jitterNode.$(EXT)
jitterNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(jitterNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(jitterNode_EXTRA_CFLAGS)
$(jitterNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(jitterNode_EXTRA_C++FLAGS)
$(jitterNode_OBJECTS): INCLUDES := $(INCLUDES) $(jitterNode_EXTRA_INCLUDES)

depend_jitterNode:     INCLUDES := $(INCLUDES) $(jitterNode_EXTRA_INCLUDES)

$(jitterNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(jitterNode_EXTRA_LFLAGS) 
$(jitterNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(jitterNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_jitterNode clean_jitterNode Clean_jitterNode


$(jitterNode_PLUGIN): $(jitterNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_jitterNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(jitterNode_SOURCES)

clean_jitterNode:
	-rm -f $(jitterNode_OBJECTS)

Clean_jitterNode:
	-rm -f $(jitterNode_MAKEFILE).bak $(jitterNode_OBJECTS) $(jitterNode_PLUGIN)


plugins: $(jitterNode_PLUGIN)
depend:	 depend_jitterNode
clean:	 clean_jitterNode
Clean:	 Clean_jitterNode

