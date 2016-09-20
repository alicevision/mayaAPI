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

SRCDIR := $(TOP)/ownerEmitter
DSTDIR := $(TOP)/ownerEmitter

ownerEmitter_SOURCES  := $(TOP)/ownerEmitter/ownerEmitter.cpp
ownerEmitter_OBJECTS  := $(TOP)/ownerEmitter/ownerEmitter.o
ownerEmitter_PLUGIN   := $(DSTDIR)/ownerEmitter.$(EXT)
ownerEmitter_MAKEFILE := $(DSTDIR)/Makefile

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

$(ownerEmitter_OBJECTS): CFLAGS   := $(CFLAGS)   $(ownerEmitter_EXTRA_CFLAGS)
$(ownerEmitter_OBJECTS): C++FLAGS := $(C++FLAGS) $(ownerEmitter_EXTRA_C++FLAGS)
$(ownerEmitter_OBJECTS): INCLUDES := $(INCLUDES) $(ownerEmitter_EXTRA_INCLUDES)

depend_ownerEmitter:     INCLUDES := $(INCLUDES) $(ownerEmitter_EXTRA_INCLUDES)

$(ownerEmitter_PLUGIN):  LFLAGS   := $(LFLAGS) $(ownerEmitter_EXTRA_LFLAGS) 
$(ownerEmitter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(ownerEmitter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_ownerEmitter clean_ownerEmitter Clean_ownerEmitter


$(ownerEmitter_PLUGIN): $(ownerEmitter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_ownerEmitter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(ownerEmitter_SOURCES)

clean_ownerEmitter:
	-rm -f $(ownerEmitter_OBJECTS)

Clean_ownerEmitter:
	-rm -f $(ownerEmitter_MAKEFILE).bak $(ownerEmitter_OBJECTS) $(ownerEmitter_PLUGIN)


plugins: $(ownerEmitter_PLUGIN)
depend:	 depend_ownerEmitter
clean:	 clean_ownerEmitter
Clean:	 Clean_ownerEmitter

