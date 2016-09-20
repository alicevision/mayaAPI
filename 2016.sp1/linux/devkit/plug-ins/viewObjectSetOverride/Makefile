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

SRCDIR := $(TOP)/viewObjectSetOverride
DSTDIR := $(TOP)/viewObjectSetOverride

viewObjectSetOverride_SOURCES  := $(TOP)/viewObjectSetOverride/viewObjectSetOverride.cpp
viewObjectSetOverride_OBJECTS  := $(TOP)/viewObjectSetOverride/viewObjectSetOverride.o
viewObjectSetOverride_PLUGIN   := $(DSTDIR)/viewObjectSetOverride.$(EXT)
viewObjectSetOverride_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewObjectSetOverride_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewObjectSetOverride_EXTRA_CFLAGS)
$(viewObjectSetOverride_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewObjectSetOverride_EXTRA_C++FLAGS)
$(viewObjectSetOverride_OBJECTS): INCLUDES := $(INCLUDES) $(viewObjectSetOverride_EXTRA_INCLUDES)

depend_viewObjectSetOverride:     INCLUDES := $(INCLUDES) $(viewObjectSetOverride_EXTRA_INCLUDES)

$(viewObjectSetOverride_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewObjectSetOverride_EXTRA_LFLAGS) 
$(viewObjectSetOverride_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(viewObjectSetOverride_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewObjectSetOverride clean_viewObjectSetOverride Clean_viewObjectSetOverride


$(viewObjectSetOverride_PLUGIN): $(viewObjectSetOverride_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewObjectSetOverride :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewObjectSetOverride_SOURCES)

clean_viewObjectSetOverride:
	-rm -f $(viewObjectSetOverride_OBJECTS)

Clean_viewObjectSetOverride:
	-rm -f $(viewObjectSetOverride_MAKEFILE).bak $(viewObjectSetOverride_OBJECTS) $(viewObjectSetOverride_PLUGIN)


plugins: $(viewObjectSetOverride_PLUGIN)
depend:	 depend_viewObjectSetOverride
clean:	 clean_viewObjectSetOverride
Clean:	 Clean_viewObjectSetOverride

