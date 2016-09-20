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

SRCDIR := $(TOP)/findFileTexturesCmd
DSTDIR := $(TOP)/findFileTexturesCmd

findFileTexturesCmd_SOURCES  := $(TOP)/findFileTexturesCmd/findFileTexturesCmd.cpp
findFileTexturesCmd_OBJECTS  := $(TOP)/findFileTexturesCmd/findFileTexturesCmd.o
findFileTexturesCmd_PLUGIN   := $(DSTDIR)/findFileTexturesCmd.$(EXT)
findFileTexturesCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(findFileTexturesCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(findFileTexturesCmd_EXTRA_CFLAGS)
$(findFileTexturesCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(findFileTexturesCmd_EXTRA_C++FLAGS)
$(findFileTexturesCmd_OBJECTS): INCLUDES := $(INCLUDES) $(findFileTexturesCmd_EXTRA_INCLUDES)

depend_findFileTexturesCmd:     INCLUDES := $(INCLUDES) $(findFileTexturesCmd_EXTRA_INCLUDES)

$(findFileTexturesCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(findFileTexturesCmd_EXTRA_LFLAGS) 
$(findFileTexturesCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(findFileTexturesCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_findFileTexturesCmd clean_findFileTexturesCmd Clean_findFileTexturesCmd


$(findFileTexturesCmd_PLUGIN): $(findFileTexturesCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_findFileTexturesCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(findFileTexturesCmd_SOURCES)

clean_findFileTexturesCmd:
	-rm -f $(findFileTexturesCmd_OBJECTS)

Clean_findFileTexturesCmd:
	-rm -f $(findFileTexturesCmd_MAKEFILE).bak $(findFileTexturesCmd_OBJECTS) $(findFileTexturesCmd_PLUGIN)


plugins: $(findFileTexturesCmd_PLUGIN)
depend:	 depend_findFileTexturesCmd
clean:	 clean_findFileTexturesCmd
Clean:	 Clean_findFileTexturesCmd

