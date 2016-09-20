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

SRCDIR := $(TOP)/anisotropicShader
DSTDIR := $(TOP)/anisotropicShader

anisotropicShader_SOURCES  := $(TOP)/anisotropicShader/anisotropicShader.cpp
anisotropicShader_OBJECTS  := $(TOP)/anisotropicShader/anisotropicShader.o
anisotropicShader_PLUGIN   := $(DSTDIR)/anisotropicShader.$(EXT)
anisotropicShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(anisotropicShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(anisotropicShader_EXTRA_CFLAGS)
$(anisotropicShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(anisotropicShader_EXTRA_C++FLAGS)
$(anisotropicShader_OBJECTS): INCLUDES := $(INCLUDES) $(anisotropicShader_EXTRA_INCLUDES)

depend_anisotropicShader:     INCLUDES := $(INCLUDES) $(anisotropicShader_EXTRA_INCLUDES)

$(anisotropicShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(anisotropicShader_EXTRA_LFLAGS) 
$(anisotropicShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(anisotropicShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_anisotropicShader clean_anisotropicShader Clean_anisotropicShader


$(anisotropicShader_PLUGIN): $(anisotropicShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_anisotropicShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(anisotropicShader_SOURCES)

clean_anisotropicShader:
	-rm -f $(anisotropicShader_OBJECTS)

Clean_anisotropicShader:
	-rm -f $(anisotropicShader_MAKEFILE).bak $(anisotropicShader_OBJECTS) $(anisotropicShader_PLUGIN)


plugins: $(anisotropicShader_PLUGIN)
depend:	 depend_anisotropicShader
clean:	 clean_anisotropicShader
Clean:	 Clean_anisotropicShader

