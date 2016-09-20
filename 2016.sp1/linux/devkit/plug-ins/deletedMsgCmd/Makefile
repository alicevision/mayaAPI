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

SRCDIR := $(TOP)/deletedMsgCmd
DSTDIR := $(TOP)/deletedMsgCmd

deletedMsgCmd_SOURCES  := $(TOP)/deletedMsgCmd/deletedMsgCmd.cpp
deletedMsgCmd_OBJECTS  := $(TOP)/deletedMsgCmd/deletedMsgCmd.o
deletedMsgCmd_PLUGIN   := $(DSTDIR)/deletedMsgCmd.$(EXT)
deletedMsgCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(deletedMsgCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(deletedMsgCmd_EXTRA_CFLAGS)
$(deletedMsgCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(deletedMsgCmd_EXTRA_C++FLAGS)
$(deletedMsgCmd_OBJECTS): INCLUDES := $(INCLUDES) $(deletedMsgCmd_EXTRA_INCLUDES)

depend_deletedMsgCmd:     INCLUDES := $(INCLUDES) $(deletedMsgCmd_EXTRA_INCLUDES)

$(deletedMsgCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(deletedMsgCmd_EXTRA_LFLAGS) 
$(deletedMsgCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(deletedMsgCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_deletedMsgCmd clean_deletedMsgCmd Clean_deletedMsgCmd


$(deletedMsgCmd_PLUGIN): $(deletedMsgCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_deletedMsgCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(deletedMsgCmd_SOURCES)

clean_deletedMsgCmd:
	-rm -f $(deletedMsgCmd_OBJECTS)

Clean_deletedMsgCmd:
	-rm -f $(deletedMsgCmd_MAKEFILE).bak $(deletedMsgCmd_OBJECTS) $(deletedMsgCmd_PLUGIN)


plugins: $(deletedMsgCmd_PLUGIN)
depend:	 depend_deletedMsgCmd
clean:	 clean_deletedMsgCmd
Clean:	 Clean_deletedMsgCmd

