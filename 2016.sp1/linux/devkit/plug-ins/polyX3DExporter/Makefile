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

SRCDIR := $(TOP)/polyX3DExporter
DSTDIR := $(TOP)/polyX3DExporter

polyX3DExporter_SOURCES  := $(TOP)/polyX3DExporter/polyX3DExporter.cpp $(TOP)/polyX3DExporter/polyX3DWriter.cpp $(TOP)/polyX3DExporter/polyExporter.cpp $(TOP)/polyX3DExporter/polyWriter.cpp
polyX3DExporter_OBJECTS  := $(TOP)/polyX3DExporter/polyX3DExporter.o $(TOP)/polyX3DExporter/polyX3DWriter.o $(TOP)/polyX3DExporter/polyExporter.o $(TOP)/polyX3DExporter/polyWriter.o
polyX3DExporter_PLUGIN   := $(DSTDIR)/polyX3DExporter.$(EXT)
polyX3DExporter_MAKEFILE := $(DSTDIR)/Makefile

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

$(polyX3DExporter_OBJECTS): CFLAGS   := $(CFLAGS)   $(polyX3DExporter_EXTRA_CFLAGS)
$(polyX3DExporter_OBJECTS): C++FLAGS := $(C++FLAGS) $(polyX3DExporter_EXTRA_C++FLAGS)
$(polyX3DExporter_OBJECTS): INCLUDES := $(INCLUDES) $(polyX3DExporter_EXTRA_INCLUDES)

depend_polyX3DExporter:     INCLUDES := $(INCLUDES) $(polyX3DExporter_EXTRA_INCLUDES)

$(polyX3DExporter_PLUGIN):  LFLAGS   := $(LFLAGS) $(polyX3DExporter_EXTRA_LFLAGS) 
$(polyX3DExporter_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(polyX3DExporter_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_polyX3DExporter clean_polyX3DExporter Clean_polyX3DExporter


$(polyX3DExporter_PLUGIN): $(polyX3DExporter_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_polyX3DExporter :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(polyX3DExporter_SOURCES)

clean_polyX3DExporter:
	-rm -f $(polyX3DExporter_OBJECTS)

Clean_polyX3DExporter:
	-rm -f $(polyX3DExporter_MAKEFILE).bak $(polyX3DExporter_OBJECTS) $(polyX3DExporter_PLUGIN)


plugins: $(polyX3DExporter_PLUGIN)
depend:	 depend_polyX3DExporter
clean:	 clean_polyX3DExporter
Clean:	 Clean_polyX3DExporter

