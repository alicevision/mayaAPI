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

SRCDIR := $(TOP)/viewRenderOverride
DSTDIR := $(TOP)/viewRenderOverride

viewRenderOverride_SOURCES  := $(TOP)/viewRenderOverride/viewRenderOverride.cpp $(TOP)/viewRenderOverride/viewRenderOverrideMain.cpp $(TOP)/viewRenderOverride/viewRenderOverrideOperations.cpp $(TOP)/viewRenderOverride/viewRenderOverrideUserOperation.cpp
viewRenderOverride_OBJECTS  := $(TOP)/viewRenderOverride/viewRenderOverride.o $(TOP)/viewRenderOverride/viewRenderOverrideMain.o $(TOP)/viewRenderOverride/viewRenderOverrideOperations.o $(TOP)/viewRenderOverride/viewRenderOverrideUserOperation.o
viewRenderOverride_PLUGIN   := $(DSTDIR)/viewRenderOverride.$(EXT)
viewRenderOverride_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewRenderOverride_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewRenderOverride_EXTRA_CFLAGS)
$(viewRenderOverride_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewRenderOverride_EXTRA_C++FLAGS)
$(viewRenderOverride_OBJECTS): INCLUDES := $(INCLUDES) $(viewRenderOverride_EXTRA_INCLUDES)

depend_viewRenderOverride:     INCLUDES := $(INCLUDES) $(viewRenderOverride_EXTRA_INCLUDES)

$(viewRenderOverride_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewRenderOverride_EXTRA_LFLAGS) 
$(viewRenderOverride_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(viewRenderOverride_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewRenderOverride clean_viewRenderOverride Clean_viewRenderOverride


$(viewRenderOverride_PLUGIN): $(viewRenderOverride_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewRenderOverride :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewRenderOverride_SOURCES)

clean_viewRenderOverride:
	-rm -f $(viewRenderOverride_OBJECTS)

Clean_viewRenderOverride:
	-rm -f $(viewRenderOverride_MAKEFILE).bak $(viewRenderOverride_OBJECTS) $(viewRenderOverride_PLUGIN)


plugins: $(viewRenderOverride_PLUGIN)
depend:	 depend_viewRenderOverride
clean:	 clean_viewRenderOverride
Clean:	 Clean_viewRenderOverride

