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

SRCDIR := $(TOP)/hairCollisionSolver
DSTDIR := $(TOP)/hairCollisionSolver

hairCollisionSolver_SOURCES  := $(TOP)/hairCollisionSolver/hairCollisionSolver.cpp
hairCollisionSolver_OBJECTS  := $(TOP)/hairCollisionSolver/hairCollisionSolver.o
hairCollisionSolver_PLUGIN   := $(DSTDIR)/hairCollisionSolver.$(EXT)
hairCollisionSolver_MAKEFILE := $(DSTDIR)/Makefile

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

$(hairCollisionSolver_OBJECTS): CFLAGS   := $(CFLAGS)   $(hairCollisionSolver_EXTRA_CFLAGS)
$(hairCollisionSolver_OBJECTS): C++FLAGS := $(C++FLAGS) $(hairCollisionSolver_EXTRA_C++FLAGS)
$(hairCollisionSolver_OBJECTS): INCLUDES := $(INCLUDES) $(hairCollisionSolver_EXTRA_INCLUDES)

depend_hairCollisionSolver:     INCLUDES := $(INCLUDES) $(hairCollisionSolver_EXTRA_INCLUDES)

$(hairCollisionSolver_PLUGIN):  LFLAGS   := $(LFLAGS) $(hairCollisionSolver_EXTRA_LFLAGS) 
$(hairCollisionSolver_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaFX -lOpenMaya -lFoundation $(hairCollisionSolver_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_hairCollisionSolver clean_hairCollisionSolver Clean_hairCollisionSolver


$(hairCollisionSolver_PLUGIN): $(hairCollisionSolver_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_hairCollisionSolver :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(hairCollisionSolver_SOURCES)

clean_hairCollisionSolver:
	-rm -f $(hairCollisionSolver_OBJECTS)

Clean_hairCollisionSolver:
	-rm -f $(hairCollisionSolver_MAKEFILE).bak $(hairCollisionSolver_OBJECTS) $(hairCollisionSolver_PLUGIN)


plugins: $(hairCollisionSolver_PLUGIN)
depend:	 depend_hairCollisionSolver
clean:	 clean_hairCollisionSolver
Clean:	 Clean_hairCollisionSolver

