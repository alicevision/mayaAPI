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

SRCDIR := $(TOP)/polyMessageCmd
DSTDIR := $(TOP)/polyMessageCmd

polyMessageCmd_SOURCES  := $(TOP)/polyMessageCmd/polyMessageCmd.cpp
polyMessageCmd_OBJECTS  := $(TOP)/polyMessageCmd/polyMessageCmd.o
polyMessageCmd_PLUGIN   := $(DSTDIR)/polyMessageCmd.$(EXT)
polyMessageCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(polyMessageCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(polyMessageCmd_EXTRA_CFLAGS)
$(polyMessageCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(polyMessageCmd_EXTRA_C++FLAGS)
$(polyMessageCmd_OBJECTS): INCLUDES := $(INCLUDES) $(polyMessageCmd_EXTRA_INCLUDES)

depend_polyMessageCmd:     INCLUDES := $(INCLUDES) $(polyMessageCmd_EXTRA_INCLUDES)

$(polyMessageCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(polyMessageCmd_EXTRA_LFLAGS) 
$(polyMessageCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(polyMessageCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_polyMessageCmd clean_polyMessageCmd Clean_polyMessageCmd


$(polyMessageCmd_PLUGIN): $(polyMessageCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_polyMessageCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(polyMessageCmd_SOURCES)

clean_polyMessageCmd:
	-rm -f $(polyMessageCmd_OBJECTS)

Clean_polyMessageCmd:
	-rm -f $(polyMessageCmd_MAKEFILE).bak $(polyMessageCmd_OBJECTS) $(polyMessageCmd_PLUGIN)


plugins: $(polyMessageCmd_PLUGIN)
depend:	 depend_polyMessageCmd
clean:	 clean_polyMessageCmd
Clean:	 Clean_polyMessageCmd

