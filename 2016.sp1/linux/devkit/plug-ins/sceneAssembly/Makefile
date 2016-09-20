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

SRCDIR := $(TOP)/sceneAssembly
DSTDIR := $(TOP)/sceneAssembly

sceneAssembly_SOURCES  := $(TOP)/sceneAssembly/sceneAssemblyPluginMain.cpp $(TOP)/sceneAssembly/assemblyReference.cpp $(TOP)/sceneAssembly/assemblyDefinition.cpp $(TOP)/sceneAssembly/assemblyDefinitionFileCache.cpp $(TOP)/sceneAssembly/adskRepresentations.cpp $(TOP)/sceneAssembly/adskRepresentationFactory.cpp $(TOP)/sceneAssembly/adskRepresentationCmd.cpp $(TOP)/sceneAssembly/assemblyReferenceInitialRep.cpp $(TOP)/sceneAssembly/adskPrepareRenderGlobals.cpp $(TOP)/sceneAssembly/adskSceneMetadataCmd.cpp
sceneAssembly_OBJECTS  := $(TOP)/sceneAssembly/sceneAssemblyPluginMain.o $(TOP)/sceneAssembly/assemblyReference.o $(TOP)/sceneAssembly/assemblyDefinition.o $(TOP)/sceneAssembly/assemblyDefinitionFileCache.o $(TOP)/sceneAssembly/adskRepresentations.o $(TOP)/sceneAssembly/adskRepresentationFactory.o $(TOP)/sceneAssembly/adskRepresentationCmd.o $(TOP)/sceneAssembly/assemblyReferenceInitialRep.o $(TOP)/sceneAssembly/adskPrepareRenderGlobals.o $(TOP)/sceneAssembly/adskSceneMetadataCmd.o
sceneAssembly_PLUGIN   := $(DSTDIR)/sceneAssembly.$(EXT)
sceneAssembly_MAKEFILE := $(DSTDIR)/Makefile

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

$(sceneAssembly_OBJECTS): CFLAGS   := $(CFLAGS)   $(sceneAssembly_EXTRA_CFLAGS)
$(sceneAssembly_OBJECTS): C++FLAGS := $(C++FLAGS) $(sceneAssembly_EXTRA_C++FLAGS)
$(sceneAssembly_OBJECTS): INCLUDES := $(INCLUDES) $(sceneAssembly_EXTRA_INCLUDES)

depend_sceneAssembly:     INCLUDES := $(INCLUDES) $(sceneAssembly_EXTRA_INCLUDES)

$(sceneAssembly_PLUGIN):  LFLAGS   := $(LFLAGS) $(sceneAssembly_EXTRA_LFLAGS) 
$(sceneAssembly_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lMetaData $(sceneAssembly_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_sceneAssembly clean_sceneAssembly Clean_sceneAssembly


$(sceneAssembly_PLUGIN): $(sceneAssembly_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_sceneAssembly :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(sceneAssembly_SOURCES)

clean_sceneAssembly:
	-rm -f $(sceneAssembly_OBJECTS)

Clean_sceneAssembly:
	-rm -f $(sceneAssembly_MAKEFILE).bak $(sceneAssembly_OBJECTS) $(sceneAssembly_PLUGIN)


plugins: $(sceneAssembly_PLUGIN)
depend:	 depend_sceneAssembly
clean:	 clean_sceneAssembly
Clean:	 Clean_sceneAssembly

