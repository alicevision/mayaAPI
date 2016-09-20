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
all : thirdPartyPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/pnTriangles
DSTDIR := $(TOP)/pnTriangles

pnTriangles_SOURCES  := $(TOP)/pnTriangles/pnTrianglesNode.cpp $(TOP)/pnTriangles/ATIext.cpp
pnTriangles_OBJECTS  := $(TOP)/pnTriangles/pnTrianglesNode.o $(TOP)/pnTriangles/ATIext.o
pnTriangles_PLUGIN   := $(DSTDIR)/pnTriangles.$(EXT)
pnTriangles_MAKEFILE := $(DSTDIR)/Makefile

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

$(pnTriangles_OBJECTS): CFLAGS   := $(CFLAGS)   $(pnTriangles_EXTRA_CFLAGS)
$(pnTriangles_OBJECTS): C++FLAGS := $(C++FLAGS) $(pnTriangles_EXTRA_C++FLAGS)
$(pnTriangles_OBJECTS): INCLUDES := $(INCLUDES) $(pnTriangles_EXTRA_INCLUDES)

depend_pnTriangles:     INCLUDES := $(INCLUDES) $(pnTriangles_EXTRA_INCLUDES)

$(pnTriangles_PLUGIN):  LFLAGS   := $(LFLAGS) $(pnTriangles_EXTRA_LFLAGS) 
$(pnTriangles_PLUGIN):  LIBS     := $(LIBS)   -lFoundation -lOpenMaya -lOpenMayaUI -lGL $(pnTriangles_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_pnTriangles clean_pnTriangles Clean_pnTriangles


$(pnTriangles_PLUGIN): $(pnTriangles_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_pnTriangles :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(pnTriangles_SOURCES)

clean_pnTriangles:
	-rm -f $(pnTriangles_OBJECTS)

Clean_pnTriangles:
	-rm -f $(pnTriangles_MAKEFILE).bak $(pnTriangles_OBJECTS) $(pnTriangles_PLUGIN)


thirdPartyPlugins: $(pnTriangles_PLUGIN)
depend:	 depend_pnTriangles
clean:	 clean_pnTriangles
Clean:	 Clean_pnTriangles

