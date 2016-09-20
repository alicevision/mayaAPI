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

SRCDIR := $(TOP)/moveTool
DSTDIR := $(TOP)/moveTool

moveTool_SOURCES  := $(TOP)/moveTool/moveTool.cpp
moveTool_OBJECTS  := $(TOP)/moveTool/moveTool.o
moveTool_PLUGIN   := $(DSTDIR)/moveTool.$(EXT)
moveTool_MAKEFILE := $(DSTDIR)/Makefile

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

$(moveTool_OBJECTS): CFLAGS   := $(CFLAGS)   $(moveTool_EXTRA_CFLAGS)
$(moveTool_OBJECTS): C++FLAGS := $(C++FLAGS) $(moveTool_EXTRA_C++FLAGS)
$(moveTool_OBJECTS): INCLUDES := $(INCLUDES) $(moveTool_EXTRA_INCLUDES)

depend_moveTool:     INCLUDES := $(INCLUDES) $(moveTool_EXTRA_INCLUDES)

$(moveTool_PLUGIN):  LFLAGS   := $(LFLAGS) $(moveTool_EXTRA_LFLAGS) 
$(moveTool_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(moveTool_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_moveTool clean_moveTool Clean_moveTool


$(moveTool_PLUGIN): $(moveTool_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_moveTool :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(moveTool_SOURCES)

clean_moveTool:
	-rm -f $(moveTool_OBJECTS)

Clean_moveTool:
	-rm -f $(moveTool_MAKEFILE).bak $(moveTool_OBJECTS) $(moveTool_PLUGIN)


plugins: $(moveTool_PLUGIN)
depend:	 depend_moveTool
clean:	 clean_moveTool
Clean:	 Clean_moveTool

