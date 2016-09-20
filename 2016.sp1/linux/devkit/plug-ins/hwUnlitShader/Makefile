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

SRCDIR := $(TOP)/hwUnlitShader
DSTDIR := $(TOP)/hwUnlitShader

hwUnlitShader_SOURCES  := $(TOP)/hwUnlitShader/hwUnlitShader.cpp $(TOP)/hwUnlitShader/MNormalMapConverter.cpp $(TOP)/hwUnlitShader/MTexture.cpp $(TOP)/hwUnlitShader/MTextureCache.cpp $(TOP)/hwUnlitShader/NodeMonitor.cpp $(TOP)/hwUnlitShader/ShadingConnection.cpp
hwUnlitShader_OBJECTS  := $(TOP)/hwUnlitShader/hwUnlitShader.o $(TOP)/hwUnlitShader/MNormalMapConverter.o $(TOP)/hwUnlitShader/MTexture.o $(TOP)/hwUnlitShader/MTextureCache.o $(TOP)/hwUnlitShader/NodeMonitor.o $(TOP)/hwUnlitShader/ShadingConnection.o
hwUnlitShader_PLUGIN   := $(DSTDIR)/hwUnlitShader.$(EXT)
hwUnlitShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwUnlitShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwUnlitShader_EXTRA_CFLAGS)
$(hwUnlitShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwUnlitShader_EXTRA_C++FLAGS)
$(hwUnlitShader_OBJECTS): INCLUDES := $(INCLUDES) $(hwUnlitShader_EXTRA_INCLUDES)

depend_hwUnlitShader:     INCLUDES := $(INCLUDES) $(hwUnlitShader_EXTRA_INCLUDES)

$(hwUnlitShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwUnlitShader_EXTRA_LFLAGS) 
$(hwUnlitShader_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL -lGLU $(hwUnlitShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwUnlitShader clean_hwUnlitShader Clean_hwUnlitShader


$(hwUnlitShader_PLUGIN): $(hwUnlitShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwUnlitShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwUnlitShader_SOURCES)

clean_hwUnlitShader:
	-rm -f $(hwUnlitShader_OBJECTS)

Clean_hwUnlitShader:
	-rm -f $(hwUnlitShader_MAKEFILE).bak $(hwUnlitShader_OBJECTS) $(hwUnlitShader_PLUGIN)


thirdPartyPlugins: $(hwUnlitShader_PLUGIN)
depend:	 depend_hwUnlitShader
clean:	 clean_hwUnlitShader
Clean:	 Clean_hwUnlitShader

