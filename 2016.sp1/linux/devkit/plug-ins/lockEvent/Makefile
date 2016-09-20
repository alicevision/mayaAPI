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

SRCDIR := $(TOP)/lockEvent
DSTDIR := $(TOP)/lockEvent

lockEvent_SOURCES  := $(TOP)/lockEvent/lockEvent.cpp
lockEvent_OBJECTS  := $(TOP)/lockEvent/lockEvent.o
lockEvent_PLUGIN   := $(DSTDIR)/lockEvent.$(EXT)
lockEvent_MAKEFILE := $(DSTDIR)/Makefile

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

$(lockEvent_OBJECTS): CFLAGS   := $(CFLAGS)   $(lockEvent_EXTRA_CFLAGS)
$(lockEvent_OBJECTS): C++FLAGS := $(C++FLAGS) $(lockEvent_EXTRA_C++FLAGS)
$(lockEvent_OBJECTS): INCLUDES := $(INCLUDES) $(lockEvent_EXTRA_INCLUDES)

depend_lockEvent:     INCLUDES := $(INCLUDES) $(lockEvent_EXTRA_INCLUDES)

$(lockEvent_PLUGIN):  LFLAGS   := $(LFLAGS) $(lockEvent_EXTRA_LFLAGS) 
$(lockEvent_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(lockEvent_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lockEvent clean_lockEvent Clean_lockEvent


$(lockEvent_PLUGIN): $(lockEvent_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lockEvent :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lockEvent_SOURCES)

clean_lockEvent:
	-rm -f $(lockEvent_OBJECTS)

Clean_lockEvent:
	-rm -f $(lockEvent_MAKEFILE).bak $(lockEvent_OBJECTS) $(lockEvent_PLUGIN)


plugins: $(lockEvent_PLUGIN)
depend:	 depend_lockEvent
clean:	 clean_lockEvent
Clean:	 Clean_lockEvent

