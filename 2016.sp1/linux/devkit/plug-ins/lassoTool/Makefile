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

SRCDIR := $(TOP)/lassoTool
DSTDIR := $(TOP)/lassoTool

lassoTool_SOURCES  := $(TOP)/lassoTool/lassoTool.cpp
lassoTool_OBJECTS  := $(TOP)/lassoTool/lassoTool.o
lassoTool_PLUGIN   := $(DSTDIR)/lassoTool.$(EXT)
lassoTool_MAKEFILE := $(DSTDIR)/Makefile

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

$(lassoTool_OBJECTS): CFLAGS   := $(CFLAGS)   $(lassoTool_EXTRA_CFLAGS)
$(lassoTool_OBJECTS): C++FLAGS := $(C++FLAGS) $(lassoTool_EXTRA_C++FLAGS)
$(lassoTool_OBJECTS): INCLUDES := $(INCLUDES) $(lassoTool_EXTRA_INCLUDES)

depend_lassoTool:     INCLUDES := $(INCLUDES) $(lassoTool_EXTRA_INCLUDES)

$(lassoTool_PLUGIN):  LFLAGS   := $(LFLAGS) $(lassoTool_EXTRA_LFLAGS) 
$(lassoTool_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL -lGLU $(lassoTool_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lassoTool clean_lassoTool Clean_lassoTool


$(lassoTool_PLUGIN): $(lassoTool_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lassoTool :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lassoTool_SOURCES)

clean_lassoTool:
	-rm -f $(lassoTool_OBJECTS)

Clean_lassoTool:
	-rm -f $(lassoTool_MAKEFILE).bak $(lassoTool_OBJECTS) $(lassoTool_PLUGIN)


plugins: $(lassoTool_PLUGIN)
depend:	 depend_lassoTool
clean:	 clean_lassoTool
Clean:	 Clean_lassoTool

