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

SRCDIR := $(TOP)/geomShader
DSTDIR := $(TOP)/geomShader

geomShader_SOURCES  := $(TOP)/geomShader/geomShader.cpp
geomShader_OBJECTS  := $(TOP)/geomShader/geomShader.o
geomShader_PLUGIN   := $(DSTDIR)/geomShader.$(EXT)
geomShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(geomShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(geomShader_EXTRA_CFLAGS)
$(geomShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(geomShader_EXTRA_C++FLAGS)
$(geomShader_OBJECTS): INCLUDES := $(INCLUDES) $(geomShader_EXTRA_INCLUDES)

depend_geomShader:     INCLUDES := $(INCLUDES) $(geomShader_EXTRA_INCLUDES)

$(geomShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(geomShader_EXTRA_LFLAGS) 
$(geomShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(geomShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_geomShader clean_geomShader Clean_geomShader


$(geomShader_PLUGIN): $(geomShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_geomShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(geomShader_SOURCES)

clean_geomShader:
	-rm -f $(geomShader_OBJECTS)

Clean_geomShader:
	-rm -f $(geomShader_MAKEFILE).bak $(geomShader_OBJECTS) $(geomShader_PLUGIN)


plugins: $(geomShader_PLUGIN)
depend:	 depend_geomShader
clean:	 clean_geomShader
Clean:	 Clean_geomShader

