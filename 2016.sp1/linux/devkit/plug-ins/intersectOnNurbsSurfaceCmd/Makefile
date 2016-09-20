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

SRCDIR := $(TOP)/intersectOnNurbsSurfaceCmd
DSTDIR := $(TOP)/intersectOnNurbsSurfaceCmd

intersectOnNurbsSurfaceCmd_SOURCES  := $(TOP)/intersectOnNurbsSurfaceCmd/intersectOnNurbsSurfaceCmd.cpp
intersectOnNurbsSurfaceCmd_OBJECTS  := $(TOP)/intersectOnNurbsSurfaceCmd/intersectOnNurbsSurfaceCmd.o
intersectOnNurbsSurfaceCmd_PLUGIN   := $(DSTDIR)/intersectOnNurbsSurfaceCmd.$(EXT)
intersectOnNurbsSurfaceCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(intersectOnNurbsSurfaceCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(intersectOnNurbsSurfaceCmd_EXTRA_CFLAGS)
$(intersectOnNurbsSurfaceCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(intersectOnNurbsSurfaceCmd_EXTRA_C++FLAGS)
$(intersectOnNurbsSurfaceCmd_OBJECTS): INCLUDES := $(INCLUDES) $(intersectOnNurbsSurfaceCmd_EXTRA_INCLUDES)

depend_intersectOnNurbsSurfaceCmd:     INCLUDES := $(INCLUDES) $(intersectOnNurbsSurfaceCmd_EXTRA_INCLUDES)

$(intersectOnNurbsSurfaceCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(intersectOnNurbsSurfaceCmd_EXTRA_LFLAGS) 
$(intersectOnNurbsSurfaceCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(intersectOnNurbsSurfaceCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_intersectOnNurbsSurfaceCmd clean_intersectOnNurbsSurfaceCmd Clean_intersectOnNurbsSurfaceCmd


$(intersectOnNurbsSurfaceCmd_PLUGIN): $(intersectOnNurbsSurfaceCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_intersectOnNurbsSurfaceCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(intersectOnNurbsSurfaceCmd_SOURCES)

clean_intersectOnNurbsSurfaceCmd:
	-rm -f $(intersectOnNurbsSurfaceCmd_OBJECTS)

Clean_intersectOnNurbsSurfaceCmd:
	-rm -f $(intersectOnNurbsSurfaceCmd_MAKEFILE).bak $(intersectOnNurbsSurfaceCmd_OBJECTS) $(intersectOnNurbsSurfaceCmd_PLUGIN)


plugins: $(intersectOnNurbsSurfaceCmd_PLUGIN)
depend:	 depend_intersectOnNurbsSurfaceCmd
clean:	 clean_intersectOnNurbsSurfaceCmd
Clean:	 Clean_intersectOnNurbsSurfaceCmd

