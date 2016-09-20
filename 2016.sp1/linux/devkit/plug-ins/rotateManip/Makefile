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

SRCDIR := $(TOP)/rotateManip
DSTDIR := $(TOP)/rotateManip

rotateManip_SOURCES  := $(TOP)/rotateManip/rotateManip.cpp
rotateManip_OBJECTS  := $(TOP)/rotateManip/rotateManip.o
rotateManip_PLUGIN   := $(DSTDIR)/rotateManip.$(EXT)
rotateManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(rotateManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(rotateManip_EXTRA_CFLAGS)
$(rotateManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(rotateManip_EXTRA_C++FLAGS)
$(rotateManip_OBJECTS): INCLUDES := $(INCLUDES) $(rotateManip_EXTRA_INCLUDES)

depend_rotateManip:     INCLUDES := $(INCLUDES) $(rotateManip_EXTRA_INCLUDES)

$(rotateManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(rotateManip_EXTRA_LFLAGS) 
$(rotateManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(rotateManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_rotateManip clean_rotateManip Clean_rotateManip


$(rotateManip_PLUGIN): $(rotateManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_rotateManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(rotateManip_SOURCES)

clean_rotateManip:
	-rm -f $(rotateManip_OBJECTS)

Clean_rotateManip:
	-rm -f $(rotateManip_MAKEFILE).bak $(rotateManip_OBJECTS) $(rotateManip_PLUGIN)


plugins: $(rotateManip_PLUGIN)
depend:	 depend_rotateManip
clean:	 clean_rotateManip
Clean:	 Clean_rotateManip

