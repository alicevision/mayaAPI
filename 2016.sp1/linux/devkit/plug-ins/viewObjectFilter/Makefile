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

SRCDIR := $(TOP)/viewObjectFilter
DSTDIR := $(TOP)/viewObjectFilter

viewObjectFilter_SOURCES  := $(TOP)/viewObjectFilter/viewObjectFilter.cpp
viewObjectFilter_OBJECTS  := $(TOP)/viewObjectFilter/viewObjectFilter.o
viewObjectFilter_PLUGIN   := $(DSTDIR)/viewObjectFilter.$(EXT)
viewObjectFilter_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewObjectFilter_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewObjectFilter_EXTRA_CFLAGS)
$(viewObjectFilter_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewObjectFilter_EXTRA_C++FLAGS)
$(viewObjectFilter_OBJECTS): INCLUDES := $(INCLUDES) $(viewObjectFilter_EXTRA_INCLUDES)

depend_viewObjectFilter:     INCLUDES := $(INCLUDES) $(viewObjectFilter_EXTRA_INCLUDES)

$(viewObjectFilter_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewObjectFilter_EXTRA_LFLAGS) 
$(viewObjectFilter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(viewObjectFilter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewObjectFilter clean_viewObjectFilter Clean_viewObjectFilter


$(viewObjectFilter_PLUGIN): $(viewObjectFilter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewObjectFilter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewObjectFilter_SOURCES)

clean_viewObjectFilter:
	-rm -f $(viewObjectFilter_OBJECTS)

Clean_viewObjectFilter:
	-rm -f $(viewObjectFilter_MAKEFILE).bak $(viewObjectFilter_OBJECTS) $(viewObjectFilter_PLUGIN)


plugins: $(viewObjectFilter_PLUGIN)
depend:	 depend_viewObjectFilter
clean:	 clean_viewObjectFilter
Clean:	 Clean_viewObjectFilter

