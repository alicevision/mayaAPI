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

SRCDIR := $(TOP)/simpleEmitter
DSTDIR := $(TOP)/simpleEmitter

simpleEmitter_SOURCES  := $(TOP)/simpleEmitter/simpleEmitter.cpp
simpleEmitter_OBJECTS  := $(TOP)/simpleEmitter/simpleEmitter.o
simpleEmitter_PLUGIN   := $(DSTDIR)/simpleEmitter.$(EXT)
simpleEmitter_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleEmitter_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleEmitter_EXTRA_CFLAGS)
$(simpleEmitter_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleEmitter_EXTRA_C++FLAGS)
$(simpleEmitter_OBJECTS): INCLUDES := $(INCLUDES) $(simpleEmitter_EXTRA_INCLUDES)

depend_simpleEmitter:     INCLUDES := $(INCLUDES) $(simpleEmitter_EXTRA_INCLUDES)

$(simpleEmitter_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleEmitter_EXTRA_LFLAGS) 
$(simpleEmitter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lOpenMayaUI -lFoundation -lGL -lGLU $(simpleEmitter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleEmitter clean_simpleEmitter Clean_simpleEmitter


$(simpleEmitter_PLUGIN): $(simpleEmitter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleEmitter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleEmitter_SOURCES)

clean_simpleEmitter:
	-rm -f $(simpleEmitter_OBJECTS)

Clean_simpleEmitter:
	-rm -f $(simpleEmitter_MAKEFILE).bak $(simpleEmitter_OBJECTS) $(simpleEmitter_PLUGIN)


plugins: $(simpleEmitter_PLUGIN)
depend:	 depend_simpleEmitter
clean:	 clean_simpleEmitter
Clean:	 Clean_simpleEmitter

