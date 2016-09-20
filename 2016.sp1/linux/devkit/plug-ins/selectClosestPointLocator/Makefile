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

SRCDIR := $(TOP)/selectClosestPointLocator
DSTDIR := $(TOP)/selectClosestPointLocator

selectClosestPointLocator_SOURCES  := $(TOP)/selectClosestPointLocator/selectClosestPointLocator.cpp
selectClosestPointLocator_OBJECTS  := $(TOP)/selectClosestPointLocator/selectClosestPointLocator.o
selectClosestPointLocator_PLUGIN   := $(DSTDIR)/selectClosestPointLocator.$(EXT)
selectClosestPointLocator_MAKEFILE := $(DSTDIR)/Makefile

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

$(selectClosestPointLocator_OBJECTS): CFLAGS   := $(CFLAGS)   $(selectClosestPointLocator_EXTRA_CFLAGS)
$(selectClosestPointLocator_OBJECTS): C++FLAGS := $(C++FLAGS) $(selectClosestPointLocator_EXTRA_C++FLAGS)
$(selectClosestPointLocator_OBJECTS): INCLUDES := $(INCLUDES) $(selectClosestPointLocator_EXTRA_INCLUDES)

depend_selectClosestPointLocator:     INCLUDES := $(INCLUDES) $(selectClosestPointLocator_EXTRA_INCLUDES)

$(selectClosestPointLocator_PLUGIN):  LFLAGS   := $(LFLAGS) $(selectClosestPointLocator_EXTRA_LFLAGS) 
$(selectClosestPointLocator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL $(selectClosestPointLocator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_selectClosestPointLocator clean_selectClosestPointLocator Clean_selectClosestPointLocator


$(selectClosestPointLocator_PLUGIN): $(selectClosestPointLocator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_selectClosestPointLocator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(selectClosestPointLocator_SOURCES)

clean_selectClosestPointLocator:
	-rm -f $(selectClosestPointLocator_OBJECTS)

Clean_selectClosestPointLocator:
	-rm -f $(selectClosestPointLocator_MAKEFILE).bak $(selectClosestPointLocator_OBJECTS) $(selectClosestPointLocator_PLUGIN)


plugins: $(selectClosestPointLocator_PLUGIN)
depend:	 depend_selectClosestPointLocator
clean:	 clean_selectClosestPointLocator
Clean:	 Clean_selectClosestPointLocator

