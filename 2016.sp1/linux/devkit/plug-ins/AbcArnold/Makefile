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
all : thirdPartyPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/AbcArnold
DSTDIR := $(TOP)/AbcArnold

AbcArnold_SOURCES  := $(TOP)/AbcArnold/ArbGeomParams.cpp $(TOP)/AbcArnold/ProcArgs.cpp $(TOP)/AbcArnold/SampleUtil.cpp $(TOP)/AbcArnold/PathUtil.cpp $(TOP)/AbcArnold/ProcMain.cpp $(TOP)/AbcArnold/WriteGeo.cpp
AbcArnold_OBJECTS  := $(TOP)/AbcArnold/ArbGeomParams.o $(TOP)/AbcArnold/ProcArgs.o $(TOP)/AbcArnold/SampleUtil.o $(TOP)/AbcArnold/PathUtil.o $(TOP)/AbcArnold/ProcMain.o $(TOP)/AbcArnold/WriteGeo.o
AbcArnold_PLUGIN   := $(DSTDIR)/AbcArnold.$(EXT)
AbcArnold_MAKEFILE := $(DSTDIR)/Makefile

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

$(AbcArnold_OBJECTS): CFLAGS   := $(CFLAGS)   $(AbcArnold_EXTRA_CFLAGS)
$(AbcArnold_OBJECTS): C++FLAGS := $(C++FLAGS) $(AbcArnold_EXTRA_C++FLAGS)
$(AbcArnold_OBJECTS): INCLUDES := $(INCLUDES) $(AbcArnold_EXTRA_INCLUDES)

depend_AbcArnold:     INCLUDES := $(INCLUDES) $(AbcArnold_EXTRA_INCLUDES)

$(AbcArnold_PLUGIN):  LFLAGS   := $(LFLAGS) $(AbcArnold_EXTRA_LFLAGS) 
$(AbcArnold_PLUGIN):  LIBS     := $(LIBS)   -lai -lAlembicAbcCoreHDF5 -lAlembicAbcGeom -lAlembicAbc -lAlembicAbcCoreAbstract -lAlembicUtil -lhdf5_hl -lhdf5 -lAlembicIex -lAlembicHalf -lz $(AbcArnold_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_AbcArnold clean_AbcArnold Clean_AbcArnold


$(AbcArnold_PLUGIN): $(AbcArnold_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_AbcArnold :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(AbcArnold_SOURCES)

clean_AbcArnold:
	-rm -f $(AbcArnold_OBJECTS)

Clean_AbcArnold:
	-rm -f $(AbcArnold_MAKEFILE).bak $(AbcArnold_OBJECTS) $(AbcArnold_PLUGIN)


thirdPartyPlugins: $(AbcArnold_PLUGIN)
depend:	 depend_AbcArnold
clean:	 clean_AbcArnold
Clean:	 Clean_AbcArnold

