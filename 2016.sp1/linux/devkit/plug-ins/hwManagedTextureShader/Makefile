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

SRCDIR := $(TOP)/hwManagedTextureShader
DSTDIR := $(TOP)/hwManagedTextureShader

hwManagedTextureShader_SOURCES  := $(TOP)/hwManagedTextureShader/hwManagedTextureShader.cpp
hwManagedTextureShader_OBJECTS  := $(TOP)/hwManagedTextureShader/hwManagedTextureShader.o
hwManagedTextureShader_PLUGIN   := $(DSTDIR)/hwManagedTextureShader.$(EXT)
hwManagedTextureShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwManagedTextureShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwManagedTextureShader_EXTRA_CFLAGS)
$(hwManagedTextureShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwManagedTextureShader_EXTRA_C++FLAGS)
$(hwManagedTextureShader_OBJECTS): INCLUDES := $(INCLUDES) $(hwManagedTextureShader_EXTRA_INCLUDES)

depend_hwManagedTextureShader:     INCLUDES := $(INCLUDES) $(hwManagedTextureShader_EXTRA_INCLUDES)

$(hwManagedTextureShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwManagedTextureShader_EXTRA_LFLAGS) 
$(hwManagedTextureShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL $(hwManagedTextureShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwManagedTextureShader clean_hwManagedTextureShader Clean_hwManagedTextureShader


$(hwManagedTextureShader_PLUGIN): $(hwManagedTextureShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwManagedTextureShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwManagedTextureShader_SOURCES)

clean_hwManagedTextureShader:
	-rm -f $(hwManagedTextureShader_OBJECTS)

Clean_hwManagedTextureShader:
	-rm -f $(hwManagedTextureShader_MAKEFILE).bak $(hwManagedTextureShader_OBJECTS) $(hwManagedTextureShader_PLUGIN)


plugins: $(hwManagedTextureShader_PLUGIN)
depend:	 depend_hwManagedTextureShader
clean:	 clean_hwManagedTextureShader
Clean:	 Clean_hwManagedTextureShader

