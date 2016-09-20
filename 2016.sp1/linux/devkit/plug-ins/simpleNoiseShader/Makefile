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

SRCDIR := $(TOP)/simpleNoiseShader
DSTDIR := $(TOP)/simpleNoiseShader

simpleNoiseShader_SOURCES  := $(TOP)/simpleNoiseShader/simpleNoiseShader.cpp $(TOP)/simpleNoiseShader/simpleNoiseShaderOverride.cpp
simpleNoiseShader_OBJECTS  := $(TOP)/simpleNoiseShader/simpleNoiseShader.o $(TOP)/simpleNoiseShader/simpleNoiseShaderOverride.o
simpleNoiseShader_PLUGIN   := $(DSTDIR)/simpleNoiseShader.$(EXT)
simpleNoiseShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleNoiseShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleNoiseShader_EXTRA_CFLAGS)
$(simpleNoiseShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleNoiseShader_EXTRA_C++FLAGS)
$(simpleNoiseShader_OBJECTS): INCLUDES := $(INCLUDES) $(simpleNoiseShader_EXTRA_INCLUDES)

depend_simpleNoiseShader:     INCLUDES := $(INCLUDES) $(simpleNoiseShader_EXTRA_INCLUDES)

$(simpleNoiseShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleNoiseShader_EXTRA_LFLAGS) 
$(simpleNoiseShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(simpleNoiseShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleNoiseShader clean_simpleNoiseShader Clean_simpleNoiseShader


$(simpleNoiseShader_PLUGIN): $(simpleNoiseShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleNoiseShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleNoiseShader_SOURCES)

clean_simpleNoiseShader:
	-rm -f $(simpleNoiseShader_OBJECTS)

Clean_simpleNoiseShader:
	-rm -f $(simpleNoiseShader_MAKEFILE).bak $(simpleNoiseShader_OBJECTS) $(simpleNoiseShader_PLUGIN)


plugins: $(simpleNoiseShader_PLUGIN)
depend:	 depend_simpleNoiseShader
clean:	 clean_simpleNoiseShader
Clean:	 Clean_simpleNoiseShader

