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

SRCDIR := $(TOP)/volumeShader
DSTDIR := $(TOP)/volumeShader

volumeShader_SOURCES  := $(TOP)/volumeShader/volumeShader.cpp
volumeShader_OBJECTS  := $(TOP)/volumeShader/volumeShader.o
volumeShader_PLUGIN   := $(DSTDIR)/volumeShader.$(EXT)
volumeShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(volumeShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(volumeShader_EXTRA_CFLAGS)
$(volumeShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(volumeShader_EXTRA_C++FLAGS)
$(volumeShader_OBJECTS): INCLUDES := $(INCLUDES) $(volumeShader_EXTRA_INCLUDES)

depend_volumeShader:     INCLUDES := $(INCLUDES) $(volumeShader_EXTRA_INCLUDES)

$(volumeShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(volumeShader_EXTRA_LFLAGS) 
$(volumeShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(volumeShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_volumeShader clean_volumeShader Clean_volumeShader


$(volumeShader_PLUGIN): $(volumeShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_volumeShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(volumeShader_SOURCES)

clean_volumeShader:
	-rm -f $(volumeShader_OBJECTS)

Clean_volumeShader:
	-rm -f $(volumeShader_MAKEFILE).bak $(volumeShader_OBJECTS) $(volumeShader_PLUGIN)


plugins: $(volumeShader_PLUGIN)
depend:	 depend_volumeShader
clean:	 clean_volumeShader
Clean:	 Clean_volumeShader

