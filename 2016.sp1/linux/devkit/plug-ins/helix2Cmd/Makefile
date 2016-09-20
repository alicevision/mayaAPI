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

SRCDIR := $(TOP)/helix2Cmd
DSTDIR := $(TOP)/helix2Cmd

helix2Cmd_SOURCES  := $(TOP)/helix2Cmd/helix2Cmd.cpp
helix2Cmd_OBJECTS  := $(TOP)/helix2Cmd/helix2Cmd.o
helix2Cmd_PLUGIN   := $(DSTDIR)/helix2Cmd.$(EXT)
helix2Cmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(helix2Cmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(helix2Cmd_EXTRA_CFLAGS)
$(helix2Cmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(helix2Cmd_EXTRA_C++FLAGS)
$(helix2Cmd_OBJECTS): INCLUDES := $(INCLUDES) $(helix2Cmd_EXTRA_INCLUDES)

depend_helix2Cmd:     INCLUDES := $(INCLUDES) $(helix2Cmd_EXTRA_INCLUDES)

$(helix2Cmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(helix2Cmd_EXTRA_LFLAGS) 
$(helix2Cmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(helix2Cmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_helix2Cmd clean_helix2Cmd Clean_helix2Cmd


$(helix2Cmd_PLUGIN): $(helix2Cmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_helix2Cmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(helix2Cmd_SOURCES)

clean_helix2Cmd:
	-rm -f $(helix2Cmd_OBJECTS)

Clean_helix2Cmd:
	-rm -f $(helix2Cmd_MAKEFILE).bak $(helix2Cmd_OBJECTS) $(helix2Cmd_PLUGIN)


plugins: $(helix2Cmd_PLUGIN)
depend:	 depend_helix2Cmd
clean:	 clean_helix2Cmd
Clean:	 Clean_helix2Cmd

