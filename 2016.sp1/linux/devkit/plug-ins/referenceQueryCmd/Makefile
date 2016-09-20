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

SRCDIR := $(TOP)/referenceQueryCmd
DSTDIR := $(TOP)/referenceQueryCmd

referenceQueryCmd_SOURCES  := $(TOP)/referenceQueryCmd/referenceQueryCmd.cpp
referenceQueryCmd_OBJECTS  := $(TOP)/referenceQueryCmd/referenceQueryCmd.o
referenceQueryCmd_PLUGIN   := $(DSTDIR)/referenceQueryCmd.$(EXT)
referenceQueryCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(referenceQueryCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(referenceQueryCmd_EXTRA_CFLAGS)
$(referenceQueryCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(referenceQueryCmd_EXTRA_C++FLAGS)
$(referenceQueryCmd_OBJECTS): INCLUDES := $(INCLUDES) $(referenceQueryCmd_EXTRA_INCLUDES)

depend_referenceQueryCmd:     INCLUDES := $(INCLUDES) $(referenceQueryCmd_EXTRA_INCLUDES)

$(referenceQueryCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(referenceQueryCmd_EXTRA_LFLAGS) 
$(referenceQueryCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(referenceQueryCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_referenceQueryCmd clean_referenceQueryCmd Clean_referenceQueryCmd


$(referenceQueryCmd_PLUGIN): $(referenceQueryCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_referenceQueryCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(referenceQueryCmd_SOURCES)

clean_referenceQueryCmd:
	-rm -f $(referenceQueryCmd_OBJECTS)

Clean_referenceQueryCmd:
	-rm -f $(referenceQueryCmd_MAKEFILE).bak $(referenceQueryCmd_OBJECTS) $(referenceQueryCmd_PLUGIN)


plugins: $(referenceQueryCmd_PLUGIN)
depend:	 depend_referenceQueryCmd
clean:	 clean_referenceQueryCmd
Clean:	 Clean_referenceQueryCmd

