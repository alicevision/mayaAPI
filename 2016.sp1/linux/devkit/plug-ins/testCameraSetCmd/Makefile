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

SRCDIR := $(TOP)/testCameraSetCmd
DSTDIR := $(TOP)/testCameraSetCmd

testCameraSetCmd_SOURCES  := $(TOP)/testCameraSetCmd/testCameraSetCmd.cpp
testCameraSetCmd_OBJECTS  := $(TOP)/testCameraSetCmd/testCameraSetCmd.o
testCameraSetCmd_PLUGIN   := $(DSTDIR)/testCameraSetCmd.$(EXT)
testCameraSetCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(testCameraSetCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(testCameraSetCmd_EXTRA_CFLAGS)
$(testCameraSetCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(testCameraSetCmd_EXTRA_C++FLAGS)
$(testCameraSetCmd_OBJECTS): INCLUDES := $(INCLUDES) $(testCameraSetCmd_EXTRA_INCLUDES)

depend_testCameraSetCmd:     INCLUDES := $(INCLUDES) $(testCameraSetCmd_EXTRA_INCLUDES)

$(testCameraSetCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(testCameraSetCmd_EXTRA_LFLAGS) 
$(testCameraSetCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(testCameraSetCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_testCameraSetCmd clean_testCameraSetCmd Clean_testCameraSetCmd


$(testCameraSetCmd_PLUGIN): $(testCameraSetCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_testCameraSetCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(testCameraSetCmd_SOURCES)

clean_testCameraSetCmd:
	-rm -f $(testCameraSetCmd_OBJECTS)

Clean_testCameraSetCmd:
	-rm -f $(testCameraSetCmd_MAKEFILE).bak $(testCameraSetCmd_OBJECTS) $(testCameraSetCmd_PLUGIN)


plugins: $(testCameraSetCmd_PLUGIN)
depend:	 depend_testCameraSetCmd
clean:	 clean_testCameraSetCmd
Clean:	 Clean_testCameraSetCmd

