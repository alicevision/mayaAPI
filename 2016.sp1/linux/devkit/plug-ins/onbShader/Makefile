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

SRCDIR := $(TOP)/onbShader
DSTDIR := $(TOP)/onbShader

onbShader_SOURCES  := $(TOP)/onbShader/onbShader.cpp $(TOP)/onbShader/onbShaderOverride.cpp
onbShader_OBJECTS  := $(TOP)/onbShader/onbShader.o $(TOP)/onbShader/onbShaderOverride.o
onbShader_PLUGIN   := $(DSTDIR)/onbShader.$(EXT)
onbShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(onbShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(onbShader_EXTRA_CFLAGS)
$(onbShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(onbShader_EXTRA_C++FLAGS)
$(onbShader_OBJECTS): INCLUDES := $(INCLUDES) $(onbShader_EXTRA_INCLUDES)

depend_onbShader:     INCLUDES := $(INCLUDES) $(onbShader_EXTRA_INCLUDES)

$(onbShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(onbShader_EXTRA_LFLAGS) 
$(onbShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(onbShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_onbShader clean_onbShader Clean_onbShader


$(onbShader_PLUGIN): $(onbShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_onbShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(onbShader_SOURCES)

clean_onbShader:
	-rm -f $(onbShader_OBJECTS)

Clean_onbShader:
	-rm -f $(onbShader_MAKEFILE).bak $(onbShader_OBJECTS) $(onbShader_PLUGIN)


plugins: $(onbShader_PLUGIN)
depend:	 depend_onbShader
clean:	 clean_onbShader
Clean:	 Clean_onbShader

