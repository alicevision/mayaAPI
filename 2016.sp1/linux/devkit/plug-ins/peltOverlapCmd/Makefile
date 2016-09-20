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

SRCDIR := $(TOP)/peltOverlapCmd
DSTDIR := $(TOP)/peltOverlapCmd

peltOverlapCmd_SOURCES  := $(TOP)/peltOverlapCmd/peltOverlapCmd.cpp
peltOverlapCmd_OBJECTS  := $(TOP)/peltOverlapCmd/peltOverlapCmd.o
peltOverlapCmd_PLUGIN   := $(DSTDIR)/peltOverlapCmd.$(EXT)
peltOverlapCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(peltOverlapCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(peltOverlapCmd_EXTRA_CFLAGS)
$(peltOverlapCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(peltOverlapCmd_EXTRA_C++FLAGS)
$(peltOverlapCmd_OBJECTS): INCLUDES := $(INCLUDES) $(peltOverlapCmd_EXTRA_INCLUDES)

depend_peltOverlapCmd:     INCLUDES := $(INCLUDES) $(peltOverlapCmd_EXTRA_INCLUDES)

$(peltOverlapCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(peltOverlapCmd_EXTRA_LFLAGS) 
$(peltOverlapCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(peltOverlapCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_peltOverlapCmd clean_peltOverlapCmd Clean_peltOverlapCmd


$(peltOverlapCmd_PLUGIN): $(peltOverlapCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_peltOverlapCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(peltOverlapCmd_SOURCES)

clean_peltOverlapCmd:
	-rm -f $(peltOverlapCmd_OBJECTS)

Clean_peltOverlapCmd:
	-rm -f $(peltOverlapCmd_MAKEFILE).bak $(peltOverlapCmd_OBJECTS) $(peltOverlapCmd_PLUGIN)


plugins: $(peltOverlapCmd_PLUGIN)
depend:	 depend_peltOverlapCmd
clean:	 clean_peltOverlapCmd
Clean:	 Clean_peltOverlapCmd

