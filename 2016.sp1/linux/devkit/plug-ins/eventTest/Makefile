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

SRCDIR := $(TOP)/eventTest
DSTDIR := $(TOP)/eventTest

eventTest_SOURCES  := $(TOP)/eventTest/eventTest.cpp
eventTest_OBJECTS  := $(TOP)/eventTest/eventTest.o
eventTest_PLUGIN   := $(DSTDIR)/eventTest.$(EXT)
eventTest_MAKEFILE := $(DSTDIR)/Makefile

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

$(eventTest_OBJECTS): CFLAGS   := $(CFLAGS)   $(eventTest_EXTRA_CFLAGS)
$(eventTest_OBJECTS): C++FLAGS := $(C++FLAGS) $(eventTest_EXTRA_C++FLAGS)
$(eventTest_OBJECTS): INCLUDES := $(INCLUDES) $(eventTest_EXTRA_INCLUDES)

depend_eventTest:     INCLUDES := $(INCLUDES) $(eventTest_EXTRA_INCLUDES)

$(eventTest_PLUGIN):  LFLAGS   := $(LFLAGS) $(eventTest_EXTRA_LFLAGS) 
$(eventTest_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(eventTest_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_eventTest clean_eventTest Clean_eventTest


$(eventTest_PLUGIN): $(eventTest_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_eventTest :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(eventTest_SOURCES)

clean_eventTest:
	-rm -f $(eventTest_OBJECTS)

Clean_eventTest:
	-rm -f $(eventTest_MAKEFILE).bak $(eventTest_OBJECTS) $(eventTest_PLUGIN)


plugins: $(eventTest_PLUGIN)
depend:	 depend_eventTest
clean:	 clean_eventTest
Clean:	 Clean_eventTest

