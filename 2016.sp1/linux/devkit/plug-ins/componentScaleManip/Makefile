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

SRCDIR := $(TOP)/componentScaleManip
DSTDIR := $(TOP)/componentScaleManip

componentScaleManip_SOURCES  := $(TOP)/componentScaleManip/componentScaleManip.cpp
componentScaleManip_OBJECTS  := $(TOP)/componentScaleManip/componentScaleManip.o
componentScaleManip_PLUGIN   := $(DSTDIR)/componentScaleManip.$(EXT)
componentScaleManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(componentScaleManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(componentScaleManip_EXTRA_CFLAGS)
$(componentScaleManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(componentScaleManip_EXTRA_C++FLAGS)
$(componentScaleManip_OBJECTS): INCLUDES := $(INCLUDES) $(componentScaleManip_EXTRA_INCLUDES)

depend_componentScaleManip:     INCLUDES := $(INCLUDES) $(componentScaleManip_EXTRA_INCLUDES)

$(componentScaleManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(componentScaleManip_EXTRA_LFLAGS) 
$(componentScaleManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(componentScaleManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_componentScaleManip clean_componentScaleManip Clean_componentScaleManip


$(componentScaleManip_PLUGIN): $(componentScaleManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_componentScaleManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(componentScaleManip_SOURCES)

clean_componentScaleManip:
	-rm -f $(componentScaleManip_OBJECTS)

Clean_componentScaleManip:
	-rm -f $(componentScaleManip_MAKEFILE).bak $(componentScaleManip_OBJECTS) $(componentScaleManip_PLUGIN)


plugins: $(componentScaleManip_PLUGIN)
depend:	 depend_componentScaleManip
clean:	 clean_componentScaleManip
Clean:	 Clean_componentScaleManip

