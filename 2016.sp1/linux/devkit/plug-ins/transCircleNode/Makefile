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

SRCDIR := $(TOP)/transCircleNode
DSTDIR := $(TOP)/transCircleNode

transCircleNode_SOURCES  := $(TOP)/transCircleNode/transCircleNode.cpp
transCircleNode_OBJECTS  := $(TOP)/transCircleNode/transCircleNode.o
transCircleNode_PLUGIN   := $(DSTDIR)/transCircleNode.$(EXT)
transCircleNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(transCircleNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(transCircleNode_EXTRA_CFLAGS)
$(transCircleNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(transCircleNode_EXTRA_C++FLAGS)
$(transCircleNode_OBJECTS): INCLUDES := $(INCLUDES) $(transCircleNode_EXTRA_INCLUDES)

depend_transCircleNode:     INCLUDES := $(INCLUDES) $(transCircleNode_EXTRA_INCLUDES)

$(transCircleNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(transCircleNode_EXTRA_LFLAGS) 
$(transCircleNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(transCircleNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_transCircleNode clean_transCircleNode Clean_transCircleNode


$(transCircleNode_PLUGIN): $(transCircleNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_transCircleNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(transCircleNode_SOURCES)

clean_transCircleNode:
	-rm -f $(transCircleNode_OBJECTS)

Clean_transCircleNode:
	-rm -f $(transCircleNode_MAKEFILE).bak $(transCircleNode_OBJECTS) $(transCircleNode_PLUGIN)


plugins: $(transCircleNode_PLUGIN)
depend:	 depend_transCircleNode
clean:	 clean_transCircleNode
Clean:	 Clean_transCircleNode

