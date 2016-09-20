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

SRCDIR := $(TOP)/progressWindowCmd
DSTDIR := $(TOP)/progressWindowCmd

progressWindowCmd_SOURCES  := $(TOP)/progressWindowCmd/progressWindowCmd.cpp
progressWindowCmd_OBJECTS  := $(TOP)/progressWindowCmd/progressWindowCmd.o
progressWindowCmd_PLUGIN   := $(DSTDIR)/progressWindowCmd.$(EXT)
progressWindowCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(progressWindowCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(progressWindowCmd_EXTRA_CFLAGS)
$(progressWindowCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(progressWindowCmd_EXTRA_C++FLAGS)
$(progressWindowCmd_OBJECTS): INCLUDES := $(INCLUDES) $(progressWindowCmd_EXTRA_INCLUDES)

depend_progressWindowCmd:     INCLUDES := $(INCLUDES) $(progressWindowCmd_EXTRA_INCLUDES)

$(progressWindowCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(progressWindowCmd_EXTRA_LFLAGS) 
$(progressWindowCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(progressWindowCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_progressWindowCmd clean_progressWindowCmd Clean_progressWindowCmd


$(progressWindowCmd_PLUGIN): $(progressWindowCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_progressWindowCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(progressWindowCmd_SOURCES)

clean_progressWindowCmd:
	-rm -f $(progressWindowCmd_OBJECTS)

Clean_progressWindowCmd:
	-rm -f $(progressWindowCmd_MAKEFILE).bak $(progressWindowCmd_OBJECTS) $(progressWindowCmd_PLUGIN)


plugins: $(progressWindowCmd_PLUGIN)
depend:	 depend_progressWindowCmd
clean:	 clean_progressWindowCmd
Clean:	 Clean_progressWindowCmd

