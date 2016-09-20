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

SRCDIR := $(TOP)/uiDrawManager
DSTDIR := $(TOP)/uiDrawManager

uiDrawManager_SOURCES  := $(TOP)/uiDrawManager/uiDrawManager.cpp
uiDrawManager_OBJECTS  := $(TOP)/uiDrawManager/uiDrawManager.o
uiDrawManager_PLUGIN   := $(DSTDIR)/uiDrawManager.$(EXT)
uiDrawManager_MAKEFILE := $(DSTDIR)/Makefile

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

$(uiDrawManager_OBJECTS): CFLAGS   := $(CFLAGS)   $(uiDrawManager_EXTRA_CFLAGS)
$(uiDrawManager_OBJECTS): C++FLAGS := $(C++FLAGS) $(uiDrawManager_EXTRA_C++FLAGS)
$(uiDrawManager_OBJECTS): INCLUDES := $(INCLUDES) $(uiDrawManager_EXTRA_INCLUDES)

depend_uiDrawManager:     INCLUDES := $(INCLUDES) $(uiDrawManager_EXTRA_INCLUDES)

$(uiDrawManager_PLUGIN):  LFLAGS   := $(LFLAGS) $(uiDrawManager_EXTRA_LFLAGS) 
$(uiDrawManager_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation $(uiDrawManager_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_uiDrawManager clean_uiDrawManager Clean_uiDrawManager


$(uiDrawManager_PLUGIN): $(uiDrawManager_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_uiDrawManager :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(uiDrawManager_SOURCES)

clean_uiDrawManager:
	-rm -f $(uiDrawManager_OBJECTS)

Clean_uiDrawManager:
	-rm -f $(uiDrawManager_MAKEFILE).bak $(uiDrawManager_OBJECTS) $(uiDrawManager_PLUGIN)


plugins: $(uiDrawManager_PLUGIN)
depend:	 depend_uiDrawManager
clean:	 clean_uiDrawManager
Clean:	 Clean_uiDrawManager

