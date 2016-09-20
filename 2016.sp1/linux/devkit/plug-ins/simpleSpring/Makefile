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

SRCDIR := $(TOP)/simpleSpring
DSTDIR := $(TOP)/simpleSpring

simpleSpring_SOURCES  := $(TOP)/simpleSpring/simpleSpring.cpp
simpleSpring_OBJECTS  := $(TOP)/simpleSpring/simpleSpring.o
simpleSpring_PLUGIN   := $(DSTDIR)/simpleSpring.$(EXT)
simpleSpring_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleSpring_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleSpring_EXTRA_CFLAGS)
$(simpleSpring_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleSpring_EXTRA_C++FLAGS)
$(simpleSpring_OBJECTS): INCLUDES := $(INCLUDES) $(simpleSpring_EXTRA_INCLUDES)

depend_simpleSpring:     INCLUDES := $(INCLUDES) $(simpleSpring_EXTRA_INCLUDES)

$(simpleSpring_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleSpring_EXTRA_LFLAGS) 
$(simpleSpring_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(simpleSpring_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleSpring clean_simpleSpring Clean_simpleSpring


$(simpleSpring_PLUGIN): $(simpleSpring_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleSpring :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleSpring_SOURCES)

clean_simpleSpring:
	-rm -f $(simpleSpring_OBJECTS)

Clean_simpleSpring:
	-rm -f $(simpleSpring_MAKEFILE).bak $(simpleSpring_OBJECTS) $(simpleSpring_PLUGIN)


plugins: $(simpleSpring_PLUGIN)
depend:	 depend_simpleSpring
clean:	 clean_simpleSpring
Clean:	 Clean_simpleSpring

