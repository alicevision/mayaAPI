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
all : prebuiltPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/cleanPerFaceAssignment
DSTDIR := $(TOP)/cleanPerFaceAssignment

cleanPerFaceAssignment_SOURCES  := $(TOP)/cleanPerFaceAssignment/cleanPerFaceAssignmentCmd.cpp $(TOP)/cleanPerFaceAssignment/cleanPerFaceAssignmentPluginMain.cpp
cleanPerFaceAssignment_OBJECTS  := $(TOP)/cleanPerFaceAssignment/cleanPerFaceAssignmentCmd.o $(TOP)/cleanPerFaceAssignment/cleanPerFaceAssignmentPluginMain.o
cleanPerFaceAssignment_PLUGIN   := $(DSTDIR)/cleanPerFaceAssignment.$(EXT)
cleanPerFaceAssignment_MAKEFILE := $(DSTDIR)/Makefile

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

$(cleanPerFaceAssignment_OBJECTS): CFLAGS   := $(CFLAGS)   $(cleanPerFaceAssignment_EXTRA_CFLAGS)
$(cleanPerFaceAssignment_OBJECTS): C++FLAGS := $(C++FLAGS) $(cleanPerFaceAssignment_EXTRA_C++FLAGS)
$(cleanPerFaceAssignment_OBJECTS): INCLUDES := $(INCLUDES) $(cleanPerFaceAssignment_EXTRA_INCLUDES)

depend_cleanPerFaceAssignment:     INCLUDES := $(INCLUDES) $(cleanPerFaceAssignment_EXTRA_INCLUDES)

$(cleanPerFaceAssignment_PLUGIN):  LFLAGS   := $(LFLAGS) $(cleanPerFaceAssignment_EXTRA_LFLAGS) 
$(cleanPerFaceAssignment_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(cleanPerFaceAssignment_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cleanPerFaceAssignment clean_cleanPerFaceAssignment Clean_cleanPerFaceAssignment


$(cleanPerFaceAssignment_PLUGIN): $(cleanPerFaceAssignment_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cleanPerFaceAssignment :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cleanPerFaceAssignment_SOURCES)

clean_cleanPerFaceAssignment:
	-rm -f $(cleanPerFaceAssignment_OBJECTS)

Clean_cleanPerFaceAssignment:
	-rm -f $(cleanPerFaceAssignment_MAKEFILE).bak $(cleanPerFaceAssignment_OBJECTS) $(cleanPerFaceAssignment_PLUGIN)


prebuiltPlugins: $(cleanPerFaceAssignment_PLUGIN)
depend:	 depend_cleanPerFaceAssignment
clean:	 clean_cleanPerFaceAssignment
Clean:	 Clean_cleanPerFaceAssignment

