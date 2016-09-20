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

SRCDIR := $(TOP)/iffPixelCmd
DSTDIR := $(TOP)/iffPixelCmd

iffPixelCmd_SOURCES  := $(TOP)/iffPixelCmd/iffPixelCmd.cpp $(TOP)/iffPixelCmd/iffreader.cpp
iffPixelCmd_OBJECTS  := $(TOP)/iffPixelCmd/iffPixelCmd.o $(TOP)/iffPixelCmd/iffreader.o
iffPixelCmd_PLUGIN   := $(DSTDIR)/iffPixelCmd.$(EXT)
iffPixelCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(iffPixelCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(iffPixelCmd_EXTRA_CFLAGS)
$(iffPixelCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(iffPixelCmd_EXTRA_C++FLAGS)
$(iffPixelCmd_OBJECTS): INCLUDES := $(INCLUDES) $(iffPixelCmd_EXTRA_INCLUDES)

depend_iffPixelCmd:     INCLUDES := $(INCLUDES) $(iffPixelCmd_EXTRA_INCLUDES)

$(iffPixelCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(iffPixelCmd_EXTRA_LFLAGS) 
$(iffPixelCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lImage -lFoundation $(iffPixelCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_iffPixelCmd clean_iffPixelCmd Clean_iffPixelCmd


$(iffPixelCmd_PLUGIN): $(iffPixelCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_iffPixelCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(iffPixelCmd_SOURCES)

clean_iffPixelCmd:
	-rm -f $(iffPixelCmd_OBJECTS)

Clean_iffPixelCmd:
	-rm -f $(iffPixelCmd_MAKEFILE).bak $(iffPixelCmd_OBJECTS) $(iffPixelCmd_PLUGIN)


plugins: $(iffPixelCmd_PLUGIN)
depend:	 depend_iffPixelCmd
clean:	 clean_iffPixelCmd
Clean:	 Clean_iffPixelCmd

