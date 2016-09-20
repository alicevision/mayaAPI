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

SRCDIR := $(TOP)/lightShader
DSTDIR := $(TOP)/lightShader

lightShader_SOURCES  := $(TOP)/lightShader/lightShader.cpp
lightShader_OBJECTS  := $(TOP)/lightShader/lightShader.o
lightShader_PLUGIN   := $(DSTDIR)/lightShader.$(EXT)
lightShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(lightShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(lightShader_EXTRA_CFLAGS)
$(lightShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(lightShader_EXTRA_C++FLAGS)
$(lightShader_OBJECTS): INCLUDES := $(INCLUDES) $(lightShader_EXTRA_INCLUDES)

depend_lightShader:     INCLUDES := $(INCLUDES) $(lightShader_EXTRA_INCLUDES)

$(lightShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(lightShader_EXTRA_LFLAGS) 
$(lightShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(lightShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lightShader clean_lightShader Clean_lightShader


$(lightShader_PLUGIN): $(lightShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lightShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lightShader_SOURCES)

clean_lightShader:
	-rm -f $(lightShader_OBJECTS)

Clean_lightShader:
	-rm -f $(lightShader_MAKEFILE).bak $(lightShader_OBJECTS) $(lightShader_PLUGIN)


plugins: $(lightShader_PLUGIN)
depend:	 depend_lightShader
clean:	 clean_lightShader
Clean:	 Clean_lightShader

