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

SRCDIR := $(TOP)/slopeShader
DSTDIR := $(TOP)/slopeShader

slopeShader_SOURCES  := $(TOP)/slopeShader/slopeShader.cpp $(TOP)/slopeShader/slopeShaderBehavior.cpp $(TOP)/slopeShader/slopeShaderNode.cpp
slopeShader_OBJECTS  := $(TOP)/slopeShader/slopeShader.o $(TOP)/slopeShader/slopeShaderBehavior.o $(TOP)/slopeShader/slopeShaderNode.o
slopeShader_PLUGIN   := $(DSTDIR)/slopeShader.$(EXT)
slopeShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(slopeShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(slopeShader_EXTRA_CFLAGS)
$(slopeShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(slopeShader_EXTRA_C++FLAGS)
$(slopeShader_OBJECTS): INCLUDES := $(INCLUDES) $(slopeShader_EXTRA_INCLUDES)

depend_slopeShader:     INCLUDES := $(INCLUDES) $(slopeShader_EXTRA_INCLUDES)

$(slopeShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(slopeShader_EXTRA_LFLAGS) 
$(slopeShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(slopeShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_slopeShader clean_slopeShader Clean_slopeShader


$(slopeShader_PLUGIN): $(slopeShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_slopeShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(slopeShader_SOURCES)

clean_slopeShader:
	-rm -f $(slopeShader_OBJECTS)

Clean_slopeShader:
	-rm -f $(slopeShader_MAKEFILE).bak $(slopeShader_OBJECTS) $(slopeShader_PLUGIN)


plugins: $(slopeShader_PLUGIN)
depend:	 depend_slopeShader
clean:	 clean_slopeShader
Clean:	 Clean_slopeShader

