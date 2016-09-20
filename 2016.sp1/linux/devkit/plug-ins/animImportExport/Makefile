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
all : prebuiltPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/animImportExport
DSTDIR := $(TOP)/animImportExport

animImportExport_SOURCES  := $(TOP)/animImportExport/animImportExport.cpp $(TOP)/animImportExport/animFileUtils.cpp
animImportExport_OBJECTS  := $(TOP)/animImportExport/animImportExport.o $(TOP)/animImportExport/animFileUtils.o
animImportExport_PLUGIN   := $(DSTDIR)/animImportExport.$(EXT)
animImportExport_MAKEFILE := $(DSTDIR)/Makefile

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

$(animImportExport_OBJECTS): CFLAGS   := $(CFLAGS)   $(animImportExport_EXTRA_CFLAGS)
$(animImportExport_OBJECTS): C++FLAGS := $(C++FLAGS) $(animImportExport_EXTRA_C++FLAGS)
$(animImportExport_OBJECTS): INCLUDES := $(INCLUDES) $(animImportExport_EXTRA_INCLUDES)

depend_animImportExport:     INCLUDES := $(INCLUDES) $(animImportExport_EXTRA_INCLUDES)

$(animImportExport_PLUGIN):  LFLAGS   := $(LFLAGS) $(animImportExport_EXTRA_LFLAGS) 
$(animImportExport_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(animImportExport_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_animImportExport clean_animImportExport Clean_animImportExport


$(animImportExport_PLUGIN): $(animImportExport_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_animImportExport :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(animImportExport_SOURCES)

clean_animImportExport:
	-rm -f $(animImportExport_OBJECTS)

Clean_animImportExport:
	-rm -f $(animImportExport_MAKEFILE).bak $(animImportExport_OBJECTS) $(animImportExport_PLUGIN)


prebuiltPlugins: $(animImportExport_PLUGIN)
depend:	 depend_animImportExport
clean:	 clean_animImportExport
Clean:	 Clean_animImportExport

