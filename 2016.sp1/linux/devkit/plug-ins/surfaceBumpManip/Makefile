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

SRCDIR := $(TOP)/surfaceBumpManip
DSTDIR := $(TOP)/surfaceBumpManip

surfaceBumpManip_SOURCES  := $(TOP)/surfaceBumpManip/surfaceBumpManip.cpp
surfaceBumpManip_OBJECTS  := $(TOP)/surfaceBumpManip/surfaceBumpManip.o
surfaceBumpManip_PLUGIN   := $(DSTDIR)/surfaceBumpManip.$(EXT)
surfaceBumpManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(surfaceBumpManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(surfaceBumpManip_EXTRA_CFLAGS)
$(surfaceBumpManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(surfaceBumpManip_EXTRA_C++FLAGS)
$(surfaceBumpManip_OBJECTS): INCLUDES := $(INCLUDES) $(surfaceBumpManip_EXTRA_INCLUDES)

depend_surfaceBumpManip:     INCLUDES := $(INCLUDES) $(surfaceBumpManip_EXTRA_INCLUDES)

$(surfaceBumpManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(surfaceBumpManip_EXTRA_LFLAGS) 
$(surfaceBumpManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(surfaceBumpManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_surfaceBumpManip clean_surfaceBumpManip Clean_surfaceBumpManip


$(surfaceBumpManip_PLUGIN): $(surfaceBumpManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_surfaceBumpManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(surfaceBumpManip_SOURCES)

clean_surfaceBumpManip:
	-rm -f $(surfaceBumpManip_OBJECTS)

Clean_surfaceBumpManip:
	-rm -f $(surfaceBumpManip_MAKEFILE).bak $(surfaceBumpManip_OBJECTS) $(surfaceBumpManip_PLUGIN)


plugins: $(surfaceBumpManip_PLUGIN)
depend:	 depend_surfaceBumpManip
clean:	 clean_surfaceBumpManip
Clean:	 Clean_surfaceBumpManip

