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

SRCDIR := $(TOP)/viewMRenderOverride
DSTDIR := $(TOP)/viewMRenderOverride

viewMRenderOverride_SOURCES  := $(TOP)/viewMRenderOverride/viewMRenderOverride.cpp
viewMRenderOverride_OBJECTS  := $(TOP)/viewMRenderOverride/viewMRenderOverride.o
viewMRenderOverride_PLUGIN   := $(DSTDIR)/viewMRenderOverride.$(EXT)
viewMRenderOverride_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewMRenderOverride_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewMRenderOverride_EXTRA_CFLAGS)
$(viewMRenderOverride_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewMRenderOverride_EXTRA_C++FLAGS)
$(viewMRenderOverride_OBJECTS): INCLUDES := $(INCLUDES) $(viewMRenderOverride_EXTRA_INCLUDES)

depend_viewMRenderOverride:     INCLUDES := $(INCLUDES) $(viewMRenderOverride_EXTRA_INCLUDES)

$(viewMRenderOverride_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewMRenderOverride_EXTRA_LFLAGS) 
$(viewMRenderOverride_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(viewMRenderOverride_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewMRenderOverride clean_viewMRenderOverride Clean_viewMRenderOverride


$(viewMRenderOverride_PLUGIN): $(viewMRenderOverride_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewMRenderOverride :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewMRenderOverride_SOURCES)

clean_viewMRenderOverride:
	-rm -f $(viewMRenderOverride_OBJECTS)

Clean_viewMRenderOverride:
	-rm -f $(viewMRenderOverride_MAKEFILE).bak $(viewMRenderOverride_OBJECTS) $(viewMRenderOverride_PLUGIN)


plugins: $(viewMRenderOverride_PLUGIN)
depend:	 depend_viewMRenderOverride
clean:	 clean_viewMRenderOverride
Clean:	 Clean_viewMRenderOverride

