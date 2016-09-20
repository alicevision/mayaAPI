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

SRCDIR := $(TOP)/conditionTest
DSTDIR := $(TOP)/conditionTest

conditionTest_SOURCES  := $(TOP)/conditionTest/conditionTest.cpp
conditionTest_OBJECTS  := $(TOP)/conditionTest/conditionTest.o
conditionTest_PLUGIN   := $(DSTDIR)/conditionTest.$(EXT)
conditionTest_MAKEFILE := $(DSTDIR)/Makefile

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

$(conditionTest_OBJECTS): CFLAGS   := $(CFLAGS)   $(conditionTest_EXTRA_CFLAGS)
$(conditionTest_OBJECTS): C++FLAGS := $(C++FLAGS) $(conditionTest_EXTRA_C++FLAGS)
$(conditionTest_OBJECTS): INCLUDES := $(INCLUDES) $(conditionTest_EXTRA_INCLUDES)

depend_conditionTest:     INCLUDES := $(INCLUDES) $(conditionTest_EXTRA_INCLUDES)

$(conditionTest_PLUGIN):  LFLAGS   := $(LFLAGS) $(conditionTest_EXTRA_LFLAGS) 
$(conditionTest_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(conditionTest_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_conditionTest clean_conditionTest Clean_conditionTest


$(conditionTest_PLUGIN): $(conditionTest_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_conditionTest :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(conditionTest_SOURCES)

clean_conditionTest:
	-rm -f $(conditionTest_OBJECTS)

Clean_conditionTest:
	-rm -f $(conditionTest_MAKEFILE).bak $(conditionTest_OBJECTS) $(conditionTest_PLUGIN)


plugins: $(conditionTest_PLUGIN)
depend:	 depend_conditionTest
clean:	 clean_conditionTest
Clean:	 Clean_conditionTest

