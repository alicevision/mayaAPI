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

SRCDIR := $(TOP)/AbcExport
DSTDIR := $(TOP)/AbcExport

AbcExport_SOURCES  := $(TOP)/AbcExport/AbcExport.cpp $(TOP)/AbcExport/AbcWriteJob.cpp $(TOP)/AbcExport/AttributesWriter.cpp $(TOP)/AbcExport/MayaMeshWriter.cpp $(TOP)/AbcExport/MayaPointPrimitiveWriter.cpp $(TOP)/AbcExport/MayaTransformWriter.cpp $(TOP)/AbcExport/MayaUtility.cpp $(TOP)/AbcExport/MayaCameraWriter.cpp $(TOP)/AbcExport/MayaNurbsCurveWriter.cpp $(TOP)/AbcExport/MayaLocatorWriter.cpp $(TOP)/AbcExport/MayaNurbsSurfaceWriter.cpp
AbcExport_OBJECTS  := $(TOP)/AbcExport/AbcExport.o $(TOP)/AbcExport/AbcWriteJob.o $(TOP)/AbcExport/AttributesWriter.o $(TOP)/AbcExport/MayaMeshWriter.o $(TOP)/AbcExport/MayaPointPrimitiveWriter.o $(TOP)/AbcExport/MayaTransformWriter.o $(TOP)/AbcExport/MayaUtility.o $(TOP)/AbcExport/MayaCameraWriter.o $(TOP)/AbcExport/MayaNurbsCurveWriter.o $(TOP)/AbcExport/MayaLocatorWriter.o $(TOP)/AbcExport/MayaNurbsSurfaceWriter.o
AbcExport_PLUGIN   := $(DSTDIR)/AbcExport.$(EXT)
AbcExport_MAKEFILE := $(DSTDIR)/Makefile

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

$(AbcExport_OBJECTS): CFLAGS   := $(CFLAGS)   $(AbcExport_EXTRA_CFLAGS)
$(AbcExport_OBJECTS): C++FLAGS := $(C++FLAGS) $(AbcExport_EXTRA_C++FLAGS)
$(AbcExport_OBJECTS): INCLUDES := $(INCLUDES) $(AbcExport_EXTRA_INCLUDES)

depend_AbcExport:     INCLUDES := $(INCLUDES) $(AbcExport_EXTRA_INCLUDES)

$(AbcExport_PLUGIN):  LFLAGS   := $(LFLAGS) $(AbcExport_EXTRA_LFLAGS) 
$(AbcExport_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaAnim -lOpenMayaFX -lOpenMayaRender -lOpenMayaUI -lAlembicAbcCoreFactory -lAlembicAbcCoreHDF5 -lAlembicAbcCoreOgawa -lAlembicAbcGeom -lAlembicAbc -lAlembicAbcCoreAbstract -lAlembicAbcMaterial -lAlembicAbcCollection -lAlembicOgawa -lAlembicUtil -lhdf5_hl -lhdf5 -lAlembicIex -lAlembicHalf -lAlembicImath -lz $(AbcExport_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_AbcExport clean_AbcExport Clean_AbcExport


$(AbcExport_PLUGIN): $(AbcExport_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_AbcExport :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(AbcExport_SOURCES)

clean_AbcExport:
	-rm -f $(AbcExport_OBJECTS)

Clean_AbcExport:
	-rm -f $(AbcExport_MAKEFILE).bak $(AbcExport_OBJECTS) $(AbcExport_PLUGIN)


prebuiltPlugins: $(AbcExport_PLUGIN)
depend:	 depend_AbcExport
clean:	 clean_AbcExport
Clean:	 Clean_AbcExport

