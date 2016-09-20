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

SRCDIR := $(TOP)/hwColorPerVertexShader
DSTDIR := $(TOP)/hwColorPerVertexShader

hwColorPerVertexShader_SOURCES  := $(TOP)/hwColorPerVertexShader/hwColorPerVertexShader.cpp
hwColorPerVertexShader_OBJECTS  := $(TOP)/hwColorPerVertexShader/hwColorPerVertexShader.o
hwColorPerVertexShader_PLUGIN   := $(DSTDIR)/hwColorPerVertexShader.$(EXT)
hwColorPerVertexShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwColorPerVertexShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwColorPerVertexShader_EXTRA_CFLAGS)
$(hwColorPerVertexShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwColorPerVertexShader_EXTRA_C++FLAGS)
$(hwColorPerVertexShader_OBJECTS): INCLUDES := $(INCLUDES) $(hwColorPerVertexShader_EXTRA_INCLUDES)

depend_hwColorPerVertexShader:     INCLUDES := $(INCLUDES) $(hwColorPerVertexShader_EXTRA_INCLUDES)

$(hwColorPerVertexShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwColorPerVertexShader_EXTRA_LFLAGS) 
$(hwColorPerVertexShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL $(hwColorPerVertexShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwColorPerVertexShader clean_hwColorPerVertexShader Clean_hwColorPerVertexShader


$(hwColorPerVertexShader_PLUGIN): $(hwColorPerVertexShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwColorPerVertexShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwColorPerVertexShader_SOURCES)

clean_hwColorPerVertexShader:
	-rm -f $(hwColorPerVertexShader_OBJECTS)

Clean_hwColorPerVertexShader:
	-rm -f $(hwColorPerVertexShader_MAKEFILE).bak $(hwColorPerVertexShader_OBJECTS) $(hwColorPerVertexShader_PLUGIN)


plugins: $(hwColorPerVertexShader_PLUGIN)
depend:	 depend_hwColorPerVertexShader
clean:	 clean_hwColorPerVertexShader
Clean:	 Clean_hwColorPerVertexShader

