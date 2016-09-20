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

SRCDIR := $(TOP)/viewCaptureCmd
DSTDIR := $(TOP)/viewCaptureCmd

viewCaptureCmd_SOURCES  := $(TOP)/viewCaptureCmd/viewCapturePPM.c $(TOP)/viewCaptureCmd/viewCaptureCmd.cpp
viewCaptureCmd_OBJECTS  := $(TOP)/viewCaptureCmd/viewCapturePPM.o $(TOP)/viewCaptureCmd/viewCaptureCmd.o
viewCaptureCmd_PLUGIN   := $(DSTDIR)/viewCaptureCmd.$(EXT)
viewCaptureCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewCaptureCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewCaptureCmd_EXTRA_CFLAGS)
$(viewCaptureCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewCaptureCmd_EXTRA_C++FLAGS)
$(viewCaptureCmd_OBJECTS): INCLUDES := $(INCLUDES) $(viewCaptureCmd_EXTRA_INCLUDES)

depend_viewCaptureCmd:     INCLUDES := $(INCLUDES) $(viewCaptureCmd_EXTRA_INCLUDES)

$(viewCaptureCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewCaptureCmd_EXTRA_LFLAGS) 
$(viewCaptureCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL $(viewCaptureCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewCaptureCmd clean_viewCaptureCmd Clean_viewCaptureCmd


$(viewCaptureCmd_PLUGIN): $(viewCaptureCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewCaptureCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewCaptureCmd_SOURCES)

clean_viewCaptureCmd:
	-rm -f $(viewCaptureCmd_OBJECTS)

Clean_viewCaptureCmd:
	-rm -f $(viewCaptureCmd_MAKEFILE).bak $(viewCaptureCmd_OBJECTS) $(viewCaptureCmd_PLUGIN)


plugins: $(viewCaptureCmd_PLUGIN)
depend:	 depend_viewCaptureCmd
clean:	 clean_viewCaptureCmd
Clean:	 Clean_viewCaptureCmd

