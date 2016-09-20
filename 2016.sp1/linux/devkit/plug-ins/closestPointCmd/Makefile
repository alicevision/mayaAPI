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

SRCDIR := $(TOP)/closestPointCmd
DSTDIR := $(TOP)/closestPointCmd

closestPointCmd_SOURCES  := $(TOP)/closestPointCmd/closestPointCmd.cpp
closestPointCmd_OBJECTS  := $(TOP)/closestPointCmd/closestPointCmd.o
closestPointCmd_PLUGIN   := $(DSTDIR)/closestPointCmd.$(EXT)
closestPointCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(closestPointCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(closestPointCmd_EXTRA_CFLAGS)
$(closestPointCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(closestPointCmd_EXTRA_C++FLAGS)
$(closestPointCmd_OBJECTS): INCLUDES := $(INCLUDES) $(closestPointCmd_EXTRA_INCLUDES)

depend_closestPointCmd:     INCLUDES := $(INCLUDES) $(closestPointCmd_EXTRA_INCLUDES)

$(closestPointCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(closestPointCmd_EXTRA_LFLAGS) 
$(closestPointCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(closestPointCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_closestPointCmd clean_closestPointCmd Clean_closestPointCmd


$(closestPointCmd_PLUGIN): $(closestPointCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_closestPointCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(closestPointCmd_SOURCES)

clean_closestPointCmd:
	-rm -f $(closestPointCmd_OBJECTS)

Clean_closestPointCmd:
	-rm -f $(closestPointCmd_MAKEFILE).bak $(closestPointCmd_OBJECTS) $(closestPointCmd_PLUGIN)


plugins: $(closestPointCmd_PLUGIN)
depend:	 depend_closestPointCmd
clean:	 clean_closestPointCmd
Clean:	 Clean_closestPointCmd

