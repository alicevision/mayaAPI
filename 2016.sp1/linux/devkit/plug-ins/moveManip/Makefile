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

SRCDIR := $(TOP)/moveManip
DSTDIR := $(TOP)/moveManip

moveManip_SOURCES  := $(TOP)/moveManip/moveManip.cpp
moveManip_OBJECTS  := $(TOP)/moveManip/moveManip.o
moveManip_PLUGIN   := $(DSTDIR)/moveManip.$(EXT)
moveManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(moveManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(moveManip_EXTRA_CFLAGS)
$(moveManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(moveManip_EXTRA_C++FLAGS)
$(moveManip_OBJECTS): INCLUDES := $(INCLUDES) $(moveManip_EXTRA_INCLUDES)

depend_moveManip:     INCLUDES := $(INCLUDES) $(moveManip_EXTRA_INCLUDES)

$(moveManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(moveManip_EXTRA_LFLAGS) 
$(moveManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(moveManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_moveManip clean_moveManip Clean_moveManip


$(moveManip_PLUGIN): $(moveManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_moveManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(moveManip_SOURCES)

clean_moveManip:
	-rm -f $(moveManip_OBJECTS)

Clean_moveManip:
	-rm -f $(moveManip_MAKEFILE).bak $(moveManip_OBJECTS) $(moveManip_PLUGIN)


plugins: $(moveManip_PLUGIN)
depend:	 depend_moveManip
clean:	 clean_moveManip
Clean:	 Clean_moveManip

