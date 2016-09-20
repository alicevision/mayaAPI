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

SRCDIR := $(TOP)/viewRenderOverrideShadows
DSTDIR := $(TOP)/viewRenderOverrideShadows

viewRenderOverrideShadows_SOURCES  := $(TOP)/viewRenderOverrideShadows/viewRenderOverrideShadows.cpp $(TOP)/viewRenderOverrideShadows/viewRenderOverrideShadowsMain.cpp
viewRenderOverrideShadows_OBJECTS  := $(TOP)/viewRenderOverrideShadows/viewRenderOverrideShadows.o $(TOP)/viewRenderOverrideShadows/viewRenderOverrideShadowsMain.o
viewRenderOverrideShadows_PLUGIN   := $(DSTDIR)/viewRenderOverrideShadows.$(EXT)
viewRenderOverrideShadows_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewRenderOverrideShadows_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewRenderOverrideShadows_EXTRA_CFLAGS)
$(viewRenderOverrideShadows_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewRenderOverrideShadows_EXTRA_C++FLAGS)
$(viewRenderOverrideShadows_OBJECTS): INCLUDES := $(INCLUDES) $(viewRenderOverrideShadows_EXTRA_INCLUDES)

depend_viewRenderOverrideShadows:     INCLUDES := $(INCLUDES) $(viewRenderOverrideShadows_EXTRA_INCLUDES)

$(viewRenderOverrideShadows_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewRenderOverrideShadows_EXTRA_LFLAGS) 
$(viewRenderOverrideShadows_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(viewRenderOverrideShadows_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewRenderOverrideShadows clean_viewRenderOverrideShadows Clean_viewRenderOverrideShadows


$(viewRenderOverrideShadows_PLUGIN): $(viewRenderOverrideShadows_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewRenderOverrideShadows :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewRenderOverrideShadows_SOURCES)

clean_viewRenderOverrideShadows:
	-rm -f $(viewRenderOverrideShadows_OBJECTS)

Clean_viewRenderOverrideShadows:
	-rm -f $(viewRenderOverrideShadows_MAKEFILE).bak $(viewRenderOverrideShadows_OBJECTS) $(viewRenderOverrideShadows_PLUGIN)


plugins: $(viewRenderOverrideShadows_PLUGIN)
depend:	 depend_viewRenderOverrideShadows
clean:	 clean_viewRenderOverrideShadows
Clean:	 Clean_viewRenderOverrideShadows

