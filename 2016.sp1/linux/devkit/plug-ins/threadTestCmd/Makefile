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

SRCDIR := $(TOP)/threadTestCmd
DSTDIR := $(TOP)/threadTestCmd

threadTestCmd_SOURCES  := $(TOP)/threadTestCmd/threadTestCmd.cpp
threadTestCmd_OBJECTS  := $(TOP)/threadTestCmd/threadTestCmd.o
threadTestCmd_PLUGIN   := $(DSTDIR)/threadTestCmd.$(EXT)
threadTestCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(threadTestCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(threadTestCmd_EXTRA_CFLAGS)
$(threadTestCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(threadTestCmd_EXTRA_C++FLAGS)
$(threadTestCmd_OBJECTS): INCLUDES := $(INCLUDES) $(threadTestCmd_EXTRA_INCLUDES)

depend_threadTestCmd:     INCLUDES := $(INCLUDES) $(threadTestCmd_EXTRA_INCLUDES)

$(threadTestCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(threadTestCmd_EXTRA_LFLAGS) 
$(threadTestCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(threadTestCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_threadTestCmd clean_threadTestCmd Clean_threadTestCmd


$(threadTestCmd_PLUGIN): $(threadTestCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_threadTestCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(threadTestCmd_SOURCES)

clean_threadTestCmd:
	-rm -f $(threadTestCmd_OBJECTS)

Clean_threadTestCmd:
	-rm -f $(threadTestCmd_MAKEFILE).bak $(threadTestCmd_OBJECTS) $(threadTestCmd_PLUGIN)


plugins: $(threadTestCmd_PLUGIN)
depend:	 depend_threadTestCmd
clean:	 clean_threadTestCmd
Clean:	 Clean_threadTestCmd

