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

SRCDIR := $(TOP)/intersectCmd
DSTDIR := $(TOP)/intersectCmd

intersectCmd_SOURCES  := $(TOP)/intersectCmd/intersectCmd.cpp
intersectCmd_OBJECTS  := $(TOP)/intersectCmd/intersectCmd.o
intersectCmd_PLUGIN   := $(DSTDIR)/intersectCmd.$(EXT)
intersectCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(intersectCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(intersectCmd_EXTRA_CFLAGS)
$(intersectCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(intersectCmd_EXTRA_C++FLAGS)
$(intersectCmd_OBJECTS): INCLUDES := $(INCLUDES) $(intersectCmd_EXTRA_INCLUDES)

depend_intersectCmd:     INCLUDES := $(INCLUDES) $(intersectCmd_EXTRA_INCLUDES)

$(intersectCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(intersectCmd_EXTRA_LFLAGS) 
$(intersectCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(intersectCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_intersectCmd clean_intersectCmd Clean_intersectCmd


$(intersectCmd_PLUGIN): $(intersectCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_intersectCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(intersectCmd_SOURCES)

clean_intersectCmd:
	-rm -f $(intersectCmd_OBJECTS)

Clean_intersectCmd:
	-rm -f $(intersectCmd_MAKEFILE).bak $(intersectCmd_OBJECTS) $(intersectCmd_PLUGIN)


plugins: $(intersectCmd_PLUGIN)
depend:	 depend_intersectCmd
clean:	 clean_intersectCmd
Clean:	 Clean_intersectCmd

