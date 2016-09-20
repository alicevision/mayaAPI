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

SRCDIR := $(TOP)/pluginCallbacks
DSTDIR := $(TOP)/pluginCallbacks

pluginCallbacks_SOURCES  := $(TOP)/pluginCallbacks/pluginCallbacks.cpp
pluginCallbacks_OBJECTS  := $(TOP)/pluginCallbacks/pluginCallbacks.o
pluginCallbacks_PLUGIN   := $(DSTDIR)/pluginCallbacks.$(EXT)
pluginCallbacks_MAKEFILE := $(DSTDIR)/Makefile

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

$(pluginCallbacks_OBJECTS): CFLAGS   := $(CFLAGS)   $(pluginCallbacks_EXTRA_CFLAGS)
$(pluginCallbacks_OBJECTS): C++FLAGS := $(C++FLAGS) $(pluginCallbacks_EXTRA_C++FLAGS)
$(pluginCallbacks_OBJECTS): INCLUDES := $(INCLUDES) $(pluginCallbacks_EXTRA_INCLUDES)

depend_pluginCallbacks:     INCLUDES := $(INCLUDES) $(pluginCallbacks_EXTRA_INCLUDES)

$(pluginCallbacks_PLUGIN):  LFLAGS   := $(LFLAGS) $(pluginCallbacks_EXTRA_LFLAGS) 
$(pluginCallbacks_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(pluginCallbacks_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_pluginCallbacks clean_pluginCallbacks Clean_pluginCallbacks


$(pluginCallbacks_PLUGIN): $(pluginCallbacks_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_pluginCallbacks :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(pluginCallbacks_SOURCES)

clean_pluginCallbacks:
	-rm -f $(pluginCallbacks_OBJECTS)

Clean_pluginCallbacks:
	-rm -f $(pluginCallbacks_MAKEFILE).bak $(pluginCallbacks_OBJECTS) $(pluginCallbacks_PLUGIN)


plugins: $(pluginCallbacks_PLUGIN)
depend:	 depend_pluginCallbacks
clean:	 clean_pluginCallbacks
Clean:	 Clean_pluginCallbacks

