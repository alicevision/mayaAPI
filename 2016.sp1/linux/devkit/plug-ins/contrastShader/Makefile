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

SRCDIR := $(TOP)/contrastShader
DSTDIR := $(TOP)/contrastShader

contrastShader_SOURCES  := $(TOP)/contrastShader/contrastShader.cpp
contrastShader_OBJECTS  := $(TOP)/contrastShader/contrastShader.o
contrastShader_PLUGIN   := $(DSTDIR)/contrastShader.$(EXT)
contrastShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(contrastShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(contrastShader_EXTRA_CFLAGS)
$(contrastShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(contrastShader_EXTRA_C++FLAGS)
$(contrastShader_OBJECTS): INCLUDES := $(INCLUDES) $(contrastShader_EXTRA_INCLUDES)

depend_contrastShader:     INCLUDES := $(INCLUDES) $(contrastShader_EXTRA_INCLUDES)

$(contrastShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(contrastShader_EXTRA_LFLAGS) 
$(contrastShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(contrastShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_contrastShader clean_contrastShader Clean_contrastShader


$(contrastShader_PLUGIN): $(contrastShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_contrastShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(contrastShader_SOURCES)

clean_contrastShader:
	-rm -f $(contrastShader_OBJECTS)

Clean_contrastShader:
	-rm -f $(contrastShader_MAKEFILE).bak $(contrastShader_OBJECTS) $(contrastShader_PLUGIN)


plugins: $(contrastShader_PLUGIN)
depend:	 depend_contrastShader
clean:	 clean_contrastShader
Clean:	 Clean_contrastShader

