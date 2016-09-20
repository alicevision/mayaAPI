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

SRCDIR := $(TOP)/findTexturesPerPolygonCmd
DSTDIR := $(TOP)/findTexturesPerPolygonCmd

findTexturesPerPolygonCmd_SOURCES  := $(TOP)/findTexturesPerPolygonCmd/findTexturesPerPolygonCmd.cpp
findTexturesPerPolygonCmd_OBJECTS  := $(TOP)/findTexturesPerPolygonCmd/findTexturesPerPolygonCmd.o
findTexturesPerPolygonCmd_PLUGIN   := $(DSTDIR)/findTexturesPerPolygonCmd.$(EXT)
findTexturesPerPolygonCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(findTexturesPerPolygonCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(findTexturesPerPolygonCmd_EXTRA_CFLAGS)
$(findTexturesPerPolygonCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(findTexturesPerPolygonCmd_EXTRA_C++FLAGS)
$(findTexturesPerPolygonCmd_OBJECTS): INCLUDES := $(INCLUDES) $(findTexturesPerPolygonCmd_EXTRA_INCLUDES)

depend_findTexturesPerPolygonCmd:     INCLUDES := $(INCLUDES) $(findTexturesPerPolygonCmd_EXTRA_INCLUDES)

$(findTexturesPerPolygonCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(findTexturesPerPolygonCmd_EXTRA_LFLAGS) 
$(findTexturesPerPolygonCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(findTexturesPerPolygonCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_findTexturesPerPolygonCmd clean_findTexturesPerPolygonCmd Clean_findTexturesPerPolygonCmd


$(findTexturesPerPolygonCmd_PLUGIN): $(findTexturesPerPolygonCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_findTexturesPerPolygonCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(findTexturesPerPolygonCmd_SOURCES)

clean_findTexturesPerPolygonCmd:
	-rm -f $(findTexturesPerPolygonCmd_OBJECTS)

Clean_findTexturesPerPolygonCmd:
	-rm -f $(findTexturesPerPolygonCmd_MAKEFILE).bak $(findTexturesPerPolygonCmd_OBJECTS) $(findTexturesPerPolygonCmd_PLUGIN)


plugins: $(findTexturesPerPolygonCmd_PLUGIN)
depend:	 depend_findTexturesPerPolygonCmd
clean:	 clean_findTexturesPerPolygonCmd
Clean:	 Clean_findTexturesPerPolygonCmd

