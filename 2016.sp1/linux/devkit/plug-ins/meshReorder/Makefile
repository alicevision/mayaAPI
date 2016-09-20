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

SRCDIR := $(TOP)/meshReorder
DSTDIR := $(TOP)/meshReorder

meshReorder_SOURCES  := $(TOP)/meshReorder/meshReorderCmd.cpp $(TOP)/meshReorder/meshReorderTool.cpp $(TOP)/meshReorder/meshRemapCmd.cpp $(TOP)/meshReorder/meshRemapTool.cpp $(TOP)/meshReorder/meshMapUtils.cpp $(TOP)/meshReorder/meshReorderPlugin.cpp
meshReorder_OBJECTS  := $(TOP)/meshReorder/meshReorderCmd.o $(TOP)/meshReorder/meshReorderTool.o $(TOP)/meshReorder/meshRemapCmd.o $(TOP)/meshReorder/meshRemapTool.o $(TOP)/meshReorder/meshMapUtils.o $(TOP)/meshReorder/meshReorderPlugin.o
meshReorder_PLUGIN   := $(DSTDIR)/meshReorder.$(EXT)
meshReorder_MAKEFILE := $(DSTDIR)/Makefile

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

$(meshReorder_OBJECTS): CFLAGS   := $(CFLAGS)   $(meshReorder_EXTRA_CFLAGS)
$(meshReorder_OBJECTS): C++FLAGS := $(C++FLAGS) $(meshReorder_EXTRA_C++FLAGS)
$(meshReorder_OBJECTS): INCLUDES := $(INCLUDES) $(meshReorder_EXTRA_INCLUDES)

depend_meshReorder:     INCLUDES := $(INCLUDES) $(meshReorder_EXTRA_INCLUDES)

$(meshReorder_PLUGIN):  LFLAGS   := $(LFLAGS) $(meshReorder_EXTRA_LFLAGS) 
$(meshReorder_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(meshReorder_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_meshReorder clean_meshReorder Clean_meshReorder


$(meshReorder_PLUGIN): $(meshReorder_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_meshReorder :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(meshReorder_SOURCES)

clean_meshReorder:
	-rm -f $(meshReorder_OBJECTS)

Clean_meshReorder:
	-rm -f $(meshReorder_MAKEFILE).bak $(meshReorder_OBJECTS) $(meshReorder_PLUGIN)


plugins: $(meshReorder_PLUGIN)
depend:	 depend_meshReorder
clean:	 clean_meshReorder
Clean:	 Clean_meshReorder

