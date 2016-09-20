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

SRCDIR := $(TOP)/pickCmd
DSTDIR := $(TOP)/pickCmd

pickCmd_SOURCES  := $(TOP)/pickCmd/pickCmd.cpp
pickCmd_OBJECTS  := $(TOP)/pickCmd/pickCmd.o
pickCmd_PLUGIN   := $(DSTDIR)/pickCmd.$(EXT)
pickCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(pickCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(pickCmd_EXTRA_CFLAGS)
$(pickCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(pickCmd_EXTRA_C++FLAGS)
$(pickCmd_OBJECTS): INCLUDES := $(INCLUDES) $(pickCmd_EXTRA_INCLUDES)

depend_pickCmd:     INCLUDES := $(INCLUDES) $(pickCmd_EXTRA_INCLUDES)

$(pickCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(pickCmd_EXTRA_LFLAGS) 
$(pickCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(pickCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_pickCmd clean_pickCmd Clean_pickCmd


$(pickCmd_PLUGIN): $(pickCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_pickCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(pickCmd_SOURCES)

clean_pickCmd:
	-rm -f $(pickCmd_OBJECTS)

Clean_pickCmd:
	-rm -f $(pickCmd_MAKEFILE).bak $(pickCmd_OBJECTS) $(pickCmd_PLUGIN)


plugins: $(pickCmd_PLUGIN)
depend:	 depend_pickCmd
clean:	 clean_pickCmd
Clean:	 Clean_pickCmd

