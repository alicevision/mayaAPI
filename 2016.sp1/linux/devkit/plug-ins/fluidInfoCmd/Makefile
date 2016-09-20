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

SRCDIR := $(TOP)/fluidInfoCmd
DSTDIR := $(TOP)/fluidInfoCmd

fluidInfoCmd_SOURCES  := $(TOP)/fluidInfoCmd/fluidInfoCmd.cpp
fluidInfoCmd_OBJECTS  := $(TOP)/fluidInfoCmd/fluidInfoCmd.o
fluidInfoCmd_PLUGIN   := $(DSTDIR)/fluidInfoCmd.$(EXT)
fluidInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(fluidInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(fluidInfoCmd_EXTRA_CFLAGS)
$(fluidInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(fluidInfoCmd_EXTRA_C++FLAGS)
$(fluidInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(fluidInfoCmd_EXTRA_INCLUDES)

depend_fluidInfoCmd:     INCLUDES := $(INCLUDES) $(fluidInfoCmd_EXTRA_INCLUDES)

$(fluidInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(fluidInfoCmd_EXTRA_LFLAGS) 
$(fluidInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(fluidInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_fluidInfoCmd clean_fluidInfoCmd Clean_fluidInfoCmd


$(fluidInfoCmd_PLUGIN): $(fluidInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_fluidInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(fluidInfoCmd_SOURCES)

clean_fluidInfoCmd:
	-rm -f $(fluidInfoCmd_OBJECTS)

Clean_fluidInfoCmd:
	-rm -f $(fluidInfoCmd_MAKEFILE).bak $(fluidInfoCmd_OBJECTS) $(fluidInfoCmd_PLUGIN)


plugins: $(fluidInfoCmd_PLUGIN)
depend:	 depend_fluidInfoCmd
clean:	 clean_fluidInfoCmd
Clean:	 Clean_fluidInfoCmd

