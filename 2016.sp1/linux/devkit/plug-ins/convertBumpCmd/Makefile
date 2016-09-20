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

SRCDIR := $(TOP)/convertBumpCmd
DSTDIR := $(TOP)/convertBumpCmd

convertBumpCmd_SOURCES  := $(TOP)/convertBumpCmd/convertBumpCmd.cpp
convertBumpCmd_OBJECTS  := $(TOP)/convertBumpCmd/convertBumpCmd.o
convertBumpCmd_PLUGIN   := $(DSTDIR)/convertBumpCmd.$(EXT)
convertBumpCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(convertBumpCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(convertBumpCmd_EXTRA_CFLAGS)
$(convertBumpCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(convertBumpCmd_EXTRA_C++FLAGS)
$(convertBumpCmd_OBJECTS): INCLUDES := $(INCLUDES) $(convertBumpCmd_EXTRA_INCLUDES)

depend_convertBumpCmd:     INCLUDES := $(INCLUDES) $(convertBumpCmd_EXTRA_INCLUDES)

$(convertBumpCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(convertBumpCmd_EXTRA_LFLAGS) 
$(convertBumpCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(convertBumpCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_convertBumpCmd clean_convertBumpCmd Clean_convertBumpCmd


$(convertBumpCmd_PLUGIN): $(convertBumpCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_convertBumpCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(convertBumpCmd_SOURCES)

clean_convertBumpCmd:
	-rm -f $(convertBumpCmd_OBJECTS)

Clean_convertBumpCmd:
	-rm -f $(convertBumpCmd_MAKEFILE).bak $(convertBumpCmd_OBJECTS) $(convertBumpCmd_PLUGIN)


plugins: $(convertBumpCmd_PLUGIN)
depend:	 depend_convertBumpCmd
clean:	 clean_convertBumpCmd
Clean:	 Clean_convertBumpCmd

