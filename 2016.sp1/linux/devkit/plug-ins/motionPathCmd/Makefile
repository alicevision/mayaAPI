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

SRCDIR := $(TOP)/motionPathCmd
DSTDIR := $(TOP)/motionPathCmd

motionPathCmd_SOURCES  := $(TOP)/motionPathCmd/motionPathCmd.cpp
motionPathCmd_OBJECTS  := $(TOP)/motionPathCmd/motionPathCmd.o
motionPathCmd_PLUGIN   := $(DSTDIR)/motionPathCmd.$(EXT)
motionPathCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(motionPathCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(motionPathCmd_EXTRA_CFLAGS)
$(motionPathCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(motionPathCmd_EXTRA_C++FLAGS)
$(motionPathCmd_OBJECTS): INCLUDES := $(INCLUDES) $(motionPathCmd_EXTRA_INCLUDES)

depend_motionPathCmd:     INCLUDES := $(INCLUDES) $(motionPathCmd_EXTRA_INCLUDES)

$(motionPathCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(motionPathCmd_EXTRA_LFLAGS) 
$(motionPathCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(motionPathCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_motionPathCmd clean_motionPathCmd Clean_motionPathCmd


$(motionPathCmd_PLUGIN): $(motionPathCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_motionPathCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(motionPathCmd_SOURCES)

clean_motionPathCmd:
	-rm -f $(motionPathCmd_OBJECTS)

Clean_motionPathCmd:
	-rm -f $(motionPathCmd_MAKEFILE).bak $(motionPathCmd_OBJECTS) $(motionPathCmd_PLUGIN)


plugins: $(motionPathCmd_PLUGIN)
depend:	 depend_motionPathCmd
clean:	 clean_motionPathCmd
Clean:	 Clean_motionPathCmd

