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

SRCDIR := $(TOP)/flameShader
DSTDIR := $(TOP)/flameShader

flameShader_SOURCES  := $(TOP)/flameShader/flameShader.cpp
flameShader_OBJECTS  := $(TOP)/flameShader/flameShader.o
flameShader_PLUGIN   := $(DSTDIR)/flameShader.$(EXT)
flameShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(flameShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(flameShader_EXTRA_CFLAGS)
$(flameShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(flameShader_EXTRA_C++FLAGS)
$(flameShader_OBJECTS): INCLUDES := $(INCLUDES) $(flameShader_EXTRA_INCLUDES)

depend_flameShader:     INCLUDES := $(INCLUDES) $(flameShader_EXTRA_INCLUDES)

$(flameShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(flameShader_EXTRA_LFLAGS) 
$(flameShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(flameShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_flameShader clean_flameShader Clean_flameShader


$(flameShader_PLUGIN): $(flameShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_flameShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(flameShader_SOURCES)

clean_flameShader:
	-rm -f $(flameShader_OBJECTS)

Clean_flameShader:
	-rm -f $(flameShader_MAKEFILE).bak $(flameShader_OBJECTS) $(flameShader_PLUGIN)


plugins: $(flameShader_PLUGIN)
depend:	 depend_flameShader
clean:	 clean_flameShader
Clean:	 Clean_flameShader

