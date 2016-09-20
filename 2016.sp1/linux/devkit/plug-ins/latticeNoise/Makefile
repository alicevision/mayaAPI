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

SRCDIR := $(TOP)/latticeNoise
DSTDIR := $(TOP)/latticeNoise

latticeNoise_SOURCES  := $(TOP)/latticeNoise/latticeNoiseNode.cpp $(TOP)/latticeNoise/latticeNoiseCmd.cpp $(TOP)/latticeNoise/latticeNoise.cpp
latticeNoise_OBJECTS  := $(TOP)/latticeNoise/latticeNoiseNode.o $(TOP)/latticeNoise/latticeNoiseCmd.o $(TOP)/latticeNoise/latticeNoise.o
latticeNoise_PLUGIN   := $(DSTDIR)/latticeNoise.$(EXT)
latticeNoise_MAKEFILE := $(DSTDIR)/Makefile

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

$(latticeNoise_OBJECTS): CFLAGS   := $(CFLAGS)   $(latticeNoise_EXTRA_CFLAGS)
$(latticeNoise_OBJECTS): C++FLAGS := $(C++FLAGS) $(latticeNoise_EXTRA_C++FLAGS)
$(latticeNoise_OBJECTS): INCLUDES := $(INCLUDES) $(latticeNoise_EXTRA_INCLUDES)

depend_latticeNoise:     INCLUDES := $(INCLUDES) $(latticeNoise_EXTRA_INCLUDES)

$(latticeNoise_PLUGIN):  LFLAGS   := $(LFLAGS) $(latticeNoise_EXTRA_LFLAGS) 
$(latticeNoise_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(latticeNoise_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_latticeNoise clean_latticeNoise Clean_latticeNoise


$(latticeNoise_PLUGIN): $(latticeNoise_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_latticeNoise :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(latticeNoise_SOURCES)

clean_latticeNoise:
	-rm -f $(latticeNoise_OBJECTS)

Clean_latticeNoise:
	-rm -f $(latticeNoise_MAKEFILE).bak $(latticeNoise_OBJECTS) $(latticeNoise_PLUGIN)


plugins: $(latticeNoise_PLUGIN)
depend:	 depend_latticeNoise
clean:	 clean_latticeNoise
Clean:	 Clean_latticeNoise

