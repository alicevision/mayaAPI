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

SRCDIR := $(TOP)/squareScaleManipContext
DSTDIR := $(TOP)/squareScaleManipContext

squareScaleManipContext_SOURCES  := $(TOP)/squareScaleManipContext/squareScaleManipContext.cpp $(TOP)/squareScaleManipContext/manipulatorMath.cpp
squareScaleManipContext_OBJECTS  := $(TOP)/squareScaleManipContext/squareScaleManipContext.o $(TOP)/squareScaleManipContext/manipulatorMath.o
squareScaleManipContext_PLUGIN   := $(DSTDIR)/squareScaleManipContext.$(EXT)
squareScaleManipContext_MAKEFILE := $(DSTDIR)/Makefile

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

$(squareScaleManipContext_OBJECTS): CFLAGS   := $(CFLAGS)   $(squareScaleManipContext_EXTRA_CFLAGS)
$(squareScaleManipContext_OBJECTS): C++FLAGS := $(C++FLAGS) $(squareScaleManipContext_EXTRA_C++FLAGS)
$(squareScaleManipContext_OBJECTS): INCLUDES := $(INCLUDES) $(squareScaleManipContext_EXTRA_INCLUDES)

depend_squareScaleManipContext:     INCLUDES := $(INCLUDES) $(squareScaleManipContext_EXTRA_INCLUDES)

$(squareScaleManipContext_PLUGIN):  LFLAGS   := $(LFLAGS) $(squareScaleManipContext_EXTRA_LFLAGS) 
$(squareScaleManipContext_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaUI -lOpenMayaRender $(squareScaleManipContext_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_squareScaleManipContext clean_squareScaleManipContext Clean_squareScaleManipContext


$(squareScaleManipContext_PLUGIN): $(squareScaleManipContext_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_squareScaleManipContext :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(squareScaleManipContext_SOURCES)

clean_squareScaleManipContext:
	-rm -f $(squareScaleManipContext_OBJECTS)

Clean_squareScaleManipContext:
	-rm -f $(squareScaleManipContext_MAKEFILE).bak $(squareScaleManipContext_OBJECTS) $(squareScaleManipContext_PLUGIN)


plugins: $(squareScaleManipContext_PLUGIN)
depend:	 depend_squareScaleManipContext
clean:	 clean_squareScaleManipContext
Clean:	 Clean_squareScaleManipContext

