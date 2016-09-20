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

SRCDIR := $(TOP)/genericAttributeNode
DSTDIR := $(TOP)/genericAttributeNode

genericAttributeNode_SOURCES  := $(TOP)/genericAttributeNode/genericAttributeNode.cpp
genericAttributeNode_OBJECTS  := $(TOP)/genericAttributeNode/genericAttributeNode.o
genericAttributeNode_PLUGIN   := $(DSTDIR)/genericAttributeNode.$(EXT)
genericAttributeNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(genericAttributeNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(genericAttributeNode_EXTRA_CFLAGS)
$(genericAttributeNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(genericAttributeNode_EXTRA_C++FLAGS)
$(genericAttributeNode_OBJECTS): INCLUDES := $(INCLUDES) $(genericAttributeNode_EXTRA_INCLUDES)

depend_genericAttributeNode:     INCLUDES := $(INCLUDES) $(genericAttributeNode_EXTRA_INCLUDES)

$(genericAttributeNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(genericAttributeNode_EXTRA_LFLAGS) 
$(genericAttributeNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(genericAttributeNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_genericAttributeNode clean_genericAttributeNode Clean_genericAttributeNode


$(genericAttributeNode_PLUGIN): $(genericAttributeNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_genericAttributeNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(genericAttributeNode_SOURCES)

clean_genericAttributeNode:
	-rm -f $(genericAttributeNode_OBJECTS)

Clean_genericAttributeNode:
	-rm -f $(genericAttributeNode_MAKEFILE).bak $(genericAttributeNode_OBJECTS) $(genericAttributeNode_PLUGIN)


plugins: $(genericAttributeNode_PLUGIN)
depend:	 depend_genericAttributeNode
clean:	 clean_genericAttributeNode
Clean:	 Clean_genericAttributeNode

