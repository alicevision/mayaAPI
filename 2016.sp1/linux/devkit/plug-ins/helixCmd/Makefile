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

SRCDIR := $(TOP)/helixCmd
DSTDIR := $(TOP)/helixCmd

helixCmd_SOURCES  := $(TOP)/helixCmd/helixCmd.cpp
helixCmd_OBJECTS  := $(TOP)/helixCmd/helixCmd.o
helixCmd_PLUGIN   := $(DSTDIR)/helixCmd.$(EXT)
helixCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(helixCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(helixCmd_EXTRA_CFLAGS)
$(helixCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(helixCmd_EXTRA_C++FLAGS)
$(helixCmd_OBJECTS): INCLUDES := $(INCLUDES) $(helixCmd_EXTRA_INCLUDES)

depend_helixCmd:     INCLUDES := $(INCLUDES) $(helixCmd_EXTRA_INCLUDES)

$(helixCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(helixCmd_EXTRA_LFLAGS) 
$(helixCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(helixCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_helixCmd clean_helixCmd Clean_helixCmd


$(helixCmd_PLUGIN): $(helixCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_helixCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(helixCmd_SOURCES)

clean_helixCmd:
	-rm -f $(helixCmd_OBJECTS)

Clean_helixCmd:
	-rm -f $(helixCmd_MAKEFILE).bak $(helixCmd_OBJECTS) $(helixCmd_PLUGIN)


plugins: $(helixCmd_PLUGIN)
depend:	 depend_helixCmd
clean:	 clean_helixCmd
Clean:	 Clean_helixCmd

