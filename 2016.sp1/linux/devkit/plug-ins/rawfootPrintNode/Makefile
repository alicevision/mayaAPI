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

SRCDIR := $(TOP)/rawfootPrintNode
DSTDIR := $(TOP)/rawfootPrintNode

rawfootPrintNode_SOURCES  := $(TOP)/rawfootPrintNode/rawfootPrintNode.cpp
rawfootPrintNode_OBJECTS  := $(TOP)/rawfootPrintNode/rawfootPrintNode.o
rawfootPrintNode_PLUGIN   := $(DSTDIR)/rawfootPrintNode.$(EXT)
rawfootPrintNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(rawfootPrintNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(rawfootPrintNode_EXTRA_CFLAGS)
$(rawfootPrintNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(rawfootPrintNode_EXTRA_C++FLAGS)
$(rawfootPrintNode_OBJECTS): INCLUDES := $(INCLUDES) $(rawfootPrintNode_EXTRA_INCLUDES)

depend_rawfootPrintNode:     INCLUDES := $(INCLUDES) $(rawfootPrintNode_EXTRA_INCLUDES)

$(rawfootPrintNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(rawfootPrintNode_EXTRA_LFLAGS) 
$(rawfootPrintNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lFoundation -lOpenMayaAnim -lGL $(rawfootPrintNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_rawfootPrintNode clean_rawfootPrintNode Clean_rawfootPrintNode


$(rawfootPrintNode_PLUGIN): $(rawfootPrintNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_rawfootPrintNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(rawfootPrintNode_SOURCES)

clean_rawfootPrintNode:
	-rm -f $(rawfootPrintNode_OBJECTS)

Clean_rawfootPrintNode:
	-rm -f $(rawfootPrintNode_MAKEFILE).bak $(rawfootPrintNode_OBJECTS) $(rawfootPrintNode_PLUGIN)


plugins: $(rawfootPrintNode_PLUGIN)
depend:	 depend_rawfootPrintNode
clean:	 clean_rawfootPrintNode
Clean:	 Clean_rawfootPrintNode

