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

SRCDIR := $(TOP)/AbcBullet
DSTDIR := $(TOP)/AbcBullet

AbcBullet_SOURCES  := $(TOP)/AbcBullet/AbcBullet.cpp $(TOP)/AbcBullet/AbcWriteJob.cpp $(TOP)/AbcBullet/AttributesWriter.cpp $(TOP)/AbcBullet/MayaTransformWriter.cpp $(TOP)/AbcBullet/MayaUtility.cpp $(TOP)/AbcBullet/MayaTransformCollectionWriter.cpp
AbcBullet_OBJECTS  := $(TOP)/AbcBullet/AbcBullet.o $(TOP)/AbcBullet/AbcWriteJob.o $(TOP)/AbcBullet/AttributesWriter.o $(TOP)/AbcBullet/MayaTransformWriter.o $(TOP)/AbcBullet/MayaUtility.o $(TOP)/AbcBullet/MayaTransformCollectionWriter.o
AbcBullet_PLUGIN   := $(DSTDIR)/AbcBullet.$(EXT)
AbcBullet_MAKEFILE := $(DSTDIR)/Makefile

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

$(AbcBullet_OBJECTS): CFLAGS   := $(CFLAGS)   $(AbcBullet_EXTRA_CFLAGS)
$(AbcBullet_OBJECTS): C++FLAGS := $(C++FLAGS) $(AbcBullet_EXTRA_C++FLAGS)
$(AbcBullet_OBJECTS): INCLUDES := $(INCLUDES) $(AbcBullet_EXTRA_INCLUDES)

depend_AbcBullet:     INCLUDES := $(INCLUDES) $(AbcBullet_EXTRA_INCLUDES)

$(AbcBullet_PLUGIN):  LFLAGS   := $(LFLAGS) $(AbcBullet_EXTRA_LFLAGS) 
$(AbcBullet_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaAnim -lOpenMayaFX -lOpenMayaRender -lOpenMayaUI -lAlembicAbcCoreHDF5 -lAlembicAbcGeom -lAlembicAbc -lAlembicAbcCoreAbstract -lAlembicUtil -lhdf5_hl -lhdf5 -lAlembicIex -lAlembicHalf -lz $(AbcBullet_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_AbcBullet clean_AbcBullet Clean_AbcBullet


$(AbcBullet_PLUGIN): $(AbcBullet_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_AbcBullet :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(AbcBullet_SOURCES)

clean_AbcBullet:
	-rm -f $(AbcBullet_OBJECTS)

Clean_AbcBullet:
	-rm -f $(AbcBullet_MAKEFILE).bak $(AbcBullet_OBJECTS) $(AbcBullet_PLUGIN)


prebuiltPlugins: $(AbcBullet_PLUGIN)
depend:	 depend_AbcBullet
clean:	 clean_AbcBullet
Clean:	 Clean_AbcBullet

