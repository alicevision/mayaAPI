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

SRCDIR := $(TOP)/polyRawExporter
DSTDIR := $(TOP)/polyRawExporter

polyRawExporter_SOURCES  := $(TOP)/polyRawExporter/polyExporter.cpp $(TOP)/polyRawExporter/polyWriter.cpp $(TOP)/polyRawExporter/polyRawExporter.cpp $(TOP)/polyRawExporter/polyRawWriter.cpp
polyRawExporter_OBJECTS  := $(TOP)/polyRawExporter/polyExporter.o $(TOP)/polyRawExporter/polyWriter.o $(TOP)/polyRawExporter/polyRawExporter.o $(TOP)/polyRawExporter/polyRawWriter.o
polyRawExporter_PLUGIN   := $(DSTDIR)/polyRawExporter.$(EXT)
polyRawExporter_MAKEFILE := $(DSTDIR)/Makefile

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

$(polyRawExporter_OBJECTS): CFLAGS   := $(CFLAGS)   $(polyRawExporter_EXTRA_CFLAGS)
$(polyRawExporter_OBJECTS): C++FLAGS := $(C++FLAGS) $(polyRawExporter_EXTRA_C++FLAGS)
$(polyRawExporter_OBJECTS): INCLUDES := $(INCLUDES) $(polyRawExporter_EXTRA_INCLUDES)

depend_polyRawExporter:     INCLUDES := $(INCLUDES) $(polyRawExporter_EXTRA_INCLUDES)

$(polyRawExporter_PLUGIN):  LFLAGS   := $(LFLAGS) $(polyRawExporter_EXTRA_LFLAGS) 
$(polyRawExporter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(polyRawExporter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_polyRawExporter clean_polyRawExporter Clean_polyRawExporter


$(polyRawExporter_PLUGIN): $(polyRawExporter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_polyRawExporter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(polyRawExporter_SOURCES)

clean_polyRawExporter:
	-rm -f $(polyRawExporter_OBJECTS)

Clean_polyRawExporter:
	-rm -f $(polyRawExporter_MAKEFILE).bak $(polyRawExporter_OBJECTS) $(polyRawExporter_PLUGIN)


plugins: $(polyRawExporter_PLUGIN)
depend:	 depend_polyRawExporter
clean:	 clean_polyRawExporter
Clean:	 Clean_polyRawExporter

