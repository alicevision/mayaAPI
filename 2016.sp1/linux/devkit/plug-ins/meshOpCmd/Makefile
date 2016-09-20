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

SRCDIR := $(TOP)/meshOpCmd
DSTDIR := $(TOP)/meshOpCmd

meshOpCmd_SOURCES  := $(TOP)/meshOpCmd/meshOpCmd.cpp $(TOP)/meshOpCmd/meshOpFty.cpp $(TOP)/meshOpCmd/meshOpFtyAction.cpp $(TOP)/meshOpCmd/meshOpMain.cpp $(TOP)/meshOpCmd/meshOpNode.cpp $(TOP)/meshOpCmd/polyModifierNode.cpp $(TOP)/meshOpCmd/polyModifierCmd.cpp $(TOP)/meshOpCmd/polyModifierFty.cpp
meshOpCmd_OBJECTS  := $(TOP)/meshOpCmd/meshOpCmd.o $(TOP)/meshOpCmd/meshOpFty.o $(TOP)/meshOpCmd/meshOpFtyAction.o $(TOP)/meshOpCmd/meshOpMain.o $(TOP)/meshOpCmd/meshOpNode.o $(TOP)/meshOpCmd/polyModifierNode.o $(TOP)/meshOpCmd/polyModifierCmd.o $(TOP)/meshOpCmd/polyModifierFty.o
meshOpCmd_PLUGIN   := $(DSTDIR)/meshOpCmd.$(EXT)
meshOpCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(meshOpCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(meshOpCmd_EXTRA_CFLAGS)
$(meshOpCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(meshOpCmd_EXTRA_C++FLAGS)
$(meshOpCmd_OBJECTS): INCLUDES := $(INCLUDES) $(meshOpCmd_EXTRA_INCLUDES)

depend_meshOpCmd:     INCLUDES := $(INCLUDES) $(meshOpCmd_EXTRA_INCLUDES)

$(meshOpCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(meshOpCmd_EXTRA_LFLAGS) 
$(meshOpCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(meshOpCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_meshOpCmd clean_meshOpCmd Clean_meshOpCmd


$(meshOpCmd_PLUGIN): $(meshOpCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_meshOpCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(meshOpCmd_SOURCES)

clean_meshOpCmd:
	-rm -f $(meshOpCmd_OBJECTS)

Clean_meshOpCmd:
	-rm -f $(meshOpCmd_MAKEFILE).bak $(meshOpCmd_OBJECTS) $(meshOpCmd_PLUGIN)


plugins: $(meshOpCmd_PLUGIN)
depend:	 depend_meshOpCmd
clean:	 clean_meshOpCmd
Clean:	 Clean_meshOpCmd

