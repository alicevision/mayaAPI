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

SRCDIR := $(TOP)/threadingLockTests
DSTDIR := $(TOP)/threadingLockTests

threadingLockTests_SOURCES  := $(TOP)/threadingLockTests/threadingLockTests.cpp
threadingLockTests_OBJECTS  := $(TOP)/threadingLockTests/threadingLockTests.o
threadingLockTests_PLUGIN   := $(DSTDIR)/threadingLockTests.$(EXT)
threadingLockTests_MAKEFILE := $(DSTDIR)/Makefile

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

$(threadingLockTests_OBJECTS): CFLAGS   := $(CFLAGS)   $(threadingLockTests_EXTRA_CFLAGS)
$(threadingLockTests_OBJECTS): C++FLAGS := $(C++FLAGS) $(threadingLockTests_EXTRA_C++FLAGS)
$(threadingLockTests_OBJECTS): INCLUDES := $(INCLUDES) $(threadingLockTests_EXTRA_INCLUDES)

depend_threadingLockTests:     INCLUDES := $(INCLUDES) $(threadingLockTests_EXTRA_INCLUDES)

$(threadingLockTests_PLUGIN):  LFLAGS   := $(LFLAGS) $(threadingLockTests_EXTRA_LFLAGS) 
$(threadingLockTests_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(threadingLockTests_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_threadingLockTests clean_threadingLockTests Clean_threadingLockTests


$(threadingLockTests_PLUGIN): $(threadingLockTests_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_threadingLockTests :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(threadingLockTests_SOURCES)

clean_threadingLockTests:
	-rm -f $(threadingLockTests_OBJECTS)

Clean_threadingLockTests:
	-rm -f $(threadingLockTests_MAKEFILE).bak $(threadingLockTests_OBJECTS) $(threadingLockTests_PLUGIN)


plugins: $(threadingLockTests_PLUGIN)
depend:	 depend_threadingLockTests
clean:	 clean_threadingLockTests
Clean:	 Clean_threadingLockTests

