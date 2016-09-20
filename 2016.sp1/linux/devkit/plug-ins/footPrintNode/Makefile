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

SRCDIR := $(TOP)/footPrintNode
DSTDIR := $(TOP)/footPrintNode

footPrintNode_SOURCES  := $(TOP)/footPrintNode/footPrintNode.cpp
footPrintNode_OBJECTS  := $(TOP)/footPrintNode/footPrintNode.o
footPrintNode_PLUGIN   := $(DSTDIR)/footPrintNode.$(EXT)
footPrintNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(footPrintNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(footPrintNode_EXTRA_CFLAGS)
$(footPrintNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(footPrintNode_EXTRA_C++FLAGS)
$(footPrintNode_OBJECTS): INCLUDES := $(INCLUDES) $(footPrintNode_EXTRA_INCLUDES)

depend_footPrintNode:     INCLUDES := $(INCLUDES) $(footPrintNode_EXTRA_INCLUDES)

$(footPrintNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(footPrintNode_EXTRA_LFLAGS) 
$(footPrintNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lFoundation -lGL $(footPrintNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_footPrintNode clean_footPrintNode Clean_footPrintNode


$(footPrintNode_PLUGIN): $(footPrintNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_footPrintNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(footPrintNode_SOURCES)

clean_footPrintNode:
	-rm -f $(footPrintNode_OBJECTS)

Clean_footPrintNode:
	-rm -f $(footPrintNode_MAKEFILE).bak $(footPrintNode_OBJECTS) $(footPrintNode_PLUGIN)


plugins: $(footPrintNode_PLUGIN)
depend:	 depend_footPrintNode
clean:	 clean_footPrintNode
Clean:	 Clean_footPrintNode

