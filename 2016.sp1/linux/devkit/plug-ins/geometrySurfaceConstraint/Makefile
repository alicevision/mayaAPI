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

SRCDIR := $(TOP)/geometrySurfaceConstraint
DSTDIR := $(TOP)/geometrySurfaceConstraint

geometrySurfaceConstraint_SOURCES  := $(TOP)/geometrySurfaceConstraint/geometrySurfaceConstraint.cpp
geometrySurfaceConstraint_OBJECTS  := $(TOP)/geometrySurfaceConstraint/geometrySurfaceConstraint.o
geometrySurfaceConstraint_PLUGIN   := $(DSTDIR)/geometrySurfaceConstraint.$(EXT)
geometrySurfaceConstraint_MAKEFILE := $(DSTDIR)/Makefile

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

$(geometrySurfaceConstraint_OBJECTS): CFLAGS   := $(CFLAGS)   $(geometrySurfaceConstraint_EXTRA_CFLAGS)
$(geometrySurfaceConstraint_OBJECTS): C++FLAGS := $(C++FLAGS) $(geometrySurfaceConstraint_EXTRA_C++FLAGS)
$(geometrySurfaceConstraint_OBJECTS): INCLUDES := $(INCLUDES) $(geometrySurfaceConstraint_EXTRA_INCLUDES)

depend_geometrySurfaceConstraint:     INCLUDES := $(INCLUDES) $(geometrySurfaceConstraint_EXTRA_INCLUDES)

$(geometrySurfaceConstraint_PLUGIN):  LFLAGS   := $(LFLAGS) $(geometrySurfaceConstraint_EXTRA_LFLAGS) 
$(geometrySurfaceConstraint_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaAnim $(geometrySurfaceConstraint_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_geometrySurfaceConstraint clean_geometrySurfaceConstraint Clean_geometrySurfaceConstraint


$(geometrySurfaceConstraint_PLUGIN): $(geometrySurfaceConstraint_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_geometrySurfaceConstraint :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(geometrySurfaceConstraint_SOURCES)

clean_geometrySurfaceConstraint:
	-rm -f $(geometrySurfaceConstraint_OBJECTS)

Clean_geometrySurfaceConstraint:
	-rm -f $(geometrySurfaceConstraint_MAKEFILE).bak $(geometrySurfaceConstraint_OBJECTS) $(geometrySurfaceConstraint_PLUGIN)


plugins: $(geometrySurfaceConstraint_PLUGIN)
depend:	 depend_geometrySurfaceConstraint
clean:	 clean_geometrySurfaceConstraint
Clean:	 Clean_geometrySurfaceConstraint

