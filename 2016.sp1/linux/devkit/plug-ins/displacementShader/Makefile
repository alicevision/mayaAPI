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

SRCDIR := $(TOP)/displacementShader
DSTDIR := $(TOP)/displacementShader

displacementShader_SOURCES  := $(TOP)/displacementShader/displacementShader.cpp
displacementShader_OBJECTS  := $(TOP)/displacementShader/displacementShader.o
displacementShader_PLUGIN   := $(DSTDIR)/displacementShader.$(EXT)
displacementShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(displacementShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(displacementShader_EXTRA_CFLAGS)
$(displacementShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(displacementShader_EXTRA_C++FLAGS)
$(displacementShader_OBJECTS): INCLUDES := $(INCLUDES) $(displacementShader_EXTRA_INCLUDES)

depend_displacementShader:     INCLUDES := $(INCLUDES) $(displacementShader_EXTRA_INCLUDES)

$(displacementShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(displacementShader_EXTRA_LFLAGS) 
$(displacementShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(displacementShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_displacementShader clean_displacementShader Clean_displacementShader


$(displacementShader_PLUGIN): $(displacementShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_displacementShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(displacementShader_SOURCES)

clean_displacementShader:
	-rm -f $(displacementShader_OBJECTS)

Clean_displacementShader:
	-rm -f $(displacementShader_MAKEFILE).bak $(displacementShader_OBJECTS) $(displacementShader_PLUGIN)


plugins: $(displacementShader_PLUGIN)
depend:	 depend_displacementShader
clean:	 clean_displacementShader
Clean:	 Clean_displacementShader

