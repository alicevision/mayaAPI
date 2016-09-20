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

SRCDIR := $(TOP)/moveCurveCVsCmd
DSTDIR := $(TOP)/moveCurveCVsCmd

moveCurveCVsCmd_SOURCES  := $(TOP)/moveCurveCVsCmd/moveCurveCVsCmd.cpp
moveCurveCVsCmd_OBJECTS  := $(TOP)/moveCurveCVsCmd/moveCurveCVsCmd.o
moveCurveCVsCmd_PLUGIN   := $(DSTDIR)/moveCurveCVsCmd.$(EXT)
moveCurveCVsCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(moveCurveCVsCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(moveCurveCVsCmd_EXTRA_CFLAGS)
$(moveCurveCVsCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(moveCurveCVsCmd_EXTRA_C++FLAGS)
$(moveCurveCVsCmd_OBJECTS): INCLUDES := $(INCLUDES) $(moveCurveCVsCmd_EXTRA_INCLUDES)

depend_moveCurveCVsCmd:     INCLUDES := $(INCLUDES) $(moveCurveCVsCmd_EXTRA_INCLUDES)

$(moveCurveCVsCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(moveCurveCVsCmd_EXTRA_LFLAGS) 
$(moveCurveCVsCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(moveCurveCVsCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_moveCurveCVsCmd clean_moveCurveCVsCmd Clean_moveCurveCVsCmd


$(moveCurveCVsCmd_PLUGIN): $(moveCurveCVsCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_moveCurveCVsCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(moveCurveCVsCmd_SOURCES)

clean_moveCurveCVsCmd:
	-rm -f $(moveCurveCVsCmd_OBJECTS)

Clean_moveCurveCVsCmd:
	-rm -f $(moveCurveCVsCmd_MAKEFILE).bak $(moveCurveCVsCmd_OBJECTS) $(moveCurveCVsCmd_PLUGIN)


plugins: $(moveCurveCVsCmd_PLUGIN)
depend:	 depend_moveCurveCVsCmd
clean:	 clean_moveCurveCVsCmd
Clean:	 Clean_moveCurveCVsCmd

