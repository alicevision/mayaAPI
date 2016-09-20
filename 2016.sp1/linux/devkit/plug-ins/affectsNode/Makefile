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

SRCDIR := $(TOP)/affectsNode
DSTDIR := $(TOP)/affectsNode

affectsNode_SOURCES  := $(TOP)/affectsNode/affectsNode.cpp
affectsNode_OBJECTS  := $(TOP)/affectsNode/affectsNode.o
affectsNode_PLUGIN   := $(DSTDIR)/affectsNode.$(EXT)
affectsNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(affectsNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(affectsNode_EXTRA_CFLAGS)
$(affectsNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(affectsNode_EXTRA_C++FLAGS)
$(affectsNode_OBJECTS): INCLUDES := $(INCLUDES) $(affectsNode_EXTRA_INCLUDES)

depend_affectsNode:     INCLUDES := $(INCLUDES) $(affectsNode_EXTRA_INCLUDES)

$(affectsNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(affectsNode_EXTRA_LFLAGS) 
$(affectsNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(affectsNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_affectsNode clean_affectsNode Clean_affectsNode


$(affectsNode_PLUGIN): $(affectsNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_affectsNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(affectsNode_SOURCES)

clean_affectsNode:
	-rm -f $(affectsNode_OBJECTS)

Clean_affectsNode:
	-rm -f $(affectsNode_MAKEFILE).bak $(affectsNode_OBJECTS) $(affectsNode_PLUGIN)


plugins: $(affectsNode_PLUGIN)
depend:	 depend_affectsNode
clean:	 clean_affectsNode
Clean:	 Clean_affectsNode

