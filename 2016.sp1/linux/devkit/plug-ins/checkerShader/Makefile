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

SRCDIR := $(TOP)/checkerShader
DSTDIR := $(TOP)/checkerShader

checkerShader_SOURCES  := $(TOP)/checkerShader/checkerShader.cpp
checkerShader_OBJECTS  := $(TOP)/checkerShader/checkerShader.o
checkerShader_PLUGIN   := $(DSTDIR)/checkerShader.$(EXT)
checkerShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(checkerShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(checkerShader_EXTRA_CFLAGS)
$(checkerShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(checkerShader_EXTRA_C++FLAGS)
$(checkerShader_OBJECTS): INCLUDES := $(INCLUDES) $(checkerShader_EXTRA_INCLUDES)

depend_checkerShader:     INCLUDES := $(INCLUDES) $(checkerShader_EXTRA_INCLUDES)

$(checkerShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(checkerShader_EXTRA_LFLAGS) 
$(checkerShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(checkerShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_checkerShader clean_checkerShader Clean_checkerShader


$(checkerShader_PLUGIN): $(checkerShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_checkerShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(checkerShader_SOURCES)

clean_checkerShader:
	-rm -f $(checkerShader_OBJECTS)

Clean_checkerShader:
	-rm -f $(checkerShader_MAKEFILE).bak $(checkerShader_OBJECTS) $(checkerShader_PLUGIN)


plugins: $(checkerShader_PLUGIN)
depend:	 depend_checkerShader
clean:	 clean_checkerShader
Clean:	 Clean_checkerShader

