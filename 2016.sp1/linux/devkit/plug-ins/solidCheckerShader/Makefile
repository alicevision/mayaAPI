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

SRCDIR := $(TOP)/solidCheckerShader
DSTDIR := $(TOP)/solidCheckerShader

solidCheckerShader_SOURCES  := $(TOP)/solidCheckerShader/solidCheckerShader.cpp
solidCheckerShader_OBJECTS  := $(TOP)/solidCheckerShader/solidCheckerShader.o
solidCheckerShader_PLUGIN   := $(DSTDIR)/solidCheckerShader.$(EXT)
solidCheckerShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(solidCheckerShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(solidCheckerShader_EXTRA_CFLAGS)
$(solidCheckerShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(solidCheckerShader_EXTRA_C++FLAGS)
$(solidCheckerShader_OBJECTS): INCLUDES := $(INCLUDES) $(solidCheckerShader_EXTRA_INCLUDES)

depend_solidCheckerShader:     INCLUDES := $(INCLUDES) $(solidCheckerShader_EXTRA_INCLUDES)

$(solidCheckerShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(solidCheckerShader_EXTRA_LFLAGS) 
$(solidCheckerShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(solidCheckerShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_solidCheckerShader clean_solidCheckerShader Clean_solidCheckerShader


$(solidCheckerShader_PLUGIN): $(solidCheckerShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_solidCheckerShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(solidCheckerShader_SOURCES)

clean_solidCheckerShader:
	-rm -f $(solidCheckerShader_OBJECTS)

Clean_solidCheckerShader:
	-rm -f $(solidCheckerShader_MAKEFILE).bak $(solidCheckerShader_OBJECTS) $(solidCheckerShader_PLUGIN)


plugins: $(solidCheckerShader_PLUGIN)
depend:	 depend_solidCheckerShader
clean:	 clean_solidCheckerShader
Clean:	 Clean_solidCheckerShader

