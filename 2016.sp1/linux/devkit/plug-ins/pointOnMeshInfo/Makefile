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

SRCDIR := $(TOP)/pointOnMeshInfo
DSTDIR := $(TOP)/pointOnMeshInfo

pointOnMeshInfo_SOURCES  := $(TOP)/pointOnMeshInfo/getPointAndNormal.cpp $(TOP)/pointOnMeshInfo/pointOnMeshCmd.cpp $(TOP)/pointOnMeshInfo/pointOnMeshInfoNode.cpp $(TOP)/pointOnMeshInfo/pointOnMeshInfoPlugin.cpp
pointOnMeshInfo_OBJECTS  := $(TOP)/pointOnMeshInfo/getPointAndNormal.o $(TOP)/pointOnMeshInfo/pointOnMeshCmd.o $(TOP)/pointOnMeshInfo/pointOnMeshInfoNode.o $(TOP)/pointOnMeshInfo/pointOnMeshInfoPlugin.o
pointOnMeshInfo_PLUGIN   := $(DSTDIR)/pointOnMeshInfo.$(EXT)
pointOnMeshInfo_MAKEFILE := $(DSTDIR)/Makefile

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

$(pointOnMeshInfo_OBJECTS): CFLAGS   := $(CFLAGS)   $(pointOnMeshInfo_EXTRA_CFLAGS)
$(pointOnMeshInfo_OBJECTS): C++FLAGS := $(C++FLAGS) $(pointOnMeshInfo_EXTRA_C++FLAGS)
$(pointOnMeshInfo_OBJECTS): INCLUDES := $(INCLUDES) $(pointOnMeshInfo_EXTRA_INCLUDES)

depend_pointOnMeshInfo:     INCLUDES := $(INCLUDES) $(pointOnMeshInfo_EXTRA_INCLUDES)

$(pointOnMeshInfo_PLUGIN):  LFLAGS   := $(LFLAGS) $(pointOnMeshInfo_EXTRA_LFLAGS) 
$(pointOnMeshInfo_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL $(pointOnMeshInfo_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_pointOnMeshInfo clean_pointOnMeshInfo Clean_pointOnMeshInfo


$(pointOnMeshInfo_PLUGIN): $(pointOnMeshInfo_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_pointOnMeshInfo :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(pointOnMeshInfo_SOURCES)

clean_pointOnMeshInfo:
	-rm -f $(pointOnMeshInfo_OBJECTS)

Clean_pointOnMeshInfo:
	-rm -f $(pointOnMeshInfo_MAKEFILE).bak $(pointOnMeshInfo_OBJECTS) $(pointOnMeshInfo_PLUGIN)


plugins: $(pointOnMeshInfo_PLUGIN)
depend:	 depend_pointOnMeshInfo
clean:	 clean_pointOnMeshInfo
Clean:	 Clean_pointOnMeshInfo

