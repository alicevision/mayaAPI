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

SRCDIR := $(TOP)/renderAccessNode
DSTDIR := $(TOP)/renderAccessNode

renderAccessNode_SOURCES  := $(TOP)/renderAccessNode/renderAccessNode.cpp
renderAccessNode_OBJECTS  := $(TOP)/renderAccessNode/renderAccessNode.o
renderAccessNode_PLUGIN   := $(DSTDIR)/renderAccessNode.$(EXT)
renderAccessNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(renderAccessNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(renderAccessNode_EXTRA_CFLAGS)
$(renderAccessNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(renderAccessNode_EXTRA_C++FLAGS)
$(renderAccessNode_OBJECTS): INCLUDES := $(INCLUDES) $(renderAccessNode_EXTRA_INCLUDES)

depend_renderAccessNode:     INCLUDES := $(INCLUDES) $(renderAccessNode_EXTRA_INCLUDES)

$(renderAccessNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(renderAccessNode_EXTRA_LFLAGS) 
$(renderAccessNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(renderAccessNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_renderAccessNode clean_renderAccessNode Clean_renderAccessNode


$(renderAccessNode_PLUGIN): $(renderAccessNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_renderAccessNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(renderAccessNode_SOURCES)

clean_renderAccessNode:
	-rm -f $(renderAccessNode_OBJECTS)

Clean_renderAccessNode:
	-rm -f $(renderAccessNode_MAKEFILE).bak $(renderAccessNode_OBJECTS) $(renderAccessNode_PLUGIN)


plugins: $(renderAccessNode_PLUGIN)
depend:	 depend_renderAccessNode
clean:	 clean_renderAccessNode
Clean:	 Clean_renderAccessNode

