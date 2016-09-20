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

SRCDIR := $(TOP)/lineManip
DSTDIR := $(TOP)/lineManip

lineManip_SOURCES  := $(TOP)/lineManip/lineManip.cpp $(TOP)/lineManip/manipulatorMath.cpp
lineManip_OBJECTS  := $(TOP)/lineManip/lineManip.o $(TOP)/lineManip/manipulatorMath.o
lineManip_PLUGIN   := $(DSTDIR)/lineManip.$(EXT)
lineManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(lineManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(lineManip_EXTRA_CFLAGS)
$(lineManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(lineManip_EXTRA_C++FLAGS)
$(lineManip_OBJECTS): INCLUDES := $(INCLUDES) $(lineManip_EXTRA_INCLUDES)

depend_lineManip:     INCLUDES := $(INCLUDES) $(lineManip_EXTRA_INCLUDES)

$(lineManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(lineManip_EXTRA_LFLAGS) 
$(lineManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaUI -lOpenMayaRender $(lineManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lineManip clean_lineManip Clean_lineManip


$(lineManip_PLUGIN): $(lineManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lineManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lineManip_SOURCES)

clean_lineManip:
	-rm -f $(lineManip_OBJECTS)

Clean_lineManip:
	-rm -f $(lineManip_MAKEFILE).bak $(lineManip_OBJECTS) $(lineManip_PLUGIN)


plugins: $(lineManip_PLUGIN)
depend:	 depend_lineManip
clean:	 clean_lineManip
Clean:	 Clean_lineManip

