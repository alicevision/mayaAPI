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

SRCDIR := $(TOP)/particleSystemInfoCmd
DSTDIR := $(TOP)/particleSystemInfoCmd

particleSystemInfoCmd_SOURCES  := $(TOP)/particleSystemInfoCmd/particleSystemInfoCmd.cpp
particleSystemInfoCmd_OBJECTS  := $(TOP)/particleSystemInfoCmd/particleSystemInfoCmd.o
particleSystemInfoCmd_PLUGIN   := $(DSTDIR)/particleSystemInfoCmd.$(EXT)
particleSystemInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(particleSystemInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(particleSystemInfoCmd_EXTRA_CFLAGS)
$(particleSystemInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(particleSystemInfoCmd_EXTRA_C++FLAGS)
$(particleSystemInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(particleSystemInfoCmd_EXTRA_INCLUDES)

depend_particleSystemInfoCmd:     INCLUDES := $(INCLUDES) $(particleSystemInfoCmd_EXTRA_INCLUDES)

$(particleSystemInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(particleSystemInfoCmd_EXTRA_LFLAGS) 
$(particleSystemInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(particleSystemInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_particleSystemInfoCmd clean_particleSystemInfoCmd Clean_particleSystemInfoCmd


$(particleSystemInfoCmd_PLUGIN): $(particleSystemInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_particleSystemInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(particleSystemInfoCmd_SOURCES)

clean_particleSystemInfoCmd:
	-rm -f $(particleSystemInfoCmd_OBJECTS)

Clean_particleSystemInfoCmd:
	-rm -f $(particleSystemInfoCmd_MAKEFILE).bak $(particleSystemInfoCmd_OBJECTS) $(particleSystemInfoCmd_PLUGIN)


plugins: $(particleSystemInfoCmd_PLUGIN)
depend:	 depend_particleSystemInfoCmd
clean:	 clean_particleSystemInfoCmd
Clean:	 Clean_particleSystemInfoCmd

