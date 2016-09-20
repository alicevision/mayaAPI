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

SRCDIR := $(TOP)/threadTestWithLocksCmd
DSTDIR := $(TOP)/threadTestWithLocksCmd

threadTestWithLocksCmd_SOURCES  := $(TOP)/threadTestWithLocksCmd/threadTestWithLocksCmd.cpp
threadTestWithLocksCmd_OBJECTS  := $(TOP)/threadTestWithLocksCmd/threadTestWithLocksCmd.o
threadTestWithLocksCmd_PLUGIN   := $(DSTDIR)/threadTestWithLocksCmd.$(EXT)
threadTestWithLocksCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(threadTestWithLocksCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(threadTestWithLocksCmd_EXTRA_CFLAGS)
$(threadTestWithLocksCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(threadTestWithLocksCmd_EXTRA_C++FLAGS)
$(threadTestWithLocksCmd_OBJECTS): INCLUDES := $(INCLUDES) $(threadTestWithLocksCmd_EXTRA_INCLUDES)

depend_threadTestWithLocksCmd:     INCLUDES := $(INCLUDES) $(threadTestWithLocksCmd_EXTRA_INCLUDES)

$(threadTestWithLocksCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(threadTestWithLocksCmd_EXTRA_LFLAGS) 
$(threadTestWithLocksCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(threadTestWithLocksCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_threadTestWithLocksCmd clean_threadTestWithLocksCmd Clean_threadTestWithLocksCmd


$(threadTestWithLocksCmd_PLUGIN): $(threadTestWithLocksCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_threadTestWithLocksCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(threadTestWithLocksCmd_SOURCES)

clean_threadTestWithLocksCmd:
	-rm -f $(threadTestWithLocksCmd_OBJECTS)

Clean_threadTestWithLocksCmd:
	-rm -f $(threadTestWithLocksCmd_MAKEFILE).bak $(threadTestWithLocksCmd_OBJECTS) $(threadTestWithLocksCmd_PLUGIN)


plugins: $(threadTestWithLocksCmd_PLUGIN)
depend:	 depend_threadTestWithLocksCmd
clean:	 clean_threadTestWithLocksCmd
Clean:	 Clean_threadTestWithLocksCmd

