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

SRCDIR := $(TOP)/geometryReplicator
DSTDIR := $(TOP)/geometryReplicator

geometryReplicator_SOURCES  := $(TOP)/geometryReplicator/geometryReplicator.cpp
geometryReplicator_OBJECTS  := $(TOP)/geometryReplicator/geometryReplicator.o
geometryReplicator_PLUGIN   := $(DSTDIR)/geometryReplicator.$(EXT)
geometryReplicator_MAKEFILE := $(DSTDIR)/Makefile

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

$(geometryReplicator_OBJECTS): CFLAGS   := $(CFLAGS)   $(geometryReplicator_EXTRA_CFLAGS)
$(geometryReplicator_OBJECTS): C++FLAGS := $(C++FLAGS) $(geometryReplicator_EXTRA_C++FLAGS)
$(geometryReplicator_OBJECTS): INCLUDES := $(INCLUDES) $(geometryReplicator_EXTRA_INCLUDES)

depend_geometryReplicator:     INCLUDES := $(INCLUDES) $(geometryReplicator_EXTRA_INCLUDES)

$(geometryReplicator_PLUGIN):  LFLAGS   := $(LFLAGS) $(geometryReplicator_EXTRA_LFLAGS) 
$(geometryReplicator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation $(geometryReplicator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_geometryReplicator clean_geometryReplicator Clean_geometryReplicator


$(geometryReplicator_PLUGIN): $(geometryReplicator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_geometryReplicator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(geometryReplicator_SOURCES)

clean_geometryReplicator:
	-rm -f $(geometryReplicator_OBJECTS)

Clean_geometryReplicator:
	-rm -f $(geometryReplicator_MAKEFILE).bak $(geometryReplicator_OBJECTS) $(geometryReplicator_PLUGIN)


plugins: $(geometryReplicator_PLUGIN)
depend:	 depend_geometryReplicator
clean:	 clean_geometryReplicator
Clean:	 Clean_geometryReplicator

