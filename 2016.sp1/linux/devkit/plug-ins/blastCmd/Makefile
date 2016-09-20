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

SRCDIR := $(TOP)/blastCmd
DSTDIR := $(TOP)/blastCmd

blastCmd_SOURCES  := $(TOP)/blastCmd/blastCmd.cpp
blastCmd_OBJECTS  := $(TOP)/blastCmd/blastCmd.o
blastCmd_PLUGIN   := $(DSTDIR)/blastCmd.$(EXT)
blastCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(blastCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(blastCmd_EXTRA_CFLAGS)
$(blastCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(blastCmd_EXTRA_C++FLAGS)
$(blastCmd_OBJECTS): INCLUDES := $(INCLUDES) $(blastCmd_EXTRA_INCLUDES)

depend_blastCmd:     INCLUDES := $(INCLUDES) $(blastCmd_EXTRA_INCLUDES)

$(blastCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(blastCmd_EXTRA_LFLAGS) 
$(blastCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaAnim -lFoundation -lGL $(blastCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_blastCmd clean_blastCmd Clean_blastCmd


$(blastCmd_PLUGIN): $(blastCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_blastCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(blastCmd_SOURCES)

clean_blastCmd:
	-rm -f $(blastCmd_OBJECTS)

Clean_blastCmd:
	-rm -f $(blastCmd_MAKEFILE).bak $(blastCmd_OBJECTS) $(blastCmd_PLUGIN)


plugins: $(blastCmd_PLUGIN)
depend:	 depend_blastCmd
clean:	 clean_blastCmd
Clean:	 Clean_blastCmd

