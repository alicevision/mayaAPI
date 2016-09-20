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

SRCDIR := $(TOP)/whatisCmd
DSTDIR := $(TOP)/whatisCmd

whatisCmd_SOURCES  := $(TOP)/whatisCmd/whatisCmd.cpp
whatisCmd_OBJECTS  := $(TOP)/whatisCmd/whatisCmd.o
whatisCmd_PLUGIN   := $(DSTDIR)/whatisCmd.$(EXT)
whatisCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(whatisCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(whatisCmd_EXTRA_CFLAGS)
$(whatisCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(whatisCmd_EXTRA_C++FLAGS)
$(whatisCmd_OBJECTS): INCLUDES := $(INCLUDES) $(whatisCmd_EXTRA_INCLUDES)

depend_whatisCmd:     INCLUDES := $(INCLUDES) $(whatisCmd_EXTRA_INCLUDES)

$(whatisCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(whatisCmd_EXTRA_LFLAGS) 
$(whatisCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(whatisCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_whatisCmd clean_whatisCmd Clean_whatisCmd


$(whatisCmd_PLUGIN): $(whatisCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_whatisCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(whatisCmd_SOURCES)

clean_whatisCmd:
	-rm -f $(whatisCmd_OBJECTS)

Clean_whatisCmd:
	-rm -f $(whatisCmd_MAKEFILE).bak $(whatisCmd_OBJECTS) $(whatisCmd_PLUGIN)


plugins: $(whatisCmd_PLUGIN)
depend:	 depend_whatisCmd
clean:	 clean_whatisCmd
Clean:	 Clean_whatisCmd

