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

SRCDIR := $(TOP)/idleTest
DSTDIR := $(TOP)/idleTest

idleTest_SOURCES  := $(TOP)/idleTest/idleTest.cpp
idleTest_OBJECTS  := $(TOP)/idleTest/idleTest.o
idleTest_PLUGIN   := $(DSTDIR)/idleTest.$(EXT)
idleTest_MAKEFILE := $(DSTDIR)/Makefile

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

$(idleTest_OBJECTS): CFLAGS   := $(CFLAGS)   $(idleTest_EXTRA_CFLAGS)
$(idleTest_OBJECTS): C++FLAGS := $(C++FLAGS) $(idleTest_EXTRA_C++FLAGS)
$(idleTest_OBJECTS): INCLUDES := $(INCLUDES) $(idleTest_EXTRA_INCLUDES)

depend_idleTest:     INCLUDES := $(INCLUDES) $(idleTest_EXTRA_INCLUDES)

$(idleTest_PLUGIN):  LFLAGS   := $(LFLAGS) $(idleTest_EXTRA_LFLAGS) 
$(idleTest_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(idleTest_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_idleTest clean_idleTest Clean_idleTest


$(idleTest_PLUGIN): $(idleTest_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_idleTest :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(idleTest_SOURCES)

clean_idleTest:
	-rm -f $(idleTest_OBJECTS)

Clean_idleTest:
	-rm -f $(idleTest_MAKEFILE).bak $(idleTest_OBJECTS) $(idleTest_PLUGIN)


plugins: $(idleTest_PLUGIN)
depend:	 depend_idleTest
clean:	 clean_idleTest
Clean:	 Clean_idleTest

