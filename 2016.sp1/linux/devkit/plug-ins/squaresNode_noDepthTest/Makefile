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

SRCDIR := $(TOP)/squaresNode_noDepthTest
DSTDIR := $(TOP)/squaresNode_noDepthTest

squaresNode_noDepthTest_SOURCES  := $(TOP)/squaresNode_noDepthTest/squaresNode_noDepthTest.cpp
squaresNode_noDepthTest_OBJECTS  := $(TOP)/squaresNode_noDepthTest/squaresNode_noDepthTest.o
squaresNode_noDepthTest_PLUGIN   := $(DSTDIR)/squaresNode_noDepthTest.$(EXT)
squaresNode_noDepthTest_MAKEFILE := $(DSTDIR)/Makefile

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

$(squaresNode_noDepthTest_OBJECTS): CFLAGS   := $(CFLAGS)   $(squaresNode_noDepthTest_EXTRA_CFLAGS)
$(squaresNode_noDepthTest_OBJECTS): C++FLAGS := $(C++FLAGS) $(squaresNode_noDepthTest_EXTRA_C++FLAGS)
$(squaresNode_noDepthTest_OBJECTS): INCLUDES := $(INCLUDES) $(squaresNode_noDepthTest_EXTRA_INCLUDES)

depend_squaresNode_noDepthTest:     INCLUDES := $(INCLUDES) $(squaresNode_noDepthTest_EXTRA_INCLUDES)

$(squaresNode_noDepthTest_PLUGIN):  LFLAGS   := $(LFLAGS) $(squaresNode_noDepthTest_EXTRA_LFLAGS) 
$(squaresNode_noDepthTest_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lFoundation -lGL $(squaresNode_noDepthTest_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_squaresNode_noDepthTest clean_squaresNode_noDepthTest Clean_squaresNode_noDepthTest


$(squaresNode_noDepthTest_PLUGIN): $(squaresNode_noDepthTest_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_squaresNode_noDepthTest :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(squaresNode_noDepthTest_SOURCES)

clean_squaresNode_noDepthTest:
	-rm -f $(squaresNode_noDepthTest_OBJECTS)

Clean_squaresNode_noDepthTest:
	-rm -f $(squaresNode_noDepthTest_MAKEFILE).bak $(squaresNode_noDepthTest_OBJECTS) $(squaresNode_noDepthTest_PLUGIN)


plugins: $(squaresNode_noDepthTest_PLUGIN)
depend:	 depend_squaresNode_noDepthTest
clean:	 clean_squaresNode_noDepthTest
Clean:	 Clean_squaresNode_noDepthTest

