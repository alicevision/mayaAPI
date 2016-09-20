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

SRCDIR := $(TOP)/viewRenderOverrideTargets
DSTDIR := $(TOP)/viewRenderOverrideTargets

viewRenderOverrideTargets_SOURCES  := $(TOP)/viewRenderOverrideTargets/viewRenderOverrideTargets.cpp $(TOP)/viewRenderOverrideTargets/viewRenderOverrideTargetsMain.cpp
viewRenderOverrideTargets_OBJECTS  := $(TOP)/viewRenderOverrideTargets/viewRenderOverrideTargets.o $(TOP)/viewRenderOverrideTargets/viewRenderOverrideTargetsMain.o
viewRenderOverrideTargets_PLUGIN   := $(DSTDIR)/viewRenderOverrideTargets.$(EXT)
viewRenderOverrideTargets_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewRenderOverrideTargets_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewRenderOverrideTargets_EXTRA_CFLAGS)
$(viewRenderOverrideTargets_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewRenderOverrideTargets_EXTRA_C++FLAGS)
$(viewRenderOverrideTargets_OBJECTS): INCLUDES := $(INCLUDES) $(viewRenderOverrideTargets_EXTRA_INCLUDES)

depend_viewRenderOverrideTargets:     INCLUDES := $(INCLUDES) $(viewRenderOverrideTargets_EXTRA_INCLUDES)

$(viewRenderOverrideTargets_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewRenderOverrideTargets_EXTRA_LFLAGS) 
$(viewRenderOverrideTargets_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(viewRenderOverrideTargets_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewRenderOverrideTargets clean_viewRenderOverrideTargets Clean_viewRenderOverrideTargets


$(viewRenderOverrideTargets_PLUGIN): $(viewRenderOverrideTargets_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewRenderOverrideTargets :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewRenderOverrideTargets_SOURCES)

clean_viewRenderOverrideTargets:
	-rm -f $(viewRenderOverrideTargets_OBJECTS)

Clean_viewRenderOverrideTargets:
	-rm -f $(viewRenderOverrideTargets_MAKEFILE).bak $(viewRenderOverrideTargets_OBJECTS) $(viewRenderOverrideTargets_PLUGIN)


plugins: $(viewRenderOverrideTargets_PLUGIN)
depend:	 depend_viewRenderOverrideTargets
clean:	 clean_viewRenderOverrideTargets
Clean:	 Clean_viewRenderOverrideTargets

