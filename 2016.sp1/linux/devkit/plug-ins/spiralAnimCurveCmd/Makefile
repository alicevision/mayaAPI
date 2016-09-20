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

SRCDIR := $(TOP)/spiralAnimCurveCmd
DSTDIR := $(TOP)/spiralAnimCurveCmd

spiralAnimCurveCmd_SOURCES  := $(TOP)/spiralAnimCurveCmd/spiralAnimCurveCmd.cpp
spiralAnimCurveCmd_OBJECTS  := $(TOP)/spiralAnimCurveCmd/spiralAnimCurveCmd.o
spiralAnimCurveCmd_PLUGIN   := $(DSTDIR)/spiralAnimCurveCmd.$(EXT)
spiralAnimCurveCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(spiralAnimCurveCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(spiralAnimCurveCmd_EXTRA_CFLAGS)
$(spiralAnimCurveCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(spiralAnimCurveCmd_EXTRA_C++FLAGS)
$(spiralAnimCurveCmd_OBJECTS): INCLUDES := $(INCLUDES) $(spiralAnimCurveCmd_EXTRA_INCLUDES)

depend_spiralAnimCurveCmd:     INCLUDES := $(INCLUDES) $(spiralAnimCurveCmd_EXTRA_INCLUDES)

$(spiralAnimCurveCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(spiralAnimCurveCmd_EXTRA_LFLAGS) 
$(spiralAnimCurveCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(spiralAnimCurveCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_spiralAnimCurveCmd clean_spiralAnimCurveCmd Clean_spiralAnimCurveCmd


$(spiralAnimCurveCmd_PLUGIN): $(spiralAnimCurveCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_spiralAnimCurveCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(spiralAnimCurveCmd_SOURCES)

clean_spiralAnimCurveCmd:
	-rm -f $(spiralAnimCurveCmd_OBJECTS)

Clean_spiralAnimCurveCmd:
	-rm -f $(spiralAnimCurveCmd_MAKEFILE).bak $(spiralAnimCurveCmd_OBJECTS) $(spiralAnimCurveCmd_PLUGIN)


plugins: $(spiralAnimCurveCmd_PLUGIN)
depend:	 depend_spiralAnimCurveCmd
clean:	 clean_spiralAnimCurveCmd
Clean:	 Clean_spiralAnimCurveCmd

