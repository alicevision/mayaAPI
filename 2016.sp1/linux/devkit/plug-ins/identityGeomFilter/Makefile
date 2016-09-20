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

SRCDIR := $(TOP)/identityGeomFilter
DSTDIR := $(TOP)/identityGeomFilter

identityGeomFilter_SOURCES  := $(TOP)/identityGeomFilter/identityGeomFilter.cpp
identityGeomFilter_OBJECTS  := $(TOP)/identityGeomFilter/identityGeomFilter.o
identityGeomFilter_PLUGIN   := $(DSTDIR)/identityGeomFilter.$(EXT)
identityGeomFilter_MAKEFILE := $(DSTDIR)/Makefile

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

$(identityGeomFilter_OBJECTS): CFLAGS   := $(CFLAGS)   $(identityGeomFilter_EXTRA_CFLAGS)
$(identityGeomFilter_OBJECTS): C++FLAGS := $(C++FLAGS) $(identityGeomFilter_EXTRA_C++FLAGS)
$(identityGeomFilter_OBJECTS): INCLUDES := $(INCLUDES) $(identityGeomFilter_EXTRA_INCLUDES)

depend_identityGeomFilter:     INCLUDES := $(INCLUDES) $(identityGeomFilter_EXTRA_INCLUDES)

$(identityGeomFilter_PLUGIN):  LFLAGS   := $(LFLAGS) $(identityGeomFilter_EXTRA_LFLAGS) 
$(identityGeomFilter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(identityGeomFilter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_identityGeomFilter clean_identityGeomFilter Clean_identityGeomFilter


$(identityGeomFilter_PLUGIN): $(identityGeomFilter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_identityGeomFilter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(identityGeomFilter_SOURCES)

clean_identityGeomFilter:
	-rm -f $(identityGeomFilter_OBJECTS)

Clean_identityGeomFilter:
	-rm -f $(identityGeomFilter_MAKEFILE).bak $(identityGeomFilter_OBJECTS) $(identityGeomFilter_PLUGIN)


plugins: $(identityGeomFilter_PLUGIN)
depend:	 depend_identityGeomFilter
clean:	 clean_identityGeomFilter
Clean:	 Clean_identityGeomFilter

