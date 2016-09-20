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

SRCDIR := $(TOP)/udpDevice
DSTDIR := $(TOP)/udpDevice

udpDevice_SOURCES  := $(TOP)/udpDevice/udpDevice.cpp
udpDevice_OBJECTS  := $(TOP)/udpDevice/udpDevice.o
udpDevice_PLUGIN   := $(DSTDIR)/udpDevice.$(EXT)
udpDevice_MAKEFILE := $(DSTDIR)/Makefile

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

$(udpDevice_OBJECTS): CFLAGS   := $(CFLAGS)   $(udpDevice_EXTRA_CFLAGS)
$(udpDevice_OBJECTS): C++FLAGS := $(C++FLAGS) $(udpDevice_EXTRA_C++FLAGS)
$(udpDevice_OBJECTS): INCLUDES := $(INCLUDES) $(udpDevice_EXTRA_INCLUDES)

depend_udpDevice:     INCLUDES := $(INCLUDES) $(udpDevice_EXTRA_INCLUDES)

$(udpDevice_PLUGIN):  LFLAGS   := $(LFLAGS) $(udpDevice_EXTRA_LFLAGS) 
$(udpDevice_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lOpenMayaUI -lFoundation $(udpDevice_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_udpDevice clean_udpDevice Clean_udpDevice


$(udpDevice_PLUGIN): $(udpDevice_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_udpDevice :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(udpDevice_SOURCES)

clean_udpDevice:
	-rm -f $(udpDevice_OBJECTS)

Clean_udpDevice:
	-rm -f $(udpDevice_MAKEFILE).bak $(udpDevice_OBJECTS) $(udpDevice_PLUGIN)


plugins: $(udpDevice_PLUGIN)
depend:	 depend_udpDevice
clean:	 clean_udpDevice
Clean:	 Clean_udpDevice

