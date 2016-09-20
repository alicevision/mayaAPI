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

SRCDIR := $(TOP)/animExportUtil
DSTDIR := $(TOP)/animExportUtil

animExportUtil_SOURCES  := $(TOP)/animExportUtil/animFileExport.cpp $(TOP)/animExportUtil/animExportUtil.cpp
animExportUtil_OBJECTS  := $(TOP)/animExportUtil/animFileExport.o $(TOP)/animExportUtil/animExportUtil.o
animExportUtil_PLUGIN   := $(DSTDIR)/animExportUtil.$(EXT)
animExportUtil_MAKEFILE := $(DSTDIR)/Makefile

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

$(animExportUtil_OBJECTS): CFLAGS   := $(CFLAGS)   $(animExportUtil_EXTRA_CFLAGS)
$(animExportUtil_OBJECTS): C++FLAGS := $(C++FLAGS) $(animExportUtil_EXTRA_C++FLAGS)
$(animExportUtil_OBJECTS): INCLUDES := $(INCLUDES) $(animExportUtil_EXTRA_INCLUDES)

depend_animExportUtil:     INCLUDES := $(INCLUDES) $(animExportUtil_EXTRA_INCLUDES)

$(animExportUtil_PLUGIN):  LFLAGS   := $(LFLAGS) $(animExportUtil_EXTRA_LFLAGS) 
$(animExportUtil_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(animExportUtil_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_animExportUtil clean_animExportUtil Clean_animExportUtil


$(animExportUtil_PLUGIN): $(animExportUtil_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_animExportUtil :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(animExportUtil_SOURCES)

clean_animExportUtil:
	-rm -f $(animExportUtil_OBJECTS)

Clean_animExportUtil:
	-rm -f $(animExportUtil_MAKEFILE).bak $(animExportUtil_OBJECTS) $(animExportUtil_PLUGIN)


plugins: $(animExportUtil_PLUGIN)
depend:	 depend_animExportUtil
clean:	 clean_animExportUtil
Clean:	 Clean_animExportUtil

