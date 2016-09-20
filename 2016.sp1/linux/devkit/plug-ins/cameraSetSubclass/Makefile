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

SRCDIR := $(TOP)/cameraSetSubclass
DSTDIR := $(TOP)/cameraSetSubclass

cameraSetSubclass_SOURCES  := $(TOP)/cameraSetSubclass/exCameraSet.cpp $(TOP)/cameraSetSubclass/testExCameraSetCmd.cpp
cameraSetSubclass_OBJECTS  := $(TOP)/cameraSetSubclass/exCameraSet.o $(TOP)/cameraSetSubclass/testExCameraSetCmd.o
cameraSetSubclass_PLUGIN   := $(DSTDIR)/cameraSetSubclass.$(EXT)
cameraSetSubclass_MAKEFILE := $(DSTDIR)/Makefile

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

$(cameraSetSubclass_OBJECTS): CFLAGS   := $(CFLAGS)   $(cameraSetSubclass_EXTRA_CFLAGS)
$(cameraSetSubclass_OBJECTS): C++FLAGS := $(C++FLAGS) $(cameraSetSubclass_EXTRA_C++FLAGS)
$(cameraSetSubclass_OBJECTS): INCLUDES := $(INCLUDES) $(cameraSetSubclass_EXTRA_INCLUDES)

depend_cameraSetSubclass:     INCLUDES := $(INCLUDES) $(cameraSetSubclass_EXTRA_INCLUDES)

$(cameraSetSubclass_PLUGIN):  LFLAGS   := $(LFLAGS) $(cameraSetSubclass_EXTRA_LFLAGS) 
$(cameraSetSubclass_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(cameraSetSubclass_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cameraSetSubclass clean_cameraSetSubclass Clean_cameraSetSubclass


$(cameraSetSubclass_PLUGIN): $(cameraSetSubclass_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cameraSetSubclass :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cameraSetSubclass_SOURCES)

clean_cameraSetSubclass:
	-rm -f $(cameraSetSubclass_OBJECTS)

Clean_cameraSetSubclass:
	-rm -f $(cameraSetSubclass_MAKEFILE).bak $(cameraSetSubclass_OBJECTS) $(cameraSetSubclass_PLUGIN)


plugins: $(cameraSetSubclass_PLUGIN)
depend:	 depend_cameraSetSubclass
clean:	 clean_cameraSetSubclass
Clean:	 Clean_cameraSetSubclass

