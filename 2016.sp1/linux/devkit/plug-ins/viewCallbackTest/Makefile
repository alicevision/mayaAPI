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

SRCDIR := $(TOP)/viewCallbackTest
DSTDIR := $(TOP)/viewCallbackTest

viewCallbackTest_SOURCES  := $(TOP)/viewCallbackTest/viewCallbackTest.cpp
viewCallbackTest_OBJECTS  := $(TOP)/viewCallbackTest/viewCallbackTest.o
viewCallbackTest_PLUGIN   := $(DSTDIR)/viewCallbackTest.$(EXT)
viewCallbackTest_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewCallbackTest_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewCallbackTest_EXTRA_CFLAGS)
$(viewCallbackTest_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewCallbackTest_EXTRA_C++FLAGS)
$(viewCallbackTest_OBJECTS): INCLUDES := $(INCLUDES) $(viewCallbackTest_EXTRA_INCLUDES)

depend_viewCallbackTest:     INCLUDES := $(INCLUDES) $(viewCallbackTest_EXTRA_INCLUDES)

$(viewCallbackTest_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewCallbackTest_EXTRA_LFLAGS) 
$(viewCallbackTest_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(viewCallbackTest_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewCallbackTest clean_viewCallbackTest Clean_viewCallbackTest


$(viewCallbackTest_PLUGIN): $(viewCallbackTest_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewCallbackTest :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewCallbackTest_SOURCES)

clean_viewCallbackTest:
	-rm -f $(viewCallbackTest_OBJECTS)

Clean_viewCallbackTest:
	-rm -f $(viewCallbackTest_MAKEFILE).bak $(viewCallbackTest_OBJECTS) $(viewCallbackTest_PLUGIN)


plugins: $(viewCallbackTest_PLUGIN)
depend:	 depend_viewCallbackTest
clean:	 clean_viewCallbackTest
Clean:	 Clean_viewCallbackTest

