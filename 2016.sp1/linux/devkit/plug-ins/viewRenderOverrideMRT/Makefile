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

SRCDIR := $(TOP)/viewRenderOverrideMRT
DSTDIR := $(TOP)/viewRenderOverrideMRT

viewRenderOverrideMRT_SOURCES  := $(TOP)/viewRenderOverrideMRT/viewRenderOverrideMRT.cpp $(TOP)/viewRenderOverrideMRT/viewRenderOverrideMRTMain.cpp
viewRenderOverrideMRT_OBJECTS  := $(TOP)/viewRenderOverrideMRT/viewRenderOverrideMRT.o $(TOP)/viewRenderOverrideMRT/viewRenderOverrideMRTMain.o
viewRenderOverrideMRT_PLUGIN   := $(DSTDIR)/viewRenderOverrideMRT.$(EXT)
viewRenderOverrideMRT_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewRenderOverrideMRT_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewRenderOverrideMRT_EXTRA_CFLAGS)
$(viewRenderOverrideMRT_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewRenderOverrideMRT_EXTRA_C++FLAGS)
$(viewRenderOverrideMRT_OBJECTS): INCLUDES := $(INCLUDES) $(viewRenderOverrideMRT_EXTRA_INCLUDES)

depend_viewRenderOverrideMRT:     INCLUDES := $(INCLUDES) $(viewRenderOverrideMRT_EXTRA_INCLUDES)

$(viewRenderOverrideMRT_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewRenderOverrideMRT_EXTRA_LFLAGS) 
$(viewRenderOverrideMRT_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(viewRenderOverrideMRT_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewRenderOverrideMRT clean_viewRenderOverrideMRT Clean_viewRenderOverrideMRT


$(viewRenderOverrideMRT_PLUGIN): $(viewRenderOverrideMRT_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewRenderOverrideMRT :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewRenderOverrideMRT_SOURCES)

clean_viewRenderOverrideMRT:
	-rm -f $(viewRenderOverrideMRT_OBJECTS)

Clean_viewRenderOverrideMRT:
	-rm -f $(viewRenderOverrideMRT_MAKEFILE).bak $(viewRenderOverrideMRT_OBJECTS) $(viewRenderOverrideMRT_PLUGIN)


plugins: $(viewRenderOverrideMRT_PLUGIN)
depend:	 depend_viewRenderOverrideMRT
clean:	 clean_viewRenderOverrideMRT
Clean:	 Clean_viewRenderOverrideMRT

