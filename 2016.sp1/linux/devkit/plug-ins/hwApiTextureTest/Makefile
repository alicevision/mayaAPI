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

SRCDIR := $(TOP)/hwApiTextureTest
DSTDIR := $(TOP)/hwApiTextureTest

hwApiTextureTest_SOURCES  := $(TOP)/hwApiTextureTest/hwApiTextureTest.cpp $(TOP)/hwApiTextureTest/hwApiTextureTestStrings.cpp $(TOP)/hwApiTextureTest/hwRendererHelper.cpp $(TOP)/hwApiTextureTest/hwRendererHelperDX.cpp $(TOP)/hwApiTextureTest/hwRendererHelperGL.cpp
hwApiTextureTest_OBJECTS  := $(TOP)/hwApiTextureTest/hwApiTextureTest.o $(TOP)/hwApiTextureTest/hwApiTextureTestStrings.o $(TOP)/hwApiTextureTest/hwRendererHelper.o $(TOP)/hwApiTextureTest/hwRendererHelperDX.o $(TOP)/hwApiTextureTest/hwRendererHelperGL.o
hwApiTextureTest_PLUGIN   := $(DSTDIR)/hwApiTextureTest.$(EXT)
hwApiTextureTest_MAKEFILE := $(DSTDIR)/Makefile

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

$(hwApiTextureTest_OBJECTS): CFLAGS   := $(CFLAGS)   $(hwApiTextureTest_EXTRA_CFLAGS)
$(hwApiTextureTest_OBJECTS): C++FLAGS := $(C++FLAGS) $(hwApiTextureTest_EXTRA_C++FLAGS)
$(hwApiTextureTest_OBJECTS): INCLUDES := $(INCLUDES) $(hwApiTextureTest_EXTRA_INCLUDES)

depend_hwApiTextureTest:     INCLUDES := $(INCLUDES) $(hwApiTextureTest_EXTRA_INCLUDES)

$(hwApiTextureTest_PLUGIN):  LFLAGS   := $(LFLAGS) $(hwApiTextureTest_EXTRA_LFLAGS) 
$(hwApiTextureTest_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(hwApiTextureTest_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hwApiTextureTest clean_hwApiTextureTest Clean_hwApiTextureTest


$(hwApiTextureTest_PLUGIN): $(hwApiTextureTest_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hwApiTextureTest :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hwApiTextureTest_SOURCES)

clean_hwApiTextureTest:
	-rm -f $(hwApiTextureTest_OBJECTS)

Clean_hwApiTextureTest:
	-rm -f $(hwApiTextureTest_MAKEFILE).bak $(hwApiTextureTest_OBJECTS) $(hwApiTextureTest_PLUGIN)


plugins: $(hwApiTextureTest_PLUGIN)
depend:	 depend_hwApiTextureTest
clean:	 clean_hwApiTextureTest
Clean:	 Clean_hwApiTextureTest

