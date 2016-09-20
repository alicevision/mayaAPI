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

SRCDIR := $(TOP)/swissArmyManip
DSTDIR := $(TOP)/swissArmyManip

swissArmyManip_SOURCES  := $(TOP)/swissArmyManip/swissArmyManip.cpp
swissArmyManip_OBJECTS  := $(TOP)/swissArmyManip/swissArmyManip.o
swissArmyManip_PLUGIN   := $(DSTDIR)/swissArmyManip.$(EXT)
swissArmyManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(swissArmyManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(swissArmyManip_EXTRA_CFLAGS)
$(swissArmyManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(swissArmyManip_EXTRA_C++FLAGS)
$(swissArmyManip_OBJECTS): INCLUDES := $(INCLUDES) $(swissArmyManip_EXTRA_INCLUDES)

depend_swissArmyManip:     INCLUDES := $(INCLUDES) $(swissArmyManip_EXTRA_INCLUDES)

$(swissArmyManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(swissArmyManip_EXTRA_LFLAGS) 
$(swissArmyManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL $(swissArmyManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_swissArmyManip clean_swissArmyManip Clean_swissArmyManip


$(swissArmyManip_PLUGIN): $(swissArmyManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_swissArmyManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(swissArmyManip_SOURCES)

clean_swissArmyManip:
	-rm -f $(swissArmyManip_OBJECTS)

Clean_swissArmyManip:
	-rm -f $(swissArmyManip_MAKEFILE).bak $(swissArmyManip_OBJECTS) $(swissArmyManip_PLUGIN)


plugins: $(swissArmyManip_PLUGIN)
depend:	 depend_swissArmyManip
clean:	 clean_swissArmyManip
Clean:	 Clean_swissArmyManip

