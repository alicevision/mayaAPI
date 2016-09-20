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

SRCDIR := $(TOP)/exampleCameraSetView
DSTDIR := $(TOP)/exampleCameraSetView

exampleCameraSetView_SOURCES  := $(TOP)/exampleCameraSetView/exampleCameraSetViewMain.cpp $(TOP)/exampleCameraSetView/exampleCameraSetView.cpp $(TOP)/exampleCameraSetView/exampleCameraSetViewCmd.cpp
exampleCameraSetView_OBJECTS  := $(TOP)/exampleCameraSetView/exampleCameraSetViewMain.o $(TOP)/exampleCameraSetView/exampleCameraSetView.o $(TOP)/exampleCameraSetView/exampleCameraSetViewCmd.o
exampleCameraSetView_PLUGIN   := $(DSTDIR)/exampleCameraSetView.$(EXT)
exampleCameraSetView_MAKEFILE := $(DSTDIR)/Makefile

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

$(exampleCameraSetView_OBJECTS): CFLAGS   := $(CFLAGS)   $(exampleCameraSetView_EXTRA_CFLAGS)
$(exampleCameraSetView_OBJECTS): C++FLAGS := $(C++FLAGS) $(exampleCameraSetView_EXTRA_C++FLAGS)
$(exampleCameraSetView_OBJECTS): INCLUDES := $(INCLUDES) $(exampleCameraSetView_EXTRA_INCLUDES)

depend_exampleCameraSetView:     INCLUDES := $(INCLUDES) $(exampleCameraSetView_EXTRA_INCLUDES)

$(exampleCameraSetView_PLUGIN):  LFLAGS   := $(LFLAGS) $(exampleCameraSetView_EXTRA_LFLAGS) 
$(exampleCameraSetView_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(exampleCameraSetView_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_exampleCameraSetView clean_exampleCameraSetView Clean_exampleCameraSetView


$(exampleCameraSetView_PLUGIN): $(exampleCameraSetView_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_exampleCameraSetView :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(exampleCameraSetView_SOURCES)

clean_exampleCameraSetView:
	-rm -f $(exampleCameraSetView_OBJECTS)

Clean_exampleCameraSetView:
	-rm -f $(exampleCameraSetView_MAKEFILE).bak $(exampleCameraSetView_OBJECTS) $(exampleCameraSetView_PLUGIN)


plugins: $(exampleCameraSetView_PLUGIN)
depend:	 depend_exampleCameraSetView
clean:	 clean_exampleCameraSetView
Clean:	 Clean_exampleCameraSetView

