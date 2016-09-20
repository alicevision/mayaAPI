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

SRCDIR := $(TOP)/vertexBufferGenerator
DSTDIR := $(TOP)/vertexBufferGenerator

vertexBufferGenerator_SOURCES  := $(TOP)/vertexBufferGenerator/vertexBufferGenerator.cpp
vertexBufferGenerator_OBJECTS  := $(TOP)/vertexBufferGenerator/vertexBufferGenerator.o
vertexBufferGenerator_PLUGIN   := $(DSTDIR)/vertexBufferGenerator.$(EXT)
vertexBufferGenerator_MAKEFILE := $(DSTDIR)/Makefile

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

$(vertexBufferGenerator_OBJECTS): CFLAGS   := $(CFLAGS)   $(vertexBufferGenerator_EXTRA_CFLAGS)
$(vertexBufferGenerator_OBJECTS): C++FLAGS := $(C++FLAGS) $(vertexBufferGenerator_EXTRA_C++FLAGS)
$(vertexBufferGenerator_OBJECTS): INCLUDES := $(INCLUDES) $(vertexBufferGenerator_EXTRA_INCLUDES)

depend_vertexBufferGenerator:     INCLUDES := $(INCLUDES) $(vertexBufferGenerator_EXTRA_INCLUDES)

$(vertexBufferGenerator_PLUGIN):  LFLAGS   := $(LFLAGS) $(vertexBufferGenerator_EXTRA_LFLAGS) 
$(vertexBufferGenerator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(vertexBufferGenerator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_vertexBufferGenerator clean_vertexBufferGenerator Clean_vertexBufferGenerator


$(vertexBufferGenerator_PLUGIN): $(vertexBufferGenerator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_vertexBufferGenerator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(vertexBufferGenerator_SOURCES)

clean_vertexBufferGenerator:
	-rm -f $(vertexBufferGenerator_OBJECTS)

Clean_vertexBufferGenerator:
	-rm -f $(vertexBufferGenerator_MAKEFILE).bak $(vertexBufferGenerator_OBJECTS) $(vertexBufferGenerator_PLUGIN)


plugins: $(vertexBufferGenerator_PLUGIN)
depend:	 depend_vertexBufferGenerator
clean:	 clean_vertexBufferGenerator
Clean:	 Clean_vertexBufferGenerator

