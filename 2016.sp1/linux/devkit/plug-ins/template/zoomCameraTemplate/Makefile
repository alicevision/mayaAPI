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
TOP := ../..
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

SRCDIR := $(TOP)/template/zoomCameraTemplate
DSTDIR := $(TOP)/template/zoomCameraTemplate

zoomCameraTemplate_SOURCES  := $(TOP)/template/zoomCameraTemplate/zoomCameraTemplate.cpp
zoomCameraTemplate_OBJECTS  := $(TOP)/template/zoomCameraTemplate/zoomCameraTemplate.o
zoomCameraTemplate_PLUGIN   := $(DSTDIR)/zoomCameraTemplate.$(EXT)
zoomCameraTemplate_MAKEFILE := $(DSTDIR)/Makefile

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

$(zoomCameraTemplate_OBJECTS): CFLAGS   := $(CFLAGS)   $(zoomCameraTemplate_EXTRA_CFLAGS)
$(zoomCameraTemplate_OBJECTS): C++FLAGS := $(C++FLAGS) $(zoomCameraTemplate_EXTRA_C++FLAGS)
$(zoomCameraTemplate_OBJECTS): INCLUDES := $(INCLUDES) $(zoomCameraTemplate_EXTRA_INCLUDES)

depend_zoomCameraTemplate:     INCLUDES := $(INCLUDES) $(zoomCameraTemplate_EXTRA_INCLUDES)

$(zoomCameraTemplate_PLUGIN):  LFLAGS   := $(LFLAGS) $(zoomCameraTemplate_EXTRA_LFLAGS) 
$(zoomCameraTemplate_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaUI $(zoomCameraTemplate_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_zoomCameraTemplate clean_zoomCameraTemplate Clean_zoomCameraTemplate


$(zoomCameraTemplate_PLUGIN): $(zoomCameraTemplate_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_zoomCameraTemplate :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(zoomCameraTemplate_SOURCES)

clean_zoomCameraTemplate:
	-rm -f $(zoomCameraTemplate_OBJECTS)

Clean_zoomCameraTemplate:
	-rm -f $(zoomCameraTemplate_MAKEFILE).bak $(zoomCameraTemplate_OBJECTS) $(zoomCameraTemplate_PLUGIN)


plugins: $(zoomCameraTemplate_PLUGIN)
depend:	 depend_zoomCameraTemplate
clean:	 clean_zoomCameraTemplate
Clean:	 Clean_zoomCameraTemplate

