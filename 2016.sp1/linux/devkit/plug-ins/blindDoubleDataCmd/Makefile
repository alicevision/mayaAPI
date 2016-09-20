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

SRCDIR := $(TOP)/blindDoubleDataCmd
DSTDIR := $(TOP)/blindDoubleDataCmd

blindDoubleDataCmd_SOURCES  := $(TOP)/blindDoubleDataCmd/blindDoubleDataCmd.cpp
blindDoubleDataCmd_OBJECTS  := $(TOP)/blindDoubleDataCmd/blindDoubleDataCmd.o
blindDoubleDataCmd_PLUGIN   := $(DSTDIR)/blindDoubleDataCmd.$(EXT)
blindDoubleDataCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(blindDoubleDataCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(blindDoubleDataCmd_EXTRA_CFLAGS)
$(blindDoubleDataCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(blindDoubleDataCmd_EXTRA_C++FLAGS)
$(blindDoubleDataCmd_OBJECTS): INCLUDES := $(INCLUDES) $(blindDoubleDataCmd_EXTRA_INCLUDES)

depend_blindDoubleDataCmd:     INCLUDES := $(INCLUDES) $(blindDoubleDataCmd_EXTRA_INCLUDES)

$(blindDoubleDataCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(blindDoubleDataCmd_EXTRA_LFLAGS) 
$(blindDoubleDataCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(blindDoubleDataCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_blindDoubleDataCmd clean_blindDoubleDataCmd Clean_blindDoubleDataCmd


$(blindDoubleDataCmd_PLUGIN): $(blindDoubleDataCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_blindDoubleDataCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(blindDoubleDataCmd_SOURCES)

clean_blindDoubleDataCmd:
	-rm -f $(blindDoubleDataCmd_OBJECTS)

Clean_blindDoubleDataCmd:
	-rm -f $(blindDoubleDataCmd_MAKEFILE).bak $(blindDoubleDataCmd_OBJECTS) $(blindDoubleDataCmd_PLUGIN)


plugins: $(blindDoubleDataCmd_PLUGIN)
depend:	 depend_blindDoubleDataCmd
clean:	 clean_blindDoubleDataCmd
Clean:	 Clean_blindDoubleDataCmd

