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

SRCDIR := $(TOP)/blindComplexDataCmd
DSTDIR := $(TOP)/blindComplexDataCmd

blindComplexDataCmd_SOURCES  := $(TOP)/blindComplexDataCmd/blindComplexDataCmd.cpp
blindComplexDataCmd_OBJECTS  := $(TOP)/blindComplexDataCmd/blindComplexDataCmd.o
blindComplexDataCmd_PLUGIN   := $(DSTDIR)/blindComplexDataCmd.$(EXT)
blindComplexDataCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(blindComplexDataCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(blindComplexDataCmd_EXTRA_CFLAGS)
$(blindComplexDataCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(blindComplexDataCmd_EXTRA_C++FLAGS)
$(blindComplexDataCmd_OBJECTS): INCLUDES := $(INCLUDES) $(blindComplexDataCmd_EXTRA_INCLUDES)

depend_blindComplexDataCmd:     INCLUDES := $(INCLUDES) $(blindComplexDataCmd_EXTRA_INCLUDES)

$(blindComplexDataCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(blindComplexDataCmd_EXTRA_LFLAGS) 
$(blindComplexDataCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(blindComplexDataCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_blindComplexDataCmd clean_blindComplexDataCmd Clean_blindComplexDataCmd


$(blindComplexDataCmd_PLUGIN): $(blindComplexDataCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_blindComplexDataCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(blindComplexDataCmd_SOURCES)

clean_blindComplexDataCmd:
	-rm -f $(blindComplexDataCmd_OBJECTS)

Clean_blindComplexDataCmd:
	-rm -f $(blindComplexDataCmd_MAKEFILE).bak $(blindComplexDataCmd_OBJECTS) $(blindComplexDataCmd_PLUGIN)


plugins: $(blindComplexDataCmd_PLUGIN)
depend:	 depend_blindComplexDataCmd
clean:	 clean_blindComplexDataCmd
Clean:	 Clean_blindComplexDataCmd

