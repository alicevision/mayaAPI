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

SRCDIR := $(TOP)/hwToonShader_NV20
DSTDIR := $(TOP)/hwToonShader_NV20

hwToonShader_NV20_SOURCES  := $(TOP)/hwToonShader_NV20/hwToonShader_NV20.cpp $(TOP)/hwToonShader_NV20/MNormalMapConverter.cpp $(TOP)/hwToonShader_NV20/MTexture.cpp $(TOP)/hwToonShader_NV20/MTextureCache.cpp $(TOP)/hwToonShader_NV20/NodeMonitor.cpp $(TOP)/hwToonShader_NV20/ShadingConnection.cpp
hwToonShader_NV20_OBJECTS  := $(TOP)/hwToonShader_NV20/hwToonShader_NV20.o $(TOP)/hwToonShader_NV20/MNormalMapConverter.o $(TOP)/hwToonShader_NV20/MTexture.o $(TOP)/hwToonShader_NV20/MTextureCache.o $(TOP)/hwToonShader_NV20/NodeMonitor.o $(TOP)/hwToonShader_NV20/ShadingConnection.o
hwToonShader_NV20_PLUGIN   := $(DSTDIR)/hwToonShader_NV20.$(EXT)
hwToonShader_NV20_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwToonShader_NV20_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwToonShader_NV20_EXTRA_CFLAGS)
$(hwToonShader_NV20_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwToonShader_NV20_EXTRA_C++FLAGS)
$(hwToonShader_NV20_OBJECTS): INCLUDES := $(INCLUDES) $(hwToonShader_NV20_EXTRA_INCLUDES)

depend_hwToonShader_NV20:     INCLUDES := $(INCLUDES) $(hwToonShader_NV20_EXTRA_INCLUDES)

$(hwToonShader_NV20_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwToonShader_NV20_EXTRA_LFLAGS) 
$(hwToonShader_NV20_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL -lGLU $(hwToonShader_NV20_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwToonShader_NV20 clean_hwToonShader_NV20 Clean_hwToonShader_NV20


$(hwToonShader_NV20_PLUGIN): $(hwToonShader_NV20_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwToonShader_NV20 :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwToonShader_NV20_SOURCES)

clean_hwToonShader_NV20:
	-rm -f $(hwToonShader_NV20_OBJECTS)

Clean_hwToonShader_NV20:
	-rm -f $(hwToonShader_NV20_MAKEFILE).bak $(hwToonShader_NV20_OBJECTS) $(hwToonShader_NV20_PLUGIN)


thirdPartyPlugins: $(hwToonShader_NV20_PLUGIN)
depend:	 depend_hwToonShader_NV20
clean:	 clean_hwToonShader_NV20
Clean:	 Clean_hwToonShader_NV20

