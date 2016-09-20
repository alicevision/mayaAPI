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

SRCDIR := $(TOP)/lambertShader
DSTDIR := $(TOP)/lambertShader

lambertShader_SOURCES  := $(TOP)/lambertShader/lambertShader.cpp $(TOP)/lambertShader/lambertShaderOverride.cpp
lambertShader_OBJECTS  := $(TOP)/lambertShader/lambertShader.o $(TOP)/lambertShader/lambertShaderOverride.o
lambertShader_PLUGIN   := $(DSTDIR)/lambertShader.$(EXT)
lambertShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(lambertShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(lambertShader_EXTRA_CFLAGS)
$(lambertShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(lambertShader_EXTRA_C++FLAGS)
$(lambertShader_OBJECTS): INCLUDES := $(INCLUDES) $(lambertShader_EXTRA_INCLUDES)

depend_lambertShader:     INCLUDES := $(INCLUDES) $(lambertShader_EXTRA_INCLUDES)

$(lambertShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(lambertShader_EXTRA_LFLAGS) 
$(lambertShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(lambertShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lambertShader clean_lambertShader Clean_lambertShader


$(lambertShader_PLUGIN): $(lambertShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lambertShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lambertShader_SOURCES)

clean_lambertShader:
	-rm -f $(lambertShader_OBJECTS)

Clean_lambertShader:
	-rm -f $(lambertShader_MAKEFILE).bak $(lambertShader_OBJECTS) $(lambertShader_PLUGIN)


plugins: $(lambertShader_PLUGIN)
depend:	 depend_lambertShader
clean:	 clean_lambertShader
Clean:	 Clean_lambertShader

