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

SRCDIR := $(TOP)/gammaShader
DSTDIR := $(TOP)/gammaShader

gammaShader_SOURCES  := $(TOP)/gammaShader/gammaShader.cpp
gammaShader_OBJECTS  := $(TOP)/gammaShader/gammaShader.o
gammaShader_PLUGIN   := $(DSTDIR)/gammaShader.$(EXT)
gammaShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(gammaShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(gammaShader_EXTRA_CFLAGS)
$(gammaShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(gammaShader_EXTRA_C++FLAGS)
$(gammaShader_OBJECTS): INCLUDES := $(INCLUDES) $(gammaShader_EXTRA_INCLUDES)

depend_gammaShader:     INCLUDES := $(INCLUDES) $(gammaShader_EXTRA_INCLUDES)

$(gammaShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(gammaShader_EXTRA_LFLAGS) 
$(gammaShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(gammaShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_gammaShader clean_gammaShader Clean_gammaShader


$(gammaShader_PLUGIN): $(gammaShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_gammaShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(gammaShader_SOURCES)

clean_gammaShader:
	-rm -f $(gammaShader_OBJECTS)

Clean_gammaShader:
	-rm -f $(gammaShader_MAKEFILE).bak $(gammaShader_OBJECTS) $(gammaShader_PLUGIN)


plugins: $(gammaShader_PLUGIN)
depend:	 depend_gammaShader
clean:	 clean_gammaShader
Clean:	 Clean_gammaShader

