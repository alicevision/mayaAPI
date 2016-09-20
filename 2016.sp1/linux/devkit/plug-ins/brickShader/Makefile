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

SRCDIR := $(TOP)/brickShader
DSTDIR := $(TOP)/brickShader

brickShader_SOURCES  := $(TOP)/brickShader/brickShader.cpp
brickShader_OBJECTS  := $(TOP)/brickShader/brickShader.o
brickShader_PLUGIN   := $(DSTDIR)/brickShader.$(EXT)
brickShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(brickShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(brickShader_EXTRA_CFLAGS)
$(brickShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(brickShader_EXTRA_C++FLAGS)
$(brickShader_OBJECTS): INCLUDES := $(INCLUDES) $(brickShader_EXTRA_INCLUDES)

depend_brickShader:     INCLUDES := $(INCLUDES) $(brickShader_EXTRA_INCLUDES)

$(brickShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(brickShader_EXTRA_LFLAGS) 
$(brickShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(brickShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_brickShader clean_brickShader Clean_brickShader


$(brickShader_PLUGIN): $(brickShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_brickShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(brickShader_SOURCES)

clean_brickShader:
	-rm -f $(brickShader_OBJECTS)

Clean_brickShader:
	-rm -f $(brickShader_MAKEFILE).bak $(brickShader_OBJECTS) $(brickShader_PLUGIN)


plugins: $(brickShader_PLUGIN)
depend:	 depend_brickShader
clean:	 clean_brickShader
Clean:	 Clean_brickShader

