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

SRCDIR := $(TOP)/yTwistNode
DSTDIR := $(TOP)/yTwistNode

yTwistNode_SOURCES  := $(TOP)/yTwistNode/yTwistNode.cpp
yTwistNode_OBJECTS  := $(TOP)/yTwistNode/yTwistNode.o
yTwistNode_PLUGIN   := $(DSTDIR)/yTwistNode.$(EXT)
yTwistNode_MAKEFILE := $(DSTDIR)/Makefile

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

$(yTwistNode_OBJECTS): CFLAGS   := $(CFLAGS)   $(yTwistNode_EXTRA_CFLAGS)
$(yTwistNode_OBJECTS): C++FLAGS := $(C++FLAGS) $(yTwistNode_EXTRA_C++FLAGS)
$(yTwistNode_OBJECTS): INCLUDES := $(INCLUDES) $(yTwistNode_EXTRA_INCLUDES)

depend_yTwistNode:     INCLUDES := $(INCLUDES) $(yTwistNode_EXTRA_INCLUDES)

$(yTwistNode_PLUGIN):  LFLAGS   := $(LFLAGS) $(yTwistNode_EXTRA_LFLAGS) 
$(yTwistNode_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(yTwistNode_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_yTwistNode clean_yTwistNode Clean_yTwistNode


$(yTwistNode_PLUGIN): $(yTwistNode_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_yTwistNode :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(yTwistNode_SOURCES)

clean_yTwistNode:
	-rm -f $(yTwistNode_OBJECTS)

Clean_yTwistNode:
	-rm -f $(yTwistNode_MAKEFILE).bak $(yTwistNode_OBJECTS) $(yTwistNode_PLUGIN)


plugins: $(yTwistNode_PLUGIN)
depend:	 depend_yTwistNode
clean:	 clean_yTwistNode
Clean:	 Clean_yTwistNode

