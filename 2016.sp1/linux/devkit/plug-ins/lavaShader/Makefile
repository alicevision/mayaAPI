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

SRCDIR := $(TOP)/lavaShader
DSTDIR := $(TOP)/lavaShader

lavaShader_SOURCES  := $(TOP)/lavaShader/lavaShader.cpp
lavaShader_OBJECTS  := $(TOP)/lavaShader/lavaShader.o
lavaShader_PLUGIN   := $(DSTDIR)/lavaShader.$(EXT)
lavaShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(lavaShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(lavaShader_EXTRA_CFLAGS)
$(lavaShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(lavaShader_EXTRA_C++FLAGS)
$(lavaShader_OBJECTS): INCLUDES := $(INCLUDES) $(lavaShader_EXTRA_INCLUDES)

depend_lavaShader:     INCLUDES := $(INCLUDES) $(lavaShader_EXTRA_INCLUDES)

$(lavaShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(lavaShader_EXTRA_LFLAGS) 
$(lavaShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(lavaShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lavaShader clean_lavaShader Clean_lavaShader


$(lavaShader_PLUGIN): $(lavaShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lavaShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lavaShader_SOURCES)

clean_lavaShader:
	-rm -f $(lavaShader_OBJECTS)

Clean_lavaShader:
	-rm -f $(lavaShader_MAKEFILE).bak $(lavaShader_OBJECTS) $(lavaShader_PLUGIN)


plugins: $(lavaShader_PLUGIN)
depend:	 depend_lavaShader
clean:	 clean_lavaShader
Clean:	 Clean_lavaShader

