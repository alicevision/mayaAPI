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

SRCDIR := $(TOP)/OpenGLViewportRenderer
DSTDIR := $(TOP)/OpenGLViewportRenderer

OpenGLViewportRenderer_SOURCES  := $(TOP)/OpenGLViewportRenderer/OpenGLViewportRenderer.cpp $(TOP)/OpenGLViewportRenderer/OpenGLViewportRendererPlugin.cpp
OpenGLViewportRenderer_OBJECTS  := $(TOP)/OpenGLViewportRenderer/OpenGLViewportRenderer.o $(TOP)/OpenGLViewportRenderer/OpenGLViewportRendererPlugin.o
OpenGLViewportRenderer_PLUGIN   := $(DSTDIR)/OpenGLViewportRenderer.$(EXT)
OpenGLViewportRenderer_MAKEFILE := $(DSTDIR)/Makefile

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

$(OpenGLViewportRenderer_OBJECTS): CFLAGS   := $(CFLAGS)   $(OpenGLViewportRenderer_EXTRA_CFLAGS)
$(OpenGLViewportRenderer_OBJECTS): C++FLAGS := $(C++FLAGS) $(OpenGLViewportRenderer_EXTRA_C++FLAGS)
$(OpenGLViewportRenderer_OBJECTS): INCLUDES := $(INCLUDES) $(OpenGLViewportRenderer_EXTRA_INCLUDES)

depend_OpenGLViewportRenderer:     INCLUDES := $(INCLUDES) $(OpenGLViewportRenderer_EXTRA_INCLUDES)

$(OpenGLViewportRenderer_PLUGIN):  LFLAGS   := $(LFLAGS) $(OpenGLViewportRenderer_EXTRA_LFLAGS) 
$(OpenGLViewportRenderer_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMaya -lOpenMayaUI -lFoundation -lGL $(OpenGLViewportRenderer_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_OpenGLViewportRenderer clean_OpenGLViewportRenderer Clean_OpenGLViewportRenderer


$(OpenGLViewportRenderer_PLUGIN): $(OpenGLViewportRenderer_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_OpenGLViewportRenderer :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(OpenGLViewportRenderer_SOURCES)

clean_OpenGLViewportRenderer:
	-rm -f $(OpenGLViewportRenderer_OBJECTS)

Clean_OpenGLViewportRenderer:
	-rm -f $(OpenGLViewportRenderer_MAKEFILE).bak $(OpenGLViewportRenderer_OBJECTS) $(OpenGLViewportRenderer_PLUGIN)


plugins: $(OpenGLViewportRenderer_PLUGIN)
depend:	 depend_OpenGLViewportRenderer
clean:	 clean_OpenGLViewportRenderer
Clean:	 Clean_OpenGLViewportRenderer

