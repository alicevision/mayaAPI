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
all : thirdPartyPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/hwReflectBumpShader_NV20
DSTDIR := $(TOP)/hwReflectBumpShader_NV20

hwReflectBumpShader_NV20_SOURCES  := $(TOP)/hwReflectBumpShader_NV20/hwReflectBumpShader_NV20.cpp $(TOP)/hwReflectBumpShader_NV20/MNormalMapConverter.cpp $(TOP)/hwReflectBumpShader_NV20/MTexture.cpp $(TOP)/hwReflectBumpShader_NV20/MTextureCache.cpp $(TOP)/hwReflectBumpShader_NV20/NodeMonitor.cpp $(TOP)/hwReflectBumpShader_NV20/ShadingConnection.cpp
hwReflectBumpShader_NV20_OBJECTS  := $(TOP)/hwReflectBumpShader_NV20/hwReflectBumpShader_NV20.o $(TOP)/hwReflectBumpShader_NV20/MNormalMapConverter.o $(TOP)/hwReflectBumpShader_NV20/MTexture.o $(TOP)/hwReflectBumpShader_NV20/MTextureCache.o $(TOP)/hwReflectBumpShader_NV20/NodeMonitor.o $(TOP)/hwReflectBumpShader_NV20/ShadingConnection.o
hwReflectBumpShader_NV20_PLUGIN   := $(DSTDIR)/hwReflectBumpShader_NV20.$(EXT)
hwReflectBumpShader_NV20_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwReflectBumpShader_NV20_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwReflectBumpShader_NV20_EXTRA_CFLAGS)
$(hwReflectBumpShader_NV20_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwReflectBumpShader_NV20_EXTRA_C++FLAGS)
$(hwReflectBumpShader_NV20_OBJECTS): INCLUDES := $(INCLUDES) $(hwReflectBumpShader_NV20_EXTRA_INCLUDES)

depend_hwReflectBumpShader_NV20:     INCLUDES := $(INCLUDES) $(hwReflectBumpShader_NV20_EXTRA_INCLUDES)

$(hwReflectBumpShader_NV20_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwReflectBumpShader_NV20_EXTRA_LFLAGS) 
$(hwReflectBumpShader_NV20_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL -lGLU $(hwReflectBumpShader_NV20_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwReflectBumpShader_NV20 clean_hwReflectBumpShader_NV20 Clean_hwReflectBumpShader_NV20


$(hwReflectBumpShader_NV20_PLUGIN): $(hwReflectBumpShader_NV20_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwReflectBumpShader_NV20 :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwReflectBumpShader_NV20_SOURCES)

clean_hwReflectBumpShader_NV20:
	-rm -f $(hwReflectBumpShader_NV20_OBJECTS)

Clean_hwReflectBumpShader_NV20:
	-rm -f $(hwReflectBumpShader_NV20_MAKEFILE).bak $(hwReflectBumpShader_NV20_OBJECTS) $(hwReflectBumpShader_NV20_PLUGIN)


thirdPartyPlugins: $(hwReflectBumpShader_NV20_PLUGIN)
depend:	 depend_hwReflectBumpShader_NV20
clean:	 clean_hwReflectBumpShader_NV20
Clean:	 Clean_hwReflectBumpShader_NV20

