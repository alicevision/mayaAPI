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

SRCDIR := $(TOP)/gameInputDevice
DSTDIR := $(TOP)/gameInputDevice

gameInputDevice_SOURCES  := $(TOP)/gameInputDevice/gameInputDevice.cpp
gameInputDevice_OBJECTS  := $(TOP)/gameInputDevice/gameInputDevice.o
gameInputDevice_PLUGIN   := $(DSTDIR)/gameInputDevice.$(EXT)
gameInputDevice_MAKEFILE := $(DSTDIR)/Makefile

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

$(gameInputDevice_OBJECTS): CFLAGS   := $(CFLAGS)   $(gameInputDevice_EXTRA_CFLAGS)
$(gameInputDevice_OBJECTS): C++FLAGS := $(C++FLAGS) $(gameInputDevice_EXTRA_C++FLAGS)
$(gameInputDevice_OBJECTS): INCLUDES := $(INCLUDES) $(gameInputDevice_EXTRA_INCLUDES)

depend_gameInputDevice:     INCLUDES := $(INCLUDES) $(gameInputDevice_EXTRA_INCLUDES)

$(gameInputDevice_PLUGIN):  LFLAGS   := $(LFLAGS) $(gameInputDevice_EXTRA_LFLAGS) 
$(gameInputDevice_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lOpenMayaUI -lFoundation $(gameInputDevice_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_gameInputDevice clean_gameInputDevice Clean_gameInputDevice


$(gameInputDevice_PLUGIN): $(gameInputDevice_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_gameInputDevice :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(gameInputDevice_SOURCES)

clean_gameInputDevice:
	-rm -f $(gameInputDevice_OBJECTS)

Clean_gameInputDevice:
	-rm -f $(gameInputDevice_MAKEFILE).bak $(gameInputDevice_OBJECTS) $(gameInputDevice_PLUGIN)


plugins: $(gameInputDevice_PLUGIN)
depend:	 depend_gameInputDevice
clean:	 clean_gameInputDevice
Clean:	 Clean_gameInputDevice

