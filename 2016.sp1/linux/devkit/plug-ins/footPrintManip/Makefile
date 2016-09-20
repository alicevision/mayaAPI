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

SRCDIR := $(TOP)/footPrintManip
DSTDIR := $(TOP)/footPrintManip

footPrintManip_SOURCES  := $(TOP)/footPrintManip/footPrintManip.cpp
footPrintManip_OBJECTS  := $(TOP)/footPrintManip/footPrintManip.o
footPrintManip_PLUGIN   := $(DSTDIR)/footPrintManip.$(EXT)
footPrintManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(footPrintManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(footPrintManip_EXTRA_CFLAGS)
$(footPrintManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(footPrintManip_EXTRA_C++FLAGS)
$(footPrintManip_OBJECTS): INCLUDES := $(INCLUDES) $(footPrintManip_EXTRA_INCLUDES)

depend_footPrintManip:     INCLUDES := $(INCLUDES) $(footPrintManip_EXTRA_INCLUDES)

$(footPrintManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(footPrintManip_EXTRA_LFLAGS) 
$(footPrintManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lFoundation -lGL $(footPrintManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_footPrintManip clean_footPrintManip Clean_footPrintManip


$(footPrintManip_PLUGIN): $(footPrintManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_footPrintManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(footPrintManip_SOURCES)

clean_footPrintManip:
	-rm -f $(footPrintManip_OBJECTS)

Clean_footPrintManip:
	-rm -f $(footPrintManip_MAKEFILE).bak $(footPrintManip_OBJECTS) $(footPrintManip_PLUGIN)


plugins: $(footPrintManip_PLUGIN)
depend:	 depend_footPrintManip
clean:	 clean_footPrintManip
Clean:	 Clean_footPrintManip

