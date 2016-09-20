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

SRCDIR := $(TOP)/simpleFluidEmitter
DSTDIR := $(TOP)/simpleFluidEmitter

simpleFluidEmitter_SOURCES  := $(TOP)/simpleFluidEmitter/simpleFluidEmitter.cpp
simpleFluidEmitter_OBJECTS  := $(TOP)/simpleFluidEmitter/simpleFluidEmitter.o
simpleFluidEmitter_PLUGIN   := $(DSTDIR)/simpleFluidEmitter.$(EXT)
simpleFluidEmitter_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleFluidEmitter_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleFluidEmitter_EXTRA_CFLAGS)
$(simpleFluidEmitter_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleFluidEmitter_EXTRA_C++FLAGS)
$(simpleFluidEmitter_OBJECTS): INCLUDES := $(INCLUDES) $(simpleFluidEmitter_EXTRA_INCLUDES)

depend_simpleFluidEmitter:     INCLUDES := $(INCLUDES) $(simpleFluidEmitter_EXTRA_INCLUDES)

$(simpleFluidEmitter_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleFluidEmitter_EXTRA_LFLAGS) 
$(simpleFluidEmitter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(simpleFluidEmitter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleFluidEmitter clean_simpleFluidEmitter Clean_simpleFluidEmitter


$(simpleFluidEmitter_PLUGIN): $(simpleFluidEmitter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleFluidEmitter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleFluidEmitter_SOURCES)

clean_simpleFluidEmitter:
	-rm -f $(simpleFluidEmitter_OBJECTS)

Clean_simpleFluidEmitter:
	-rm -f $(simpleFluidEmitter_MAKEFILE).bak $(simpleFluidEmitter_OBJECTS) $(simpleFluidEmitter_PLUGIN)


plugins: $(simpleFluidEmitter_PLUGIN)
depend:	 depend_simpleFluidEmitter
clean:	 clean_simpleFluidEmitter
Clean:	 Clean_simpleFluidEmitter

