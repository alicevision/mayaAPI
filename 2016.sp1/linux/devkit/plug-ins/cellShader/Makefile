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

SRCDIR := $(TOP)/cellShader
DSTDIR := $(TOP)/cellShader

cellShader_SOURCES  := $(TOP)/cellShader/cellShader.cpp
cellShader_OBJECTS  := $(TOP)/cellShader/cellShader.o
cellShader_PLUGIN   := $(DSTDIR)/cellShader.$(EXT)
cellShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(cellShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(cellShader_EXTRA_CFLAGS)
$(cellShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(cellShader_EXTRA_C++FLAGS)
$(cellShader_OBJECTS): INCLUDES := $(INCLUDES) $(cellShader_EXTRA_INCLUDES)

depend_cellShader:     INCLUDES := $(INCLUDES) $(cellShader_EXTRA_INCLUDES)

$(cellShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(cellShader_EXTRA_LFLAGS) 
$(cellShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(cellShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cellShader clean_cellShader Clean_cellShader


$(cellShader_PLUGIN): $(cellShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cellShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cellShader_SOURCES)

clean_cellShader:
	-rm -f $(cellShader_OBJECTS)

Clean_cellShader:
	-rm -f $(cellShader_MAKEFILE).bak $(cellShader_OBJECTS) $(cellShader_PLUGIN)


plugins: $(cellShader_PLUGIN)
depend:	 depend_cellShader
clean:	 clean_cellShader
Clean:	 Clean_cellShader

