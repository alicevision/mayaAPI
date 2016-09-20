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

SRCDIR := $(TOP)/richMoveCmd
DSTDIR := $(TOP)/richMoveCmd

richMoveCmd_SOURCES  := $(TOP)/richMoveCmd/richMoveCmd.cpp
richMoveCmd_OBJECTS  := $(TOP)/richMoveCmd/richMoveCmd.o
richMoveCmd_PLUGIN   := $(DSTDIR)/richMoveCmd.$(EXT)
richMoveCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(richMoveCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(richMoveCmd_EXTRA_CFLAGS)
$(richMoveCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(richMoveCmd_EXTRA_C++FLAGS)
$(richMoveCmd_OBJECTS): INCLUDES := $(INCLUDES) $(richMoveCmd_EXTRA_INCLUDES)

depend_richMoveCmd:     INCLUDES := $(INCLUDES) $(richMoveCmd_EXTRA_INCLUDES)

$(richMoveCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(richMoveCmd_EXTRA_LFLAGS) 
$(richMoveCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(richMoveCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_richMoveCmd clean_richMoveCmd Clean_richMoveCmd


$(richMoveCmd_PLUGIN): $(richMoveCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_richMoveCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(richMoveCmd_SOURCES)

clean_richMoveCmd:
	-rm -f $(richMoveCmd_OBJECTS)

Clean_richMoveCmd:
	-rm -f $(richMoveCmd_MAKEFILE).bak $(richMoveCmd_OBJECTS) $(richMoveCmd_PLUGIN)


plugins: $(richMoveCmd_PLUGIN)
depend:	 depend_richMoveCmd
clean:	 clean_richMoveCmd
Clean:	 Clean_richMoveCmd

