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

SRCDIR := $(TOP)/hwPhongShader
DSTDIR := $(TOP)/hwPhongShader

hwPhongShader_SOURCES  := $(TOP)/hwPhongShader/hwPhongShader.cpp $(TOP)/hwPhongShader/hwPhongShaderBehavior.cpp
hwPhongShader_OBJECTS  := $(TOP)/hwPhongShader/hwPhongShader.o $(TOP)/hwPhongShader/hwPhongShaderBehavior.o
hwPhongShader_PLUGIN   := $(DSTDIR)/hwPhongShader.$(EXT)
hwPhongShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwPhongShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwPhongShader_EXTRA_CFLAGS)
$(hwPhongShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwPhongShader_EXTRA_C++FLAGS)
$(hwPhongShader_OBJECTS): INCLUDES := $(INCLUDES) $(hwPhongShader_EXTRA_INCLUDES)

depend_hwPhongShader:     INCLUDES := $(INCLUDES) $(hwPhongShader_EXTRA_INCLUDES)

$(hwPhongShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwPhongShader_EXTRA_LFLAGS) 
$(hwPhongShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(hwPhongShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwPhongShader clean_hwPhongShader Clean_hwPhongShader


$(hwPhongShader_PLUGIN): $(hwPhongShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwPhongShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwPhongShader_SOURCES)

clean_hwPhongShader:
	-rm -f $(hwPhongShader_OBJECTS)

Clean_hwPhongShader:
	-rm -f $(hwPhongShader_MAKEFILE).bak $(hwPhongShader_OBJECTS) $(hwPhongShader_PLUGIN)


plugins: $(hwPhongShader_PLUGIN)
depend:	 depend_hwPhongShader
clean:	 clean_hwPhongShader
Clean:	 Clean_hwPhongShader

