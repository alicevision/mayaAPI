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

SRCDIR := $(TOP)/apiMeshShape
DSTDIR := $(TOP)/apiMeshShape

apiMeshShape_SOURCES  := $(TOP)/apiMeshShape/apiMeshShapeUI.cpp $(TOP)/apiMeshShape/apiMeshShape.cpp $(TOP)/apiMeshShape/apiMeshIterator.cpp $(TOP)/apiMeshShape/apiMeshGeom.cpp $(TOP)/apiMeshShape/apiMeshData.cpp $(TOP)/apiMeshShape/apiMeshCreator.cpp $(TOP)/apiMeshShape/apiMeshGeometryOverride.cpp $(TOP)/apiMeshShape/apiMeshSubSceneOverride.cpp
apiMeshShape_OBJECTS  := $(TOP)/apiMeshShape/apiMeshShapeUI.o $(TOP)/apiMeshShape/apiMeshShape.o $(TOP)/apiMeshShape/apiMeshIterator.o $(TOP)/apiMeshShape/apiMeshGeom.o $(TOP)/apiMeshShape/apiMeshData.o $(TOP)/apiMeshShape/apiMeshCreator.o $(TOP)/apiMeshShape/apiMeshGeometryOverride.o $(TOP)/apiMeshShape/apiMeshSubSceneOverride.o
apiMeshShape_PLUGIN   := $(DSTDIR)/apiMeshShape.$(EXT)
apiMeshShape_MAKEFILE := $(DSTDIR)/Makefile

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

$(apiMeshShape_OBJECTS): CFLAGS   := $(CFLAGS)   $(apiMeshShape_EXTRA_CFLAGS)
$(apiMeshShape_OBJECTS): C++FLAGS := $(C++FLAGS) $(apiMeshShape_EXTRA_C++FLAGS)
$(apiMeshShape_OBJECTS): INCLUDES := $(INCLUDES) $(apiMeshShape_EXTRA_INCLUDES)

depend_apiMeshShape:     INCLUDES := $(INCLUDES) $(apiMeshShape_EXTRA_INCLUDES)

$(apiMeshShape_PLUGIN):  LFLAGS   := $(LFLAGS) $(apiMeshShape_EXTRA_LFLAGS) 
$(apiMeshShape_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(apiMeshShape_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_apiMeshShape clean_apiMeshShape Clean_apiMeshShape


$(apiMeshShape_PLUGIN): $(apiMeshShape_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_apiMeshShape :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(apiMeshShape_SOURCES)

clean_apiMeshShape:
	-rm -f $(apiMeshShape_OBJECTS)

Clean_apiMeshShape:
	-rm -f $(apiMeshShape_MAKEFILE).bak $(apiMeshShape_OBJECTS) $(apiMeshShape_PLUGIN)


plugins: $(apiMeshShape_PLUGIN)
depend:	 depend_apiMeshShape
clean:	 clean_apiMeshShape
Clean:	 Clean_apiMeshShape

