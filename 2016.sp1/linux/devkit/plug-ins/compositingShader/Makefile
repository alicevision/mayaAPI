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

SRCDIR := $(TOP)/compositingShader
DSTDIR := $(TOP)/compositingShader

compositingShader_SOURCES  := $(TOP)/compositingShader/compositingShader.cpp
compositingShader_OBJECTS  := $(TOP)/compositingShader/compositingShader.o
compositingShader_PLUGIN   := $(DSTDIR)/compositingShader.$(EXT)
compositingShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(compositingShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(compositingShader_EXTRA_CFLAGS)
$(compositingShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(compositingShader_EXTRA_C++FLAGS)
$(compositingShader_OBJECTS): INCLUDES := $(INCLUDES) $(compositingShader_EXTRA_INCLUDES)

depend_compositingShader:     INCLUDES := $(INCLUDES) $(compositingShader_EXTRA_INCLUDES)

$(compositingShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(compositingShader_EXTRA_LFLAGS) 
$(compositingShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(compositingShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_compositingShader clean_compositingShader Clean_compositingShader


$(compositingShader_PLUGIN): $(compositingShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_compositingShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(compositingShader_SOURCES)

clean_compositingShader:
	-rm -f $(compositingShader_OBJECTS)

Clean_compositingShader:
	-rm -f $(compositingShader_MAKEFILE).bak $(compositingShader_OBJECTS) $(compositingShader_PLUGIN)


plugins: $(compositingShader_PLUGIN)
depend:	 depend_compositingShader
clean:	 clean_compositingShader
Clean:	 Clean_compositingShader

