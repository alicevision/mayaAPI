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

SRCDIR := $(TOP)/volumeLightCmd
DSTDIR := $(TOP)/volumeLightCmd

volumeLightCmd_SOURCES  := $(TOP)/volumeLightCmd/volumeLightCmd.cpp
volumeLightCmd_OBJECTS  := $(TOP)/volumeLightCmd/volumeLightCmd.o
volumeLightCmd_PLUGIN   := $(DSTDIR)/volumeLightCmd.$(EXT)
volumeLightCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(volumeLightCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(volumeLightCmd_EXTRA_CFLAGS)
$(volumeLightCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(volumeLightCmd_EXTRA_C++FLAGS)
$(volumeLightCmd_OBJECTS): INCLUDES := $(INCLUDES) $(volumeLightCmd_EXTRA_INCLUDES)

depend_volumeLightCmd:     INCLUDES := $(INCLUDES) $(volumeLightCmd_EXTRA_INCLUDES)

$(volumeLightCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(volumeLightCmd_EXTRA_LFLAGS) 
$(volumeLightCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(volumeLightCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_volumeLightCmd clean_volumeLightCmd Clean_volumeLightCmd


$(volumeLightCmd_PLUGIN): $(volumeLightCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_volumeLightCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(volumeLightCmd_SOURCES)

clean_volumeLightCmd:
	-rm -f $(volumeLightCmd_OBJECTS)

Clean_volumeLightCmd:
	-rm -f $(volumeLightCmd_MAKEFILE).bak $(volumeLightCmd_OBJECTS) $(volumeLightCmd_PLUGIN)


plugins: $(volumeLightCmd_PLUGIN)
depend:	 depend_volumeLightCmd
clean:	 clean_volumeLightCmd
Clean:	 Clean_volumeLightCmd

