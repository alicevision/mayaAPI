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

SRCDIR := $(TOP)/geometryCacheConverter
DSTDIR := $(TOP)/geometryCacheConverter

geometryCacheConverter_SOURCES  := $(TOP)/geometryCacheConverter/geometryCacheBlockBase.cpp $(TOP)/geometryCacheConverter/geometryCacheBlockDVAData.cpp $(TOP)/geometryCacheConverter/geometryCacheBlockFVAData.cpp $(TOP)/geometryCacheConverter/geometryCacheBlockIntData.cpp $(TOP)/geometryCacheConverter/geometryCacheBlockStringData.cpp $(TOP)/geometryCacheConverter/geometryCacheConverter.cpp $(TOP)/geometryCacheConverter/geometryCacheFile.cpp
geometryCacheConverter_OBJECTS  := $(TOP)/geometryCacheConverter/geometryCacheBlockBase.o $(TOP)/geometryCacheConverter/geometryCacheBlockDVAData.o $(TOP)/geometryCacheConverter/geometryCacheBlockFVAData.o $(TOP)/geometryCacheConverter/geometryCacheBlockIntData.o $(TOP)/geometryCacheConverter/geometryCacheBlockStringData.o $(TOP)/geometryCacheConverter/geometryCacheConverter.o $(TOP)/geometryCacheConverter/geometryCacheFile.o
geometryCacheConverter_PLUGIN   := $(DSTDIR)/geometryCacheConverter.$(EXT)
geometryCacheConverter_MAKEFILE := $(DSTDIR)/Makefile

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

$(geometryCacheConverter_OBJECTS): CFLAGS   := $(CFLAGS)   $(geometryCacheConverter_EXTRA_CFLAGS)
$(geometryCacheConverter_OBJECTS): C++FLAGS := $(C++FLAGS) $(geometryCacheConverter_EXTRA_C++FLAGS)
$(geometryCacheConverter_OBJECTS): INCLUDES := $(INCLUDES) $(geometryCacheConverter_EXTRA_INCLUDES)

depend_geometryCacheConverter:     INCLUDES := $(INCLUDES) $(geometryCacheConverter_EXTRA_INCLUDES)

$(geometryCacheConverter_PLUGIN):  LFLAGS   := $(LFLAGS) $(geometryCacheConverter_EXTRA_LFLAGS) 
$(geometryCacheConverter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(geometryCacheConverter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_geometryCacheConverter clean_geometryCacheConverter Clean_geometryCacheConverter


$(geometryCacheConverter_PLUGIN): $(geometryCacheConverter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_geometryCacheConverter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(geometryCacheConverter_SOURCES)

clean_geometryCacheConverter:
	-rm -f $(geometryCacheConverter_OBJECTS)

Clean_geometryCacheConverter:
	-rm -f $(geometryCacheConverter_MAKEFILE).bak $(geometryCacheConverter_OBJECTS) $(geometryCacheConverter_PLUGIN)


plugins: $(geometryCacheConverter_PLUGIN)
depend:	 depend_geometryCacheConverter
clean:	 clean_geometryCacheConverter
Clean:	 Clean_geometryCacheConverter

