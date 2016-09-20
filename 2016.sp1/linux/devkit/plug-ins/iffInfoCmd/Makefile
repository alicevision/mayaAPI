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

SRCDIR := $(TOP)/iffInfoCmd
DSTDIR := $(TOP)/iffInfoCmd

iffInfoCmd_SOURCES  := $(TOP)/iffInfoCmd/iffInfoCmd.cpp $(TOP)/iffInfoCmd/iffreader.cpp
iffInfoCmd_OBJECTS  := $(TOP)/iffInfoCmd/iffInfoCmd.o $(TOP)/iffInfoCmd/iffreader.o
iffInfoCmd_PLUGIN   := $(DSTDIR)/iffInfoCmd.$(EXT)
iffInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(iffInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(iffInfoCmd_EXTRA_CFLAGS)
$(iffInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(iffInfoCmd_EXTRA_C++FLAGS)
$(iffInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(iffInfoCmd_EXTRA_INCLUDES)

depend_iffInfoCmd:     INCLUDES := $(INCLUDES) $(iffInfoCmd_EXTRA_INCLUDES)

$(iffInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(iffInfoCmd_EXTRA_LFLAGS) 
$(iffInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lImage -lFoundation $(iffInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_iffInfoCmd clean_iffInfoCmd Clean_iffInfoCmd


$(iffInfoCmd_PLUGIN): $(iffInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_iffInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(iffInfoCmd_SOURCES)

clean_iffInfoCmd:
	-rm -f $(iffInfoCmd_OBJECTS)

Clean_iffInfoCmd:
	-rm -f $(iffInfoCmd_MAKEFILE).bak $(iffInfoCmd_OBJECTS) $(iffInfoCmd_PLUGIN)


plugins: $(iffInfoCmd_PLUGIN)
depend:	 depend_iffInfoCmd
clean:	 clean_iffInfoCmd
Clean:	 Clean_iffInfoCmd

