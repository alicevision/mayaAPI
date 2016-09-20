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

SRCDIR := $(TOP)/blindShortDataCmd
DSTDIR := $(TOP)/blindShortDataCmd

blindShortDataCmd_SOURCES  := $(TOP)/blindShortDataCmd/blindShortDataCmd.cpp
blindShortDataCmd_OBJECTS  := $(TOP)/blindShortDataCmd/blindShortDataCmd.o
blindShortDataCmd_PLUGIN   := $(DSTDIR)/blindShortDataCmd.$(EXT)
blindShortDataCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(blindShortDataCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(blindShortDataCmd_EXTRA_CFLAGS)
$(blindShortDataCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(blindShortDataCmd_EXTRA_C++FLAGS)
$(blindShortDataCmd_OBJECTS): INCLUDES := $(INCLUDES) $(blindShortDataCmd_EXTRA_INCLUDES)

depend_blindShortDataCmd:     INCLUDES := $(INCLUDES) $(blindShortDataCmd_EXTRA_INCLUDES)

$(blindShortDataCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(blindShortDataCmd_EXTRA_LFLAGS) 
$(blindShortDataCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(blindShortDataCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_blindShortDataCmd clean_blindShortDataCmd Clean_blindShortDataCmd


$(blindShortDataCmd_PLUGIN): $(blindShortDataCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_blindShortDataCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(blindShortDataCmd_SOURCES)

clean_blindShortDataCmd:
	-rm -f $(blindShortDataCmd_OBJECTS)

Clean_blindShortDataCmd:
	-rm -f $(blindShortDataCmd_MAKEFILE).bak $(blindShortDataCmd_OBJECTS) $(blindShortDataCmd_PLUGIN)


plugins: $(blindShortDataCmd_PLUGIN)
depend:	 depend_blindShortDataCmd
clean:	 clean_blindShortDataCmd
Clean:	 Clean_blindShortDataCmd

