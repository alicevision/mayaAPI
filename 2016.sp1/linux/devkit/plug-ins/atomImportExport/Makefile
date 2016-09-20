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

SRCDIR := $(TOP)/atomImportExport
DSTDIR := $(TOP)/atomImportExport

atomImportExport_SOURCES  := $(TOP)/atomImportExport/atomAnimLayers.cpp $(TOP)/atomImportExport/atomCachedPlugs.cpp $(TOP)/atomImportExport/atomImportExport.cpp $(TOP)/atomImportExport/atomFileUtils.cpp $(TOP)/atomImportExport/atomNodeNameReplacer.cpp
atomImportExport_OBJECTS  := $(TOP)/atomImportExport/atomAnimLayers.o $(TOP)/atomImportExport/atomCachedPlugs.o $(TOP)/atomImportExport/atomImportExport.o $(TOP)/atomImportExport/atomFileUtils.o $(TOP)/atomImportExport/atomNodeNameReplacer.o
atomImportExport_PLUGIN   := $(DSTDIR)/atomImportExport.$(EXT)
atomImportExport_MAKEFILE := $(DSTDIR)/Makefile

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

$(atomImportExport_OBJECTS): CFLAGS   := $(CFLAGS)   $(atomImportExport_EXTRA_CFLAGS)
$(atomImportExport_OBJECTS): C++FLAGS := $(C++FLAGS) $(atomImportExport_EXTRA_C++FLAGS)
$(atomImportExport_OBJECTS): INCLUDES := $(INCLUDES) $(atomImportExport_EXTRA_INCLUDES)

depend_atomImportExport:     INCLUDES := $(INCLUDES) $(atomImportExport_EXTRA_INCLUDES)

$(atomImportExport_PLUGIN):  LFLAGS   := $(LFLAGS) $(atomImportExport_EXTRA_LFLAGS) 
$(atomImportExport_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lOpenMayaUI -lFoundation $(atomImportExport_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_atomImportExport clean_atomImportExport Clean_atomImportExport


$(atomImportExport_PLUGIN): $(atomImportExport_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_atomImportExport :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(atomImportExport_SOURCES)

clean_atomImportExport:
	-rm -f $(atomImportExport_OBJECTS)

Clean_atomImportExport:
	-rm -f $(atomImportExport_MAKEFILE).bak $(atomImportExport_OBJECTS) $(atomImportExport_PLUGIN)


prebuiltPlugins: $(atomImportExport_PLUGIN)
depend:	 depend_atomImportExport
clean:	 clean_atomImportExport
Clean:	 Clean_atomImportExport

