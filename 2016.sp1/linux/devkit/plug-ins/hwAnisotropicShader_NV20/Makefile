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

SRCDIR := $(TOP)/hwAnisotropicShader_NV20
DSTDIR := $(TOP)/hwAnisotropicShader_NV20

hwAnisotropicShader_NV20_SOURCES  := $(TOP)/hwAnisotropicShader_NV20/hwAnisotropicShader_NV20.cpp $(TOP)/hwAnisotropicShader_NV20/MNormalMapConverter.cpp $(TOP)/hwAnisotropicShader_NV20/MTexture.cpp $(TOP)/hwAnisotropicShader_NV20/MTextureCache.cpp $(TOP)/hwAnisotropicShader_NV20/NodeMonitor.cpp $(TOP)/hwAnisotropicShader_NV20/ShadingConnection.cpp
hwAnisotropicShader_NV20_OBJECTS  := $(TOP)/hwAnisotropicShader_NV20/hwAnisotropicShader_NV20.o $(TOP)/hwAnisotropicShader_NV20/MNormalMapConverter.o $(TOP)/hwAnisotropicShader_NV20/MTexture.o $(TOP)/hwAnisotropicShader_NV20/MTextureCache.o $(TOP)/hwAnisotropicShader_NV20/NodeMonitor.o $(TOP)/hwAnisotropicShader_NV20/ShadingConnection.o
hwAnisotropicShader_NV20_PLUGIN   := $(DSTDIR)/hwAnisotropicShader_NV20.$(EXT)
hwAnisotropicShader_NV20_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwAnisotropicShader_NV20_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwAnisotropicShader_NV20_EXTRA_CFLAGS)
$(hwAnisotropicShader_NV20_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwAnisotropicShader_NV20_EXTRA_C++FLAGS)
$(hwAnisotropicShader_NV20_OBJECTS): INCLUDES := $(INCLUDES) $(hwAnisotropicShader_NV20_EXTRA_INCLUDES)

depend_hwAnisotropicShader_NV20:     INCLUDES := $(INCLUDES) $(hwAnisotropicShader_NV20_EXTRA_INCLUDES)

$(hwAnisotropicShader_NV20_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwAnisotropicShader_NV20_EXTRA_LFLAGS) 
$(hwAnisotropicShader_NV20_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL -lGLU $(hwAnisotropicShader_NV20_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwAnisotropicShader_NV20 clean_hwAnisotropicShader_NV20 Clean_hwAnisotropicShader_NV20


$(hwAnisotropicShader_NV20_PLUGIN): $(hwAnisotropicShader_NV20_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwAnisotropicShader_NV20 :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwAnisotropicShader_NV20_SOURCES)

clean_hwAnisotropicShader_NV20:
	-rm -f $(hwAnisotropicShader_NV20_OBJECTS)

Clean_hwAnisotropicShader_NV20:
	-rm -f $(hwAnisotropicShader_NV20_MAKEFILE).bak $(hwAnisotropicShader_NV20_OBJECTS) $(hwAnisotropicShader_NV20_PLUGIN)


thirdPartyPlugins: $(hwAnisotropicShader_NV20_PLUGIN)
depend:	 depend_hwAnisotropicShader_NV20
clean:	 clean_hwAnisotropicShader_NV20
Clean:	 Clean_hwAnisotropicShader_NV20

