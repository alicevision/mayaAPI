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

SRCDIR := $(TOP)/shadowMatteShader
DSTDIR := $(TOP)/shadowMatteShader

shadowMatteShader_SOURCES  := $(TOP)/shadowMatteShader/shadowMatteShader.cpp
shadowMatteShader_OBJECTS  := $(TOP)/shadowMatteShader/shadowMatteShader.o
shadowMatteShader_PLUGIN   := $(DSTDIR)/shadowMatteShader.$(EXT)
shadowMatteShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(shadowMatteShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(shadowMatteShader_EXTRA_CFLAGS)
$(shadowMatteShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(shadowMatteShader_EXTRA_C++FLAGS)
$(shadowMatteShader_OBJECTS): INCLUDES := $(INCLUDES) $(shadowMatteShader_EXTRA_INCLUDES)

depend_shadowMatteShader:     INCLUDES := $(INCLUDES) $(shadowMatteShader_EXTRA_INCLUDES)

$(shadowMatteShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(shadowMatteShader_EXTRA_LFLAGS) 
$(shadowMatteShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(shadowMatteShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_shadowMatteShader clean_shadowMatteShader Clean_shadowMatteShader


$(shadowMatteShader_PLUGIN): $(shadowMatteShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_shadowMatteShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(shadowMatteShader_SOURCES)

clean_shadowMatteShader:
	-rm -f $(shadowMatteShader_OBJECTS)

Clean_shadowMatteShader:
	-rm -f $(shadowMatteShader_MAKEFILE).bak $(shadowMatteShader_OBJECTS) $(shadowMatteShader_PLUGIN)


plugins: $(shadowMatteShader_PLUGIN)
depend:	 depend_shadowMatteShader
clean:	 clean_shadowMatteShader
Clean:	 Clean_shadowMatteShader

