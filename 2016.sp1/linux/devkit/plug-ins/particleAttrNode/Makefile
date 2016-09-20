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

SRCDIR := $(TOP)/particleAttrNode
DSTDIR := $(TOP)/particleAttrNode

particleAttrNode_SOURCES  := $(TOP)/particleAttrNode/particleAttrNodeMain.cpp $(TOP)/particleAttrNode/particleAttrNode.cpp
particleAttrNode_OBJECTS  := $(TOP)/particleAttrNode/particleAttrNodeMain.o $(TOP)/particleAttrNode/particleAttrNode.o
particleAttrNode_PLUGIN   := $(DSTDIR)/particleAttrNode.$(EXT)
particleAttrNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(particleAttrNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(particleAttrNode_EXTRA_CFLAGS)
$(particleAttrNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(particleAttrNode_EXTRA_C++FLAGS)
$(particleAttrNode_OBJECTS): INCLUDES := $(INCLUDES) $(particleAttrNode_EXTRA_INCLUDES)

depend_particleAttrNode:     INCLUDES := $(INCLUDES) $(particleAttrNode_EXTRA_INCLUDES)

$(particleAttrNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(particleAttrNode_EXTRA_LFLAGS) 
$(particleAttrNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(particleAttrNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_particleAttrNode clean_particleAttrNode Clean_particleAttrNode


$(particleAttrNode_PLUGIN): $(particleAttrNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_particleAttrNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(particleAttrNode_SOURCES)

clean_particleAttrNode:
	-rm -f $(particleAttrNode_OBJECTS)

Clean_particleAttrNode:
	-rm -f $(particleAttrNode_MAKEFILE).bak $(particleAttrNode_OBJECTS) $(particleAttrNode_PLUGIN)


plugins: $(particleAttrNode_PLUGIN)
depend:	 depend_particleAttrNode
clean:	 clean_particleAttrNode
Clean:	 Clean_particleAttrNode

