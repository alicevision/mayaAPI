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

SRCDIR := $(TOP)/hwRenderPostEffect
DSTDIR := $(TOP)/hwRenderPostEffect

hwRenderPostEffect_SOURCES  := $(TOP)/hwRenderPostEffect/hwRenderPostEffect.cpp
hwRenderPostEffect_OBJECTS  := $(TOP)/hwRenderPostEffect/hwRenderPostEffect.o
hwRenderPostEffect_PLUGIN   := $(DSTDIR)/hwRenderPostEffect.$(EXT)
hwRenderPostEffect_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwRenderPostEffect_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwRenderPostEffect_EXTRA_CFLAGS)
$(hwRenderPostEffect_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwRenderPostEffect_EXTRA_C++FLAGS)
$(hwRenderPostEffect_OBJECTS): INCLUDES := $(INCLUDES) $(hwRenderPostEffect_EXTRA_INCLUDES)

depend_hwRenderPostEffect:     INCLUDES := $(INCLUDES) $(hwRenderPostEffect_EXTRA_INCLUDES)

$(hwRenderPostEffect_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwRenderPostEffect_EXTRA_LFLAGS) 
$(hwRenderPostEffect_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation -lGL -lGLU $(hwRenderPostEffect_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwRenderPostEffect clean_hwRenderPostEffect Clean_hwRenderPostEffect


$(hwRenderPostEffect_PLUGIN): $(hwRenderPostEffect_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwRenderPostEffect :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwRenderPostEffect_SOURCES)

clean_hwRenderPostEffect:
	-rm -f $(hwRenderPostEffect_OBJECTS)

Clean_hwRenderPostEffect:
	-rm -f $(hwRenderPostEffect_MAKEFILE).bak $(hwRenderPostEffect_OBJECTS) $(hwRenderPostEffect_PLUGIN)


plugins: $(hwRenderPostEffect_PLUGIN)
depend:	 depend_hwRenderPostEffect
clean:	 clean_hwRenderPostEffect
Clean:	 Clean_hwRenderPostEffect

