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

SRCDIR := $(TOP)/blindDataShader
DSTDIR := $(TOP)/blindDataShader

blindDataShader_SOURCES  := $(TOP)/blindDataShader/blindDataMesh.cpp $(TOP)/blindDataShader/blindDataShader.cpp $(TOP)/blindDataShader/blindDataPluginMain.cpp
blindDataShader_OBJECTS  := $(TOP)/blindDataShader/blindDataMesh.o $(TOP)/blindDataShader/blindDataShader.o $(TOP)/blindDataShader/blindDataPluginMain.o
blindDataShader_PLUGIN   := $(DSTDIR)/blindDataShader.$(EXT)
blindDataShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(blindDataShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(blindDataShader_EXTRA_CFLAGS)
$(blindDataShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(blindDataShader_EXTRA_C++FLAGS)
$(blindDataShader_OBJECTS): INCLUDES := $(INCLUDES) $(blindDataShader_EXTRA_INCLUDES)

depend_blindDataShader:     INCLUDES := $(INCLUDES) $(blindDataShader_EXTRA_INCLUDES)

$(blindDataShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(blindDataShader_EXTRA_LFLAGS) 
$(blindDataShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL $(blindDataShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_blindDataShader clean_blindDataShader Clean_blindDataShader


$(blindDataShader_PLUGIN): $(blindDataShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_blindDataShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(blindDataShader_SOURCES)

clean_blindDataShader:
	-rm -f $(blindDataShader_OBJECTS)

Clean_blindDataShader:
	-rm -f $(blindDataShader_MAKEFILE).bak $(blindDataShader_OBJECTS) $(blindDataShader_PLUGIN)


plugins: $(blindDataShader_PLUGIN)
depend:	 depend_blindDataShader
clean:	 clean_blindDataShader
Clean:	 Clean_blindDataShader

