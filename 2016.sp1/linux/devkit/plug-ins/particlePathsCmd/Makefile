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

SRCDIR := $(TOP)/particlePathsCmd
DSTDIR := $(TOP)/particlePathsCmd

particlePathsCmd_SOURCES  := $(TOP)/particlePathsCmd/particlePathsCmd.cpp
particlePathsCmd_OBJECTS  := $(TOP)/particlePathsCmd/particlePathsCmd.o
particlePathsCmd_PLUGIN   := $(DSTDIR)/particlePathsCmd.$(EXT)
particlePathsCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(particlePathsCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(particlePathsCmd_EXTRA_CFLAGS)
$(particlePathsCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(particlePathsCmd_EXTRA_C++FLAGS)
$(particlePathsCmd_OBJECTS): INCLUDES := $(INCLUDES) $(particlePathsCmd_EXTRA_INCLUDES)

depend_particlePathsCmd:     INCLUDES := $(INCLUDES) $(particlePathsCmd_EXTRA_INCLUDES)

$(particlePathsCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(particlePathsCmd_EXTRA_LFLAGS) 
$(particlePathsCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lFoundation $(particlePathsCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_particlePathsCmd clean_particlePathsCmd Clean_particlePathsCmd


$(particlePathsCmd_PLUGIN): $(particlePathsCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_particlePathsCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(particlePathsCmd_SOURCES)

clean_particlePathsCmd:
	-rm -f $(particlePathsCmd_OBJECTS)

Clean_particlePathsCmd:
	-rm -f $(particlePathsCmd_MAKEFILE).bak $(particlePathsCmd_OBJECTS) $(particlePathsCmd_PLUGIN)


plugins: $(particlePathsCmd_PLUGIN)
depend:	 depend_particlePathsCmd
clean:	 clean_particlePathsCmd
Clean:	 Clean_particlePathsCmd

