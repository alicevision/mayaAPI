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

SRCDIR := $(TOP)/zoomCameraCmd
DSTDIR := $(TOP)/zoomCameraCmd

zoomCameraCmd_SOURCES  := $(TOP)/zoomCameraCmd/zoomCameraCmd.cpp
zoomCameraCmd_OBJECTS  := $(TOP)/zoomCameraCmd/zoomCameraCmd.o
zoomCameraCmd_PLUGIN   := $(DSTDIR)/zoomCameraCmd.$(EXT)
zoomCameraCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(zoomCameraCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(zoomCameraCmd_EXTRA_CFLAGS)
$(zoomCameraCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(zoomCameraCmd_EXTRA_C++FLAGS)
$(zoomCameraCmd_OBJECTS): INCLUDES := $(INCLUDES) $(zoomCameraCmd_EXTRA_INCLUDES)

depend_zoomCameraCmd:     INCLUDES := $(INCLUDES) $(zoomCameraCmd_EXTRA_INCLUDES)

$(zoomCameraCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(zoomCameraCmd_EXTRA_LFLAGS) 
$(zoomCameraCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(zoomCameraCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_zoomCameraCmd clean_zoomCameraCmd Clean_zoomCameraCmd


$(zoomCameraCmd_PLUGIN): $(zoomCameraCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_zoomCameraCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(zoomCameraCmd_SOURCES)

clean_zoomCameraCmd:
	-rm -f $(zoomCameraCmd_OBJECTS)

Clean_zoomCameraCmd:
	-rm -f $(zoomCameraCmd_MAKEFILE).bak $(zoomCameraCmd_OBJECTS) $(zoomCameraCmd_PLUGIN)


plugins: $(zoomCameraCmd_PLUGIN)
depend:	 depend_zoomCameraCmd
clean:	 clean_zoomCameraCmd
Clean:	 Clean_zoomCameraCmd

