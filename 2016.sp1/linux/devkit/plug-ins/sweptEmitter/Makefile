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

SRCDIR := $(TOP)/sweptEmitter
DSTDIR := $(TOP)/sweptEmitter

sweptEmitter_SOURCES  := $(TOP)/sweptEmitter/sweptEmitter.cpp
sweptEmitter_OBJECTS  := $(TOP)/sweptEmitter/sweptEmitter.o
sweptEmitter_PLUGIN   := $(DSTDIR)/sweptEmitter.$(EXT)
sweptEmitter_MAKEFILE := $(DSTDIR)/Makefile

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

$(sweptEmitter_OBJECTS): CFLAGS   := $(CFLAGS)   $(sweptEmitter_EXTRA_CFLAGS)
$(sweptEmitter_OBJECTS): C++FLAGS := $(C++FLAGS) $(sweptEmitter_EXTRA_C++FLAGS)
$(sweptEmitter_OBJECTS): INCLUDES := $(INCLUDES) $(sweptEmitter_EXTRA_INCLUDES)

depend_sweptEmitter:     INCLUDES := $(INCLUDES) $(sweptEmitter_EXTRA_INCLUDES)

$(sweptEmitter_PLUGIN):  LFLAGS   := $(LFLAGS) $(sweptEmitter_EXTRA_LFLAGS) 
$(sweptEmitter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(sweptEmitter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_sweptEmitter clean_sweptEmitter Clean_sweptEmitter


$(sweptEmitter_PLUGIN): $(sweptEmitter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_sweptEmitter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(sweptEmitter_SOURCES)

clean_sweptEmitter:
	-rm -f $(sweptEmitter_OBJECTS)

Clean_sweptEmitter:
	-rm -f $(sweptEmitter_MAKEFILE).bak $(sweptEmitter_OBJECTS) $(sweptEmitter_PLUGIN)


plugins: $(sweptEmitter_PLUGIN)
depend:	 depend_sweptEmitter
clean:	 clean_sweptEmitter
Clean:	 Clean_sweptEmitter

