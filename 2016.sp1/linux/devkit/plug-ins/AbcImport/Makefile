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

SRCDIR := $(TOP)/AbcImport
DSTDIR := $(TOP)/AbcImport

AbcImport_SOURCES  := $(TOP)/AbcImport/AbcImport.cpp $(TOP)/AbcImport/AbcImportStrings.cpp $(TOP)/AbcImport/AlembicNode.cpp $(TOP)/AbcImport/CreateSceneHelper.cpp $(TOP)/AbcImport/main.cpp $(TOP)/AbcImport/MeshHelper.cpp $(TOP)/AbcImport/NodeIteratorVisitorHelper.cpp $(TOP)/AbcImport/PointHelper.cpp $(TOP)/AbcImport/util.cpp $(TOP)/AbcImport/XformHelper.cpp $(TOP)/AbcImport/CameraHelper.cpp $(TOP)/AbcImport/NurbsCurveHelper.cpp $(TOP)/AbcImport/LocatorHelper.cpp $(TOP)/AbcImport/NurbsSurfaceHelper.cpp $(TOP)/AbcImport/AlembicImportFileTranslator.cpp
AbcImport_OBJECTS  := $(TOP)/AbcImport/AbcImport.o $(TOP)/AbcImport/AbcImportStrings.o $(TOP)/AbcImport/AlembicNode.o $(TOP)/AbcImport/CreateSceneHelper.o $(TOP)/AbcImport/main.o $(TOP)/AbcImport/MeshHelper.o $(TOP)/AbcImport/NodeIteratorVisitorHelper.o $(TOP)/AbcImport/PointHelper.o $(TOP)/AbcImport/util.o $(TOP)/AbcImport/XformHelper.o $(TOP)/AbcImport/CameraHelper.o $(TOP)/AbcImport/NurbsCurveHelper.o $(TOP)/AbcImport/LocatorHelper.o $(TOP)/AbcImport/NurbsSurfaceHelper.o $(TOP)/AbcImport/AlembicImportFileTranslator.o
AbcImport_PLUGIN   := $(DSTDIR)/AbcImport.$(EXT)
AbcImport_MAKEFILE := $(DSTDIR)/Makefile

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

$(AbcImport_OBJECTS): CFLAGS   := $(CFLAGS)   $(AbcImport_EXTRA_CFLAGS)
$(AbcImport_OBJECTS): C++FLAGS := $(C++FLAGS) $(AbcImport_EXTRA_C++FLAGS)
$(AbcImport_OBJECTS): INCLUDES := $(INCLUDES) $(AbcImport_EXTRA_INCLUDES)

depend_AbcImport:     INCLUDES := $(INCLUDES) $(AbcImport_EXTRA_INCLUDES)

$(AbcImport_PLUGIN):  LFLAGS   := $(LFLAGS) $(AbcImport_EXTRA_LFLAGS) 
$(AbcImport_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaAnim -lOpenMayaFX -lOpenMayaRender -lOpenMayaUI -lAlembicAbcCoreFactory -lAlembicAbcCoreHDF5 -lAlembicAbcCoreOgawa -lAlembicAbcGeom -lAlembicAbc -lAlembicAbcCoreAbstract -lAlembicAbcMaterial -lAlembicAbcCollection -lAlembicOgawa -lAlembicUtil -lhdf5_hl -lhdf5 -lAlembicIex -lAlembicHalf -lAlembicImath -lz $(AbcImport_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_AbcImport clean_AbcImport Clean_AbcImport


$(AbcImport_PLUGIN): $(AbcImport_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_AbcImport :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(AbcImport_SOURCES)

clean_AbcImport:
	-rm -f $(AbcImport_OBJECTS)

Clean_AbcImport:
	-rm -f $(AbcImport_MAKEFILE).bak $(AbcImport_OBJECTS) $(AbcImport_PLUGIN)


prebuiltPlugins: $(AbcImport_PLUGIN)
depend:	 depend_AbcImport
clean:	 clean_AbcImport
Clean:	 Clean_AbcImport

