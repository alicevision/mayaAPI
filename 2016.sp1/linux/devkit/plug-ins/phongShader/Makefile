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

SRCDIR := $(TOP)/phongShader
DSTDIR := $(TOP)/phongShader

phongShader_SOURCES  := $(TOP)/phongShader/phongShader.cpp $(TOP)/phongShader/phongShaderOverride.cpp
phongShader_OBJECTS  := $(TOP)/phongShader/phongShader.o $(TOP)/phongShader/phongShaderOverride.o
phongShader_PLUGIN   := $(DSTDIR)/phongShader.$(EXT)
phongShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(phongShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(phongShader_EXTRA_CFLAGS)
$(phongShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(phongShader_EXTRA_C++FLAGS)
$(phongShader_OBJECTS): INCLUDES := $(INCLUDES) $(phongShader_EXTRA_INCLUDES)

depend_phongShader:     INCLUDES := $(INCLUDES) $(phongShader_EXTRA_INCLUDES)

$(phongShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(phongShader_EXTRA_LFLAGS) 
$(phongShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(phongShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_phongShader clean_phongShader Clean_phongShader


$(phongShader_PLUGIN): $(phongShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_phongShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(phongShader_SOURCES)

clean_phongShader:
	-rm -f $(phongShader_OBJECTS)

Clean_phongShader:
	-rm -f $(phongShader_MAKEFILE).bak $(phongShader_OBJECTS) $(phongShader_PLUGIN)


plugins: $(phongShader_PLUGIN)
depend:	 depend_phongShader
clean:	 clean_phongShader
Clean:	 Clean_phongShader

