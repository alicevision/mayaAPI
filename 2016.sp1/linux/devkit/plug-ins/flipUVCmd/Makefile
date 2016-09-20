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

SRCDIR := $(TOP)/flipUVCmd
DSTDIR := $(TOP)/flipUVCmd

flipUVCmd_SOURCES  := $(TOP)/flipUVCmd/flipUVCmd.cpp $(TOP)/flipUVCmd/flipUVMain.cpp
flipUVCmd_OBJECTS  := $(TOP)/flipUVCmd/flipUVCmd.o $(TOP)/flipUVCmd/flipUVMain.o
flipUVCmd_PLUGIN   := $(DSTDIR)/flipUVCmd.$(EXT)
flipUVCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(flipUVCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(flipUVCmd_EXTRA_CFLAGS)
$(flipUVCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(flipUVCmd_EXTRA_C++FLAGS)
$(flipUVCmd_OBJECTS): INCLUDES := $(INCLUDES) $(flipUVCmd_EXTRA_INCLUDES)

depend_flipUVCmd:     INCLUDES := $(INCLUDES) $(flipUVCmd_EXTRA_INCLUDES)

$(flipUVCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(flipUVCmd_EXTRA_LFLAGS) 
$(flipUVCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL -lGLU $(flipUVCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_flipUVCmd clean_flipUVCmd Clean_flipUVCmd


$(flipUVCmd_PLUGIN): $(flipUVCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_flipUVCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(flipUVCmd_SOURCES)

clean_flipUVCmd:
	-rm -f $(flipUVCmd_OBJECTS)

Clean_flipUVCmd:
	-rm -f $(flipUVCmd_MAKEFILE).bak $(flipUVCmd_OBJECTS) $(flipUVCmd_PLUGIN)


plugins: $(flipUVCmd_PLUGIN)
depend:	 depend_flipUVCmd
clean:	 clean_flipUVCmd
Clean:	 Clean_flipUVCmd

