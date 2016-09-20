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

SRCDIR := $(TOP)/apiSimpleShape
DSTDIR := $(TOP)/apiSimpleShape

apiSimpleShape_SOURCES  := $(TOP)/apiSimpleShape/apiSimpleShapeUI.cpp $(TOP)/apiSimpleShape/apiSimpleShape.cpp $(TOP)/apiSimpleShape/apiSimpleShapeIterator.cpp
apiSimpleShape_OBJECTS  := $(TOP)/apiSimpleShape/apiSimpleShapeUI.o $(TOP)/apiSimpleShape/apiSimpleShape.o $(TOP)/apiSimpleShape/apiSimpleShapeIterator.o
apiSimpleShape_PLUGIN   := $(DSTDIR)/apiSimpleShape.$(EXT)
apiSimpleShape_MAKEFILE := $(DSTDIR)/Makefile

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

$(apiSimpleShape_OBJECTS): CFLAGS   := $(CFLAGS)   $(apiSimpleShape_EXTRA_CFLAGS)
$(apiSimpleShape_OBJECTS): C++FLAGS := $(C++FLAGS) $(apiSimpleShape_EXTRA_C++FLAGS)
$(apiSimpleShape_OBJECTS): INCLUDES := $(INCLUDES) $(apiSimpleShape_EXTRA_INCLUDES)

depend_apiSimpleShape:     INCLUDES := $(INCLUDES) $(apiSimpleShape_EXTRA_INCLUDES)

$(apiSimpleShape_PLUGIN):  LFLAGS   := $(LFLAGS) $(apiSimpleShape_EXTRA_LFLAGS) 
$(apiSimpleShape_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL -lGLU $(apiSimpleShape_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_apiSimpleShape clean_apiSimpleShape Clean_apiSimpleShape


$(apiSimpleShape_PLUGIN): $(apiSimpleShape_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_apiSimpleShape :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(apiSimpleShape_SOURCES)

clean_apiSimpleShape:
	-rm -f $(apiSimpleShape_OBJECTS)

Clean_apiSimpleShape:
	-rm -f $(apiSimpleShape_MAKEFILE).bak $(apiSimpleShape_OBJECTS) $(apiSimpleShape_PLUGIN)


plugins: $(apiSimpleShape_PLUGIN)
depend:	 depend_apiSimpleShape
clean:	 clean_apiSimpleShape
Clean:	 Clean_apiSimpleShape

