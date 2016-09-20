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

SRCDIR := $(TOP)/translateCmd
DSTDIR := $(TOP)/translateCmd

translateCmd_SOURCES  := $(TOP)/translateCmd/translateCmd.cpp
translateCmd_OBJECTS  := $(TOP)/translateCmd/translateCmd.o
translateCmd_PLUGIN   := $(DSTDIR)/translateCmd.$(EXT)
translateCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(translateCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(translateCmd_EXTRA_CFLAGS)
$(translateCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(translateCmd_EXTRA_C++FLAGS)
$(translateCmd_OBJECTS): INCLUDES := $(INCLUDES) $(translateCmd_EXTRA_INCLUDES)

depend_translateCmd:     INCLUDES := $(INCLUDES) $(translateCmd_EXTRA_INCLUDES)

$(translateCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(translateCmd_EXTRA_LFLAGS) 
$(translateCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(translateCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_translateCmd clean_translateCmd Clean_translateCmd


$(translateCmd_PLUGIN): $(translateCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_translateCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(translateCmd_SOURCES)

clean_translateCmd:
	-rm -f $(translateCmd_OBJECTS)

Clean_translateCmd:
	-rm -f $(translateCmd_MAKEFILE).bak $(translateCmd_OBJECTS) $(translateCmd_PLUGIN)


plugins: $(translateCmd_PLUGIN)
depend:	 depend_translateCmd
clean:	 clean_translateCmd
Clean:	 Clean_translateCmd

