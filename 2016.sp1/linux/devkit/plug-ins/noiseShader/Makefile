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

SRCDIR := $(TOP)/noiseShader
DSTDIR := $(TOP)/noiseShader

noiseShader_SOURCES  := $(TOP)/noiseShader/noiseShader.cpp
noiseShader_OBJECTS  := $(TOP)/noiseShader/noiseShader.o
noiseShader_PLUGIN   := $(DSTDIR)/noiseShader.$(EXT)
noiseShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(noiseShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(noiseShader_EXTRA_CFLAGS)
$(noiseShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(noiseShader_EXTRA_C++FLAGS)
$(noiseShader_OBJECTS): INCLUDES := $(INCLUDES) $(noiseShader_EXTRA_INCLUDES)

depend_noiseShader:     INCLUDES := $(INCLUDES) $(noiseShader_EXTRA_INCLUDES)

$(noiseShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(noiseShader_EXTRA_LFLAGS) 
$(noiseShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(noiseShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_noiseShader clean_noiseShader Clean_noiseShader


$(noiseShader_PLUGIN): $(noiseShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_noiseShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(noiseShader_SOURCES)

clean_noiseShader:
	-rm -f $(noiseShader_OBJECTS)

Clean_noiseShader:
	-rm -f $(noiseShader_MAKEFILE).bak $(noiseShader_OBJECTS) $(noiseShader_PLUGIN)


plugins: $(noiseShader_PLUGIN)
depend:	 depend_noiseShader
clean:	 clean_noiseShader
Clean:	 Clean_noiseShader

