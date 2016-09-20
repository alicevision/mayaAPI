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

SRCDIR := $(TOP)/fullLoftNode
DSTDIR := $(TOP)/fullLoftNode

fullLoftNode_SOURCES  := $(TOP)/fullLoftNode/fullLoftNode.cpp
fullLoftNode_OBJECTS  := $(TOP)/fullLoftNode/fullLoftNode.o
fullLoftNode_PLUGIN   := $(DSTDIR)/fullLoftNode.$(EXT)
fullLoftNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(fullLoftNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(fullLoftNode_EXTRA_CFLAGS)
$(fullLoftNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(fullLoftNode_EXTRA_C++FLAGS)
$(fullLoftNode_OBJECTS): INCLUDES := $(INCLUDES) $(fullLoftNode_EXTRA_INCLUDES)

depend_fullLoftNode:     INCLUDES := $(INCLUDES) $(fullLoftNode_EXTRA_INCLUDES)

$(fullLoftNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(fullLoftNode_EXTRA_LFLAGS) 
$(fullLoftNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(fullLoftNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_fullLoftNode clean_fullLoftNode Clean_fullLoftNode


$(fullLoftNode_PLUGIN): $(fullLoftNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_fullLoftNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(fullLoftNode_SOURCES)

clean_fullLoftNode:
	-rm -f $(fullLoftNode_OBJECTS)

Clean_fullLoftNode:
	-rm -f $(fullLoftNode_MAKEFILE).bak $(fullLoftNode_OBJECTS) $(fullLoftNode_PLUGIN)


plugins: $(fullLoftNode_PLUGIN)
depend:	 depend_fullLoftNode
clean:	 clean_fullLoftNode
Clean:	 Clean_fullLoftNode

