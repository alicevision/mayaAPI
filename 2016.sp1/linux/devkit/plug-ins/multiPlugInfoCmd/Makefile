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

SRCDIR := $(TOP)/multiPlugInfoCmd
DSTDIR := $(TOP)/multiPlugInfoCmd

multiPlugInfoCmd_SOURCES  := $(TOP)/multiPlugInfoCmd/multiPlugInfoCmd.cpp
multiPlugInfoCmd_OBJECTS  := $(TOP)/multiPlugInfoCmd/multiPlugInfoCmd.o
multiPlugInfoCmd_PLUGIN   := $(DSTDIR)/multiPlugInfoCmd.$(EXT)
multiPlugInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(multiPlugInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(multiPlugInfoCmd_EXTRA_CFLAGS)
$(multiPlugInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(multiPlugInfoCmd_EXTRA_C++FLAGS)
$(multiPlugInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(multiPlugInfoCmd_EXTRA_INCLUDES)

depend_multiPlugInfoCmd:     INCLUDES := $(INCLUDES) $(multiPlugInfoCmd_EXTRA_INCLUDES)

$(multiPlugInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(multiPlugInfoCmd_EXTRA_LFLAGS) 
$(multiPlugInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(multiPlugInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_multiPlugInfoCmd clean_multiPlugInfoCmd Clean_multiPlugInfoCmd


$(multiPlugInfoCmd_PLUGIN): $(multiPlugInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_multiPlugInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(multiPlugInfoCmd_SOURCES)

clean_multiPlugInfoCmd:
	-rm -f $(multiPlugInfoCmd_OBJECTS)

Clean_multiPlugInfoCmd:
	-rm -f $(multiPlugInfoCmd_MAKEFILE).bak $(multiPlugInfoCmd_OBJECTS) $(multiPlugInfoCmd_PLUGIN)


plugins: $(multiPlugInfoCmd_PLUGIN)
depend:	 depend_multiPlugInfoCmd
clean:	 clean_multiPlugInfoCmd
Clean:	 Clean_multiPlugInfoCmd

