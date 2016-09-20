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

SRCDIR := $(TOP)/animInfoCmd
DSTDIR := $(TOP)/animInfoCmd

animInfoCmd_SOURCES  := $(TOP)/animInfoCmd/animInfoCmd.cpp
animInfoCmd_OBJECTS  := $(TOP)/animInfoCmd/animInfoCmd.o
animInfoCmd_PLUGIN   := $(DSTDIR)/animInfoCmd.$(EXT)
animInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(animInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(animInfoCmd_EXTRA_CFLAGS)
$(animInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(animInfoCmd_EXTRA_C++FLAGS)
$(animInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(animInfoCmd_EXTRA_INCLUDES)

depend_animInfoCmd:     INCLUDES := $(INCLUDES) $(animInfoCmd_EXTRA_INCLUDES)

$(animInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(animInfoCmd_EXTRA_LFLAGS) 
$(animInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(animInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_animInfoCmd clean_animInfoCmd Clean_animInfoCmd


$(animInfoCmd_PLUGIN): $(animInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_animInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(animInfoCmd_SOURCES)

clean_animInfoCmd:
	-rm -f $(animInfoCmd_OBJECTS)

Clean_animInfoCmd:
	-rm -f $(animInfoCmd_MAKEFILE).bak $(animInfoCmd_OBJECTS) $(animInfoCmd_PLUGIN)


plugins: $(animInfoCmd_PLUGIN)
depend:	 depend_animInfoCmd
clean:	 clean_animInfoCmd
Clean:	 Clean_animInfoCmd

