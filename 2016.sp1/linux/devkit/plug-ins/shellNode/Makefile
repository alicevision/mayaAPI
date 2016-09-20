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

SRCDIR := $(TOP)/shellNode
DSTDIR := $(TOP)/shellNode

shellNode_SOURCES  := $(TOP)/shellNode/shellNode.cpp
shellNode_OBJECTS  := $(TOP)/shellNode/shellNode.o
shellNode_PLUGIN   := $(DSTDIR)/shellNode.$(EXT)
shellNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(shellNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(shellNode_EXTRA_CFLAGS)
$(shellNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(shellNode_EXTRA_C++FLAGS)
$(shellNode_OBJECTS): INCLUDES := $(INCLUDES) $(shellNode_EXTRA_INCLUDES)

depend_shellNode:     INCLUDES := $(INCLUDES) $(shellNode_EXTRA_INCLUDES)

$(shellNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(shellNode_EXTRA_LFLAGS) 
$(shellNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(shellNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_shellNode clean_shellNode Clean_shellNode


$(shellNode_PLUGIN): $(shellNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_shellNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(shellNode_SOURCES)

clean_shellNode:
	-rm -f $(shellNode_OBJECTS)

Clean_shellNode:
	-rm -f $(shellNode_MAKEFILE).bak $(shellNode_OBJECTS) $(shellNode_PLUGIN)


plugins: $(shellNode_PLUGIN)
depend:	 depend_shellNode
clean:	 clean_shellNode
Clean:	 Clean_shellNode

