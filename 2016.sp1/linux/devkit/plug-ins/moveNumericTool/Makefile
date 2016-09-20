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

SRCDIR := $(TOP)/moveNumericTool
DSTDIR := $(TOP)/moveNumericTool

moveNumericTool_SOURCES  := $(TOP)/moveNumericTool/moveNumericTool.cpp
moveNumericTool_OBJECTS  := $(TOP)/moveNumericTool/moveNumericTool.o
moveNumericTool_PLUGIN   := $(DSTDIR)/moveNumericTool.$(EXT)
moveNumericTool_MAKEFILE := $(DSTDIR)/Makefile

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

$(moveNumericTool_OBJECTS): CFLAGS   := $(CFLAGS)   $(moveNumericTool_EXTRA_CFLAGS)
$(moveNumericTool_OBJECTS): C++FLAGS := $(C++FLAGS) $(moveNumericTool_EXTRA_C++FLAGS)
$(moveNumericTool_OBJECTS): INCLUDES := $(INCLUDES) $(moveNumericTool_EXTRA_INCLUDES)

depend_moveNumericTool:     INCLUDES := $(INCLUDES) $(moveNumericTool_EXTRA_INCLUDES)

$(moveNumericTool_PLUGIN):  LFLAGS   := $(LFLAGS) $(moveNumericTool_EXTRA_LFLAGS) 
$(moveNumericTool_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(moveNumericTool_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_moveNumericTool clean_moveNumericTool Clean_moveNumericTool


$(moveNumericTool_PLUGIN): $(moveNumericTool_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_moveNumericTool :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(moveNumericTool_SOURCES)

clean_moveNumericTool:
	-rm -f $(moveNumericTool_OBJECTS)

Clean_moveNumericTool:
	-rm -f $(moveNumericTool_MAKEFILE).bak $(moveNumericTool_OBJECTS) $(moveNumericTool_PLUGIN)


plugins: $(moveNumericTool_PLUGIN)
depend:	 depend_moveNumericTool
clean:	 clean_moveNumericTool
Clean:	 Clean_moveNumericTool

