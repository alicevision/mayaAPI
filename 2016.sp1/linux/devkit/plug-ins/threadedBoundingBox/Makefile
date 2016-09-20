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

SRCDIR := $(TOP)/threadedBoundingBox
DSTDIR := $(TOP)/threadedBoundingBox

threadedBoundingBox_SOURCES  := $(TOP)/threadedBoundingBox/threadedBoundingBox.cpp
threadedBoundingBox_OBJECTS  := $(TOP)/threadedBoundingBox/threadedBoundingBox.o
threadedBoundingBox_PLUGIN   := $(DSTDIR)/threadedBoundingBox.$(EXT)
threadedBoundingBox_MAKEFILE := $(DSTDIR)/Makefile

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

$(threadedBoundingBox_OBJECTS): CFLAGS   := $(CFLAGS)   $(threadedBoundingBox_EXTRA_CFLAGS)
$(threadedBoundingBox_OBJECTS): C++FLAGS := $(C++FLAGS) $(threadedBoundingBox_EXTRA_C++FLAGS)
$(threadedBoundingBox_OBJECTS): INCLUDES := $(INCLUDES) $(threadedBoundingBox_EXTRA_INCLUDES)

depend_threadedBoundingBox:     INCLUDES := $(INCLUDES) $(threadedBoundingBox_EXTRA_INCLUDES)

$(threadedBoundingBox_PLUGIN):  LFLAGS   := $(LFLAGS) $(threadedBoundingBox_EXTRA_LFLAGS) 
$(threadedBoundingBox_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(threadedBoundingBox_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_threadedBoundingBox clean_threadedBoundingBox Clean_threadedBoundingBox


$(threadedBoundingBox_PLUGIN): $(threadedBoundingBox_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_threadedBoundingBox :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(threadedBoundingBox_SOURCES)

clean_threadedBoundingBox:
	-rm -f $(threadedBoundingBox_OBJECTS)

Clean_threadedBoundingBox:
	-rm -f $(threadedBoundingBox_MAKEFILE).bak $(threadedBoundingBox_OBJECTS) $(threadedBoundingBox_PLUGIN)


plugins: $(threadedBoundingBox_PLUGIN)
depend:	 depend_threadedBoundingBox
clean:	 clean_threadedBoundingBox
Clean:	 Clean_threadedBoundingBox

