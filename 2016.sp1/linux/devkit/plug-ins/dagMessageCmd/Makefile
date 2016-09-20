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

SRCDIR := $(TOP)/dagMessageCmd
DSTDIR := $(TOP)/dagMessageCmd

dagMessageCmd_SOURCES  := $(TOP)/dagMessageCmd/dagMessageCmd.cpp
dagMessageCmd_OBJECTS  := $(TOP)/dagMessageCmd/dagMessageCmd.o
dagMessageCmd_PLUGIN   := $(DSTDIR)/dagMessageCmd.$(EXT)
dagMessageCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(dagMessageCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(dagMessageCmd_EXTRA_CFLAGS)
$(dagMessageCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(dagMessageCmd_EXTRA_C++FLAGS)
$(dagMessageCmd_OBJECTS): INCLUDES := $(INCLUDES) $(dagMessageCmd_EXTRA_INCLUDES)

depend_dagMessageCmd:     INCLUDES := $(INCLUDES) $(dagMessageCmd_EXTRA_INCLUDES)

$(dagMessageCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(dagMessageCmd_EXTRA_LFLAGS) 
$(dagMessageCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(dagMessageCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_dagMessageCmd clean_dagMessageCmd Clean_dagMessageCmd


$(dagMessageCmd_PLUGIN): $(dagMessageCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_dagMessageCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(dagMessageCmd_SOURCES)

clean_dagMessageCmd:
	-rm -f $(dagMessageCmd_OBJECTS)

Clean_dagMessageCmd:
	-rm -f $(dagMessageCmd_MAKEFILE).bak $(dagMessageCmd_OBJECTS) $(dagMessageCmd_PLUGIN)


plugins: $(dagMessageCmd_PLUGIN)
depend:	 depend_dagMessageCmd
clean:	 clean_dagMessageCmd
Clean:	 Clean_dagMessageCmd

