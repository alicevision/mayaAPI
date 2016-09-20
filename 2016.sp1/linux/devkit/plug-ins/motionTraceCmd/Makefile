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

SRCDIR := $(TOP)/motionTraceCmd
DSTDIR := $(TOP)/motionTraceCmd

motionTraceCmd_SOURCES  := $(TOP)/motionTraceCmd/motionTraceCmd.cpp
motionTraceCmd_OBJECTS  := $(TOP)/motionTraceCmd/motionTraceCmd.o
motionTraceCmd_PLUGIN   := $(DSTDIR)/motionTraceCmd.$(EXT)
motionTraceCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(motionTraceCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(motionTraceCmd_EXTRA_CFLAGS)
$(motionTraceCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(motionTraceCmd_EXTRA_C++FLAGS)
$(motionTraceCmd_OBJECTS): INCLUDES := $(INCLUDES) $(motionTraceCmd_EXTRA_INCLUDES)

depend_motionTraceCmd:     INCLUDES := $(INCLUDES) $(motionTraceCmd_EXTRA_INCLUDES)

$(motionTraceCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(motionTraceCmd_EXTRA_LFLAGS) 
$(motionTraceCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(motionTraceCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_motionTraceCmd clean_motionTraceCmd Clean_motionTraceCmd


$(motionTraceCmd_PLUGIN): $(motionTraceCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_motionTraceCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(motionTraceCmd_SOURCES)

clean_motionTraceCmd:
	-rm -f $(motionTraceCmd_OBJECTS)

Clean_motionTraceCmd:
	-rm -f $(motionTraceCmd_MAKEFILE).bak $(motionTraceCmd_OBJECTS) $(motionTraceCmd_PLUGIN)


plugins: $(motionTraceCmd_PLUGIN)
depend:	 depend_motionTraceCmd
clean:	 clean_motionTraceCmd
Clean:	 Clean_motionTraceCmd

