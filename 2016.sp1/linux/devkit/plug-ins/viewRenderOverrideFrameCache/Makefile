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

SRCDIR := $(TOP)/viewRenderOverrideFrameCache
DSTDIR := $(TOP)/viewRenderOverrideFrameCache

viewRenderOverrideFrameCache_SOURCES  := $(TOP)/viewRenderOverrideFrameCache/viewRenderOverrideFrameCache.cpp $(TOP)/viewRenderOverrideFrameCache/viewRenderOverrideFrameCacheMain.cpp
viewRenderOverrideFrameCache_OBJECTS  := $(TOP)/viewRenderOverrideFrameCache/viewRenderOverrideFrameCache.o $(TOP)/viewRenderOverrideFrameCache/viewRenderOverrideFrameCacheMain.o
viewRenderOverrideFrameCache_PLUGIN   := $(DSTDIR)/viewRenderOverrideFrameCache.$(EXT)
viewRenderOverrideFrameCache_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewRenderOverrideFrameCache_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewRenderOverrideFrameCache_EXTRA_CFLAGS)
$(viewRenderOverrideFrameCache_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewRenderOverrideFrameCache_EXTRA_C++FLAGS)
$(viewRenderOverrideFrameCache_OBJECTS): INCLUDES := $(INCLUDES) $(viewRenderOverrideFrameCache_EXTRA_INCLUDES)

depend_viewRenderOverrideFrameCache:     INCLUDES := $(INCLUDES) $(viewRenderOverrideFrameCache_EXTRA_INCLUDES)

$(viewRenderOverrideFrameCache_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewRenderOverrideFrameCache_EXTRA_LFLAGS) 
$(viewRenderOverrideFrameCache_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lOpenMayaAnim -lFoundation $(viewRenderOverrideFrameCache_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewRenderOverrideFrameCache clean_viewRenderOverrideFrameCache Clean_viewRenderOverrideFrameCache


$(viewRenderOverrideFrameCache_PLUGIN): $(viewRenderOverrideFrameCache_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewRenderOverrideFrameCache :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewRenderOverrideFrameCache_SOURCES)

clean_viewRenderOverrideFrameCache:
	-rm -f $(viewRenderOverrideFrameCache_OBJECTS)

Clean_viewRenderOverrideFrameCache:
	-rm -f $(viewRenderOverrideFrameCache_MAKEFILE).bak $(viewRenderOverrideFrameCache_OBJECTS) $(viewRenderOverrideFrameCache_PLUGIN)


plugins: $(viewRenderOverrideFrameCache_PLUGIN)
depend:	 depend_viewRenderOverrideFrameCache
clean:	 clean_viewRenderOverrideFrameCache
Clean:	 Clean_viewRenderOverrideFrameCache

