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

SRCDIR := $(TOP)/cvColorNode
DSTDIR := $(TOP)/cvColorNode

cvColorNode_SOURCES  := $(TOP)/cvColorNode/cvColorNode.cpp
cvColorNode_OBJECTS  := $(TOP)/cvColorNode/cvColorNode.o
cvColorNode_PLUGIN   := $(DSTDIR)/cvColorNode.$(EXT)
cvColorNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(cvColorNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(cvColorNode_EXTRA_CFLAGS)
$(cvColorNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(cvColorNode_EXTRA_C++FLAGS)
$(cvColorNode_OBJECTS): INCLUDES := $(INCLUDES) $(cvColorNode_EXTRA_INCLUDES)

depend_cvColorNode:     INCLUDES := $(INCLUDES) $(cvColorNode_EXTRA_INCLUDES)

$(cvColorNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(cvColorNode_EXTRA_LFLAGS) 
$(cvColorNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL -lGLU $(cvColorNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cvColorNode clean_cvColorNode Clean_cvColorNode


$(cvColorNode_PLUGIN): $(cvColorNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cvColorNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cvColorNode_SOURCES)

clean_cvColorNode:
	-rm -f $(cvColorNode_OBJECTS)

Clean_cvColorNode:
	-rm -f $(cvColorNode_MAKEFILE).bak $(cvColorNode_OBJECTS) $(cvColorNode_PLUGIN)


plugins: $(cvColorNode_PLUGIN)
depend:	 depend_cvColorNode
clean:	 clean_cvColorNode
Clean:	 Clean_cvColorNode

