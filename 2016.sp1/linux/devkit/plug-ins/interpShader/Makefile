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

SRCDIR := $(TOP)/interpShader
DSTDIR := $(TOP)/interpShader

interpShader_SOURCES  := $(TOP)/interpShader/interpShader.cpp
interpShader_OBJECTS  := $(TOP)/interpShader/interpShader.o
interpShader_PLUGIN   := $(DSTDIR)/interpShader.$(EXT)
interpShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(interpShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(interpShader_EXTRA_CFLAGS)
$(interpShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(interpShader_EXTRA_C++FLAGS)
$(interpShader_OBJECTS): INCLUDES := $(INCLUDES) $(interpShader_EXTRA_INCLUDES)

depend_interpShader:     INCLUDES := $(INCLUDES) $(interpShader_EXTRA_INCLUDES)

$(interpShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(interpShader_EXTRA_LFLAGS) 
$(interpShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(interpShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_interpShader clean_interpShader Clean_interpShader


$(interpShader_PLUGIN): $(interpShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_interpShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(interpShader_SOURCES)

clean_interpShader:
	-rm -f $(interpShader_OBJECTS)

Clean_interpShader:
	-rm -f $(interpShader_MAKEFILE).bak $(interpShader_OBJECTS) $(interpShader_PLUGIN)


plugins: $(interpShader_PLUGIN)
depend:	 depend_interpShader
clean:	 clean_interpShader
Clean:	 Clean_interpShader

