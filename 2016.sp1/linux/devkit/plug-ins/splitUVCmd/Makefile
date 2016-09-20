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

SRCDIR := $(TOP)/splitUVCmd
DSTDIR := $(TOP)/splitUVCmd

splitUVCmd_SOURCES  := $(TOP)/splitUVCmd/splitUVCmd.cpp $(TOP)/splitUVCmd/splitUVFty.cpp $(TOP)/splitUVCmd/splitUVFtyAction.cpp $(TOP)/splitUVCmd/splitUVMain.cpp $(TOP)/splitUVCmd/splitUVNode.cpp $(TOP)/splitUVCmd/polyModifierCmd.cpp $(TOP)/splitUVCmd/polyModifierFty.cpp $(TOP)/splitUVCmd/polyModifierNode.cpp
splitUVCmd_OBJECTS  := $(TOP)/splitUVCmd/splitUVCmd.o $(TOP)/splitUVCmd/splitUVFty.o $(TOP)/splitUVCmd/splitUVFtyAction.o $(TOP)/splitUVCmd/splitUVMain.o $(TOP)/splitUVCmd/splitUVNode.o $(TOP)/splitUVCmd/polyModifierCmd.o $(TOP)/splitUVCmd/polyModifierFty.o $(TOP)/splitUVCmd/polyModifierNode.o
splitUVCmd_PLUGIN   := $(DSTDIR)/splitUVCmd.$(EXT)
splitUVCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(splitUVCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(splitUVCmd_EXTRA_CFLAGS)
$(splitUVCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(splitUVCmd_EXTRA_C++FLAGS)
$(splitUVCmd_OBJECTS): INCLUDES := $(INCLUDES) $(splitUVCmd_EXTRA_INCLUDES)

depend_splitUVCmd:     INCLUDES := $(INCLUDES) $(splitUVCmd_EXTRA_INCLUDES)

$(splitUVCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(splitUVCmd_EXTRA_LFLAGS) 
$(splitUVCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL $(splitUVCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_splitUVCmd clean_splitUVCmd Clean_splitUVCmd


$(splitUVCmd_PLUGIN): $(splitUVCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_splitUVCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(splitUVCmd_SOURCES)

clean_splitUVCmd:
	-rm -f $(splitUVCmd_OBJECTS)

Clean_splitUVCmd:
	-rm -f $(splitUVCmd_MAKEFILE).bak $(splitUVCmd_OBJECTS) $(splitUVCmd_PLUGIN)


plugins: $(splitUVCmd_PLUGIN)
depend:	 depend_splitUVCmd
clean:	 clean_splitUVCmd
Clean:	 Clean_splitUVCmd

