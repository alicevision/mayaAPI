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

SRCDIR := $(TOP)/mixtureShader
DSTDIR := $(TOP)/mixtureShader

mixtureShader_SOURCES  := $(TOP)/mixtureShader/mixtureShader.cpp
mixtureShader_OBJECTS  := $(TOP)/mixtureShader/mixtureShader.o
mixtureShader_PLUGIN   := $(DSTDIR)/mixtureShader.$(EXT)
mixtureShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(mixtureShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(mixtureShader_EXTRA_CFLAGS)
$(mixtureShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(mixtureShader_EXTRA_C++FLAGS)
$(mixtureShader_OBJECTS): INCLUDES := $(INCLUDES) $(mixtureShader_EXTRA_INCLUDES)

depend_mixtureShader:     INCLUDES := $(INCLUDES) $(mixtureShader_EXTRA_INCLUDES)

$(mixtureShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(mixtureShader_EXTRA_LFLAGS) 
$(mixtureShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(mixtureShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_mixtureShader clean_mixtureShader Clean_mixtureShader


$(mixtureShader_PLUGIN): $(mixtureShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_mixtureShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(mixtureShader_SOURCES)

clean_mixtureShader:
	-rm -f $(mixtureShader_OBJECTS)

Clean_mixtureShader:
	-rm -f $(mixtureShader_MAKEFILE).bak $(mixtureShader_OBJECTS) $(mixtureShader_PLUGIN)


plugins: $(mixtureShader_PLUGIN)
depend:	 depend_mixtureShader
clean:	 clean_mixtureShader
Clean:	 Clean_mixtureShader

