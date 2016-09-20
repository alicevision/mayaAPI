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

SRCDIR := $(TOP)/pfxInfoCmd
DSTDIR := $(TOP)/pfxInfoCmd

pfxInfoCmd_SOURCES  := $(TOP)/pfxInfoCmd/pfxInfoCmd.cpp
pfxInfoCmd_OBJECTS  := $(TOP)/pfxInfoCmd/pfxInfoCmd.o
pfxInfoCmd_PLUGIN   := $(DSTDIR)/pfxInfoCmd.$(EXT)
pfxInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(pfxInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(pfxInfoCmd_EXTRA_CFLAGS)
$(pfxInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(pfxInfoCmd_EXTRA_C++FLAGS)
$(pfxInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(pfxInfoCmd_EXTRA_INCLUDES)

depend_pfxInfoCmd:     INCLUDES := $(INCLUDES) $(pfxInfoCmd_EXTRA_INCLUDES)

$(pfxInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(pfxInfoCmd_EXTRA_LFLAGS) 
$(pfxInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(pfxInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_pfxInfoCmd clean_pfxInfoCmd Clean_pfxInfoCmd


$(pfxInfoCmd_PLUGIN): $(pfxInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_pfxInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(pfxInfoCmd_SOURCES)

clean_pfxInfoCmd:
	-rm -f $(pfxInfoCmd_OBJECTS)

Clean_pfxInfoCmd:
	-rm -f $(pfxInfoCmd_MAKEFILE).bak $(pfxInfoCmd_OBJECTS) $(pfxInfoCmd_PLUGIN)


plugins: $(pfxInfoCmd_PLUGIN)
depend:	 depend_pfxInfoCmd
clean:	 clean_pfxInfoCmd
Clean:	 Clean_pfxInfoCmd

