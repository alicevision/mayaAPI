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

SRCDIR := $(TOP)/helixTool
DSTDIR := $(TOP)/helixTool

helixTool_SOURCES  := $(TOP)/helixTool/helixTool.cpp
helixTool_OBJECTS  := $(TOP)/helixTool/helixTool.o
helixTool_PLUGIN   := $(DSTDIR)/helixTool.$(EXT)
helixTool_MAKEFILE := $(DSTDIR)/Makefile

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

$(helixTool_OBJECTS): CFLAGS   := $(CFLAGS)   $(helixTool_EXTRA_CFLAGS)
$(helixTool_OBJECTS): C++FLAGS := $(C++FLAGS) $(helixTool_EXTRA_C++FLAGS)
$(helixTool_OBJECTS): INCLUDES := $(INCLUDES) $(helixTool_EXTRA_INCLUDES)

depend_helixTool:     INCLUDES := $(INCLUDES) $(helixTool_EXTRA_INCLUDES)

$(helixTool_PLUGIN):  LFLAGS   := $(LFLAGS) $(helixTool_EXTRA_LFLAGS) 
$(helixTool_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL -lGLU $(helixTool_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_helixTool clean_helixTool Clean_helixTool


$(helixTool_PLUGIN): $(helixTool_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_helixTool :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(helixTool_SOURCES)

clean_helixTool:
	-rm -f $(helixTool_OBJECTS)

Clean_helixTool:
	-rm -f $(helixTool_MAKEFILE).bak $(helixTool_OBJECTS) $(helixTool_PLUGIN)


plugins: $(helixTool_PLUGIN)
depend:	 depend_helixTool
clean:	 clean_helixTool
Clean:	 Clean_helixTool

