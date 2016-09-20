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

SRCDIR := $(TOP)/fileIOMsgCmd
DSTDIR := $(TOP)/fileIOMsgCmd

fileIOMsgCmd_SOURCES  := $(TOP)/fileIOMsgCmd/fileIOMsgCmd.cpp
fileIOMsgCmd_OBJECTS  := $(TOP)/fileIOMsgCmd/fileIOMsgCmd.o
fileIOMsgCmd_PLUGIN   := $(DSTDIR)/fileIOMsgCmd.$(EXT)
fileIOMsgCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(fileIOMsgCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(fileIOMsgCmd_EXTRA_CFLAGS)
$(fileIOMsgCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(fileIOMsgCmd_EXTRA_C++FLAGS)
$(fileIOMsgCmd_OBJECTS): INCLUDES := $(INCLUDES) $(fileIOMsgCmd_EXTRA_INCLUDES)

depend_fileIOMsgCmd:     INCLUDES := $(INCLUDES) $(fileIOMsgCmd_EXTRA_INCLUDES)

$(fileIOMsgCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(fileIOMsgCmd_EXTRA_LFLAGS) 
$(fileIOMsgCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(fileIOMsgCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_fileIOMsgCmd clean_fileIOMsgCmd Clean_fileIOMsgCmd


$(fileIOMsgCmd_PLUGIN): $(fileIOMsgCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_fileIOMsgCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(fileIOMsgCmd_SOURCES)

clean_fileIOMsgCmd:
	-rm -f $(fileIOMsgCmd_OBJECTS)

Clean_fileIOMsgCmd:
	-rm -f $(fileIOMsgCmd_MAKEFILE).bak $(fileIOMsgCmd_OBJECTS) $(fileIOMsgCmd_PLUGIN)


plugins: $(fileIOMsgCmd_PLUGIN)
depend:	 depend_fileIOMsgCmd
clean:	 clean_fileIOMsgCmd
Clean:	 Clean_fileIOMsgCmd

