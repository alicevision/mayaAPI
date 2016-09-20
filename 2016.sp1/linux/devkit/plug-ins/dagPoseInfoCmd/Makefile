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

SRCDIR := $(TOP)/dagPoseInfoCmd
DSTDIR := $(TOP)/dagPoseInfoCmd

dagPoseInfoCmd_SOURCES  := $(TOP)/dagPoseInfoCmd/dagPoseInfoCmd.cpp
dagPoseInfoCmd_OBJECTS  := $(TOP)/dagPoseInfoCmd/dagPoseInfoCmd.o
dagPoseInfoCmd_PLUGIN   := $(DSTDIR)/dagPoseInfoCmd.$(EXT)
dagPoseInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(dagPoseInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(dagPoseInfoCmd_EXTRA_CFLAGS)
$(dagPoseInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(dagPoseInfoCmd_EXTRA_C++FLAGS)
$(dagPoseInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(dagPoseInfoCmd_EXTRA_INCLUDES)

depend_dagPoseInfoCmd:     INCLUDES := $(INCLUDES) $(dagPoseInfoCmd_EXTRA_INCLUDES)

$(dagPoseInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(dagPoseInfoCmd_EXTRA_LFLAGS) 
$(dagPoseInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(dagPoseInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_dagPoseInfoCmd clean_dagPoseInfoCmd Clean_dagPoseInfoCmd


$(dagPoseInfoCmd_PLUGIN): $(dagPoseInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_dagPoseInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(dagPoseInfoCmd_SOURCES)

clean_dagPoseInfoCmd:
	-rm -f $(dagPoseInfoCmd_OBJECTS)

Clean_dagPoseInfoCmd:
	-rm -f $(dagPoseInfoCmd_MAKEFILE).bak $(dagPoseInfoCmd_OBJECTS) $(dagPoseInfoCmd_PLUGIN)


plugins: $(dagPoseInfoCmd_PLUGIN)
depend:	 depend_dagPoseInfoCmd
clean:	 clean_dagPoseInfoCmd
Clean:	 Clean_dagPoseInfoCmd

