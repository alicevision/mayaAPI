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

SRCDIR := $(TOP)/depthShader
DSTDIR := $(TOP)/depthShader

depthShader_SOURCES  := $(TOP)/depthShader/depthShader.cpp
depthShader_OBJECTS  := $(TOP)/depthShader/depthShader.o
depthShader_PLUGIN   := $(DSTDIR)/depthShader.$(EXT)
depthShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(depthShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(depthShader_EXTRA_CFLAGS)
$(depthShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(depthShader_EXTRA_C++FLAGS)
$(depthShader_OBJECTS): INCLUDES := $(INCLUDES) $(depthShader_EXTRA_INCLUDES)

depend_depthShader:     INCLUDES := $(INCLUDES) $(depthShader_EXTRA_INCLUDES)

$(depthShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(depthShader_EXTRA_LFLAGS) 
$(depthShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(depthShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_depthShader clean_depthShader Clean_depthShader


$(depthShader_PLUGIN): $(depthShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_depthShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(depthShader_SOURCES)

clean_depthShader:
	-rm -f $(depthShader_OBJECTS)

Clean_depthShader:
	-rm -f $(depthShader_MAKEFILE).bak $(depthShader_OBJECTS) $(depthShader_PLUGIN)


plugins: $(depthShader_PLUGIN)
depend:	 depend_depthShader
clean:	 clean_depthShader
Clean:	 Clean_depthShader

