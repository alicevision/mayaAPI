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

SRCDIR := $(TOP)/stringFormatNode
DSTDIR := $(TOP)/stringFormatNode

stringFormatNode_SOURCES  := $(TOP)/stringFormatNode/stringFormatNode.cpp
stringFormatNode_OBJECTS  := $(TOP)/stringFormatNode/stringFormatNode.o
stringFormatNode_PLUGIN   := $(DSTDIR)/stringFormatNode.$(EXT)
stringFormatNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(stringFormatNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(stringFormatNode_EXTRA_CFLAGS)
$(stringFormatNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(stringFormatNode_EXTRA_C++FLAGS)
$(stringFormatNode_OBJECTS): INCLUDES := $(INCLUDES) $(stringFormatNode_EXTRA_INCLUDES)

depend_stringFormatNode:     INCLUDES := $(INCLUDES) $(stringFormatNode_EXTRA_INCLUDES)

$(stringFormatNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(stringFormatNode_EXTRA_LFLAGS) 
$(stringFormatNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(stringFormatNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_stringFormatNode clean_stringFormatNode Clean_stringFormatNode


$(stringFormatNode_PLUGIN): $(stringFormatNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_stringFormatNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(stringFormatNode_SOURCES)

clean_stringFormatNode:
	-rm -f $(stringFormatNode_OBJECTS)

Clean_stringFormatNode:
	-rm -f $(stringFormatNode_MAKEFILE).bak $(stringFormatNode_OBJECTS) $(stringFormatNode_PLUGIN)


plugins: $(stringFormatNode_PLUGIN)
depend:	 depend_stringFormatNode
clean:	 clean_stringFormatNode
Clean:	 Clean_stringFormatNode

