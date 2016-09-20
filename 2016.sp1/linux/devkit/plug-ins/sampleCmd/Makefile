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

SRCDIR := $(TOP)/sampleCmd
DSTDIR := $(TOP)/sampleCmd

sampleCmd_SOURCES  := $(TOP)/sampleCmd/sampleCmd.cpp
sampleCmd_OBJECTS  := $(TOP)/sampleCmd/sampleCmd.o
sampleCmd_PLUGIN   := $(DSTDIR)/sampleCmd.$(EXT)
sampleCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(sampleCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(sampleCmd_EXTRA_CFLAGS)
$(sampleCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(sampleCmd_EXTRA_C++FLAGS)
$(sampleCmd_OBJECTS): INCLUDES := $(INCLUDES) $(sampleCmd_EXTRA_INCLUDES)

depend_sampleCmd:     INCLUDES := $(INCLUDES) $(sampleCmd_EXTRA_INCLUDES)

$(sampleCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(sampleCmd_EXTRA_LFLAGS) 
$(sampleCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation $(sampleCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_sampleCmd clean_sampleCmd Clean_sampleCmd


$(sampleCmd_PLUGIN): $(sampleCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_sampleCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(sampleCmd_SOURCES)

clean_sampleCmd:
	-rm -f $(sampleCmd_OBJECTS)

Clean_sampleCmd:
	-rm -f $(sampleCmd_MAKEFILE).bak $(sampleCmd_OBJECTS) $(sampleCmd_PLUGIN)


plugins: $(sampleCmd_PLUGIN)
depend:	 depend_sampleCmd
clean:	 clean_sampleCmd
Clean:	 Clean_sampleCmd

