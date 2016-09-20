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

SRCDIR := $(TOP)/hwRefractReflectShader_NV20
DSTDIR := $(TOP)/hwRefractReflectShader_NV20

hwRefractReflectShader_NV20_SOURCES  := $(TOP)/hwRefractReflectShader_NV20/hwRefractReflectShader_NV20.cpp $(TOP)/hwRefractReflectShader_NV20/MNormalMapConverter.cpp $(TOP)/hwRefractReflectShader_NV20/MTexture.cpp $(TOP)/hwRefractReflectShader_NV20/MTextureCache.cpp $(TOP)/hwRefractReflectShader_NV20/NodeMonitor.cpp $(TOP)/hwRefractReflectShader_NV20/ShadingConnection.cpp
hwRefractReflectShader_NV20_OBJECTS  := $(TOP)/hwRefractReflectShader_NV20/hwRefractReflectShader_NV20.o $(TOP)/hwRefractReflectShader_NV20/MNormalMapConverter.o $(TOP)/hwRefractReflectShader_NV20/MTexture.o $(TOP)/hwRefractReflectShader_NV20/MTextureCache.o $(TOP)/hwRefractReflectShader_NV20/NodeMonitor.o $(TOP)/hwRefractReflectShader_NV20/ShadingConnection.o
hwRefractReflectShader_NV20_PLUGIN   := $(DSTDIR)/hwRefractReflectShader_NV20.$(EXT)
hwRefractReflectShader_NV20_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwRefractReflectShader_NV20_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwRefractReflectShader_NV20_EXTRA_CFLAGS)
$(hwRefractReflectShader_NV20_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwRefractReflectShader_NV20_EXTRA_C++FLAGS)
$(hwRefractReflectShader_NV20_OBJECTS): INCLUDES := $(INCLUDES) $(hwRefractReflectShader_NV20_EXTRA_INCLUDES)

depend_hwRefractReflectShader_NV20:     INCLUDES := $(INCLUDES) $(hwRefractReflectShader_NV20_EXTRA_INCLUDES)

$(hwRefractReflectShader_NV20_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwRefractReflectShader_NV20_EXTRA_LFLAGS) 
$(hwRefractReflectShader_NV20_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL -lGLU $(hwRefractReflectShader_NV20_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwRefractReflectShader_NV20 clean_hwRefractReflectShader_NV20 Clean_hwRefractReflectShader_NV20


$(hwRefractReflectShader_NV20_PLUGIN): $(hwRefractReflectShader_NV20_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwRefractReflectShader_NV20 :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwRefractReflectShader_NV20_SOURCES)

clean_hwRefractReflectShader_NV20:
	-rm -f $(hwRefractReflectShader_NV20_OBJECTS)

Clean_hwRefractReflectShader_NV20:
	-rm -f $(hwRefractReflectShader_NV20_MAKEFILE).bak $(hwRefractReflectShader_NV20_OBJECTS) $(hwRefractReflectShader_NV20_PLUGIN)


thirdPartyPlugins: $(hwRefractReflectShader_NV20_PLUGIN)
depend:	 depend_hwRefractReflectShader_NV20
clean:	 clean_hwRefractReflectShader_NV20
Clean:	 Clean_hwRefractReflectShader_NV20

