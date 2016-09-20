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

SRCDIR := $(TOP)/squareScaleManip
DSTDIR := $(TOP)/squareScaleManip

squareScaleManip_SOURCES  := $(TOP)/squareScaleManip/squareScaleManip.cpp $(TOP)/squareScaleManip/manipulatorMath.cpp
squareScaleManip_OBJECTS  := $(TOP)/squareScaleManip/squareScaleManip.o $(TOP)/squareScaleManip/manipulatorMath.o
squareScaleManip_PLUGIN   := $(DSTDIR)/squareScaleManip.$(EXT)
squareScaleManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(squareScaleManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(squareScaleManip_EXTRA_CFLAGS)
$(squareScaleManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(squareScaleManip_EXTRA_C++FLAGS)
$(squareScaleManip_OBJECTS): INCLUDES := $(INCLUDES) $(squareScaleManip_EXTRA_INCLUDES)

depend_squareScaleManip:     INCLUDES := $(INCLUDES) $(squareScaleManip_EXTRA_INCLUDES)

$(squareScaleManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(squareScaleManip_EXTRA_LFLAGS) 
$(squareScaleManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaUI -lOpenMayaRender -lGL $(squareScaleManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_squareScaleManip clean_squareScaleManip Clean_squareScaleManip


$(squareScaleManip_PLUGIN): $(squareScaleManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_squareScaleManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(squareScaleManip_SOURCES)

clean_squareScaleManip:
	-rm -f $(squareScaleManip_OBJECTS)

Clean_squareScaleManip:
	-rm -f $(squareScaleManip_MAKEFILE).bak $(squareScaleManip_OBJECTS) $(squareScaleManip_PLUGIN)


plugins: $(squareScaleManip_PLUGIN)
depend:	 depend_squareScaleManip
clean:	 clean_squareScaleManip
Clean:	 Clean_squareScaleManip

