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

SRCDIR := $(TOP)/blast2Cmd
DSTDIR := $(TOP)/blast2Cmd

blast2Cmd_SOURCES  := $(TOP)/blast2Cmd/blast2Cmd.cpp
blast2Cmd_OBJECTS  := $(TOP)/blast2Cmd/blast2Cmd.o
blast2Cmd_PLUGIN   := $(DSTDIR)/blast2Cmd.$(EXT)
blast2Cmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(blast2Cmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(blast2Cmd_EXTRA_CFLAGS)
$(blast2Cmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(blast2Cmd_EXTRA_C++FLAGS)
$(blast2Cmd_OBJECTS): INCLUDES := $(INCLUDES) $(blast2Cmd_EXTRA_INCLUDES)

depend_blast2Cmd:     INCLUDES := $(INCLUDES) $(blast2Cmd_EXTRA_INCLUDES)

$(blast2Cmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(blast2Cmd_EXTRA_LFLAGS) 
$(blast2Cmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaAnim -lOpenMayaRender -lFoundation -lGL $(blast2Cmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_blast2Cmd clean_blast2Cmd Clean_blast2Cmd


$(blast2Cmd_PLUGIN): $(blast2Cmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_blast2Cmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(blast2Cmd_SOURCES)

clean_blast2Cmd:
	-rm -f $(blast2Cmd_OBJECTS)

Clean_blast2Cmd:
	-rm -f $(blast2Cmd_MAKEFILE).bak $(blast2Cmd_OBJECTS) $(blast2Cmd_PLUGIN)


plugins: $(blast2Cmd_PLUGIN)
depend:	 depend_blast2Cmd
clean:	 clean_blast2Cmd
Clean:	 Clean_blast2Cmd

