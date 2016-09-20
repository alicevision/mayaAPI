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

SRCDIR := $(TOP)/randomizerDevice
DSTDIR := $(TOP)/randomizerDevice

randomizerDevice_SOURCES  := $(TOP)/randomizerDevice/randomizerDevice.cpp
randomizerDevice_OBJECTS  := $(TOP)/randomizerDevice/randomizerDevice.o
randomizerDevice_PLUGIN   := $(DSTDIR)/randomizerDevice.$(EXT)
randomizerDevice_MAKEFILE := $(DSTDIR)/Makefile

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

$(randomizerDevice_OBJECTS): CFLAGS   := $(CFLAGS)   $(randomizerDevice_EXTRA_CFLAGS)
$(randomizerDevice_OBJECTS): C++FLAGS := $(C++FLAGS) $(randomizerDevice_EXTRA_C++FLAGS)
$(randomizerDevice_OBJECTS): INCLUDES := $(INCLUDES) $(randomizerDevice_EXTRA_INCLUDES)

depend_randomizerDevice:     INCLUDES := $(INCLUDES) $(randomizerDevice_EXTRA_INCLUDES)

$(randomizerDevice_PLUGIN):  LFLAGS   := $(LFLAGS) $(randomizerDevice_EXTRA_LFLAGS) 
$(randomizerDevice_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lOpenMayaUI -lFoundation $(randomizerDevice_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_randomizerDevice clean_randomizerDevice Clean_randomizerDevice


$(randomizerDevice_PLUGIN): $(randomizerDevice_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_randomizerDevice :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(randomizerDevice_SOURCES)

clean_randomizerDevice:
	-rm -f $(randomizerDevice_OBJECTS)

Clean_randomizerDevice:
	-rm -f $(randomizerDevice_MAKEFILE).bak $(randomizerDevice_OBJECTS) $(randomizerDevice_PLUGIN)


plugins: $(randomizerDevice_PLUGIN)
depend:	 depend_randomizerDevice
clean:	 clean_randomizerDevice
Clean:	 Clean_randomizerDevice

