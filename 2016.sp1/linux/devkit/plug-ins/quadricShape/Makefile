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

SRCDIR := $(TOP)/quadricShape
DSTDIR := $(TOP)/quadricShape

quadricShape_SOURCES  := $(TOP)/quadricShape/quadricShape.cpp
quadricShape_OBJECTS  := $(TOP)/quadricShape/quadricShape.o
quadricShape_PLUGIN   := $(DSTDIR)/quadricShape.$(EXT)
quadricShape_MAKEFILE := $(DSTDIR)/Makefile

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

$(quadricShape_OBJECTS): CFLAGS   := $(CFLAGS)   $(quadricShape_EXTRA_CFLAGS)
$(quadricShape_OBJECTS): C++FLAGS := $(C++FLAGS) $(quadricShape_EXTRA_C++FLAGS)
$(quadricShape_OBJECTS): INCLUDES := $(INCLUDES) $(quadricShape_EXTRA_INCLUDES)

depend_quadricShape:     INCLUDES := $(INCLUDES) $(quadricShape_EXTRA_INCLUDES)

$(quadricShape_PLUGIN):  LFLAGS   := $(LFLAGS) $(quadricShape_EXTRA_LFLAGS) 
$(quadricShape_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL -lGLU $(quadricShape_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_quadricShape clean_quadricShape Clean_quadricShape


$(quadricShape_PLUGIN): $(quadricShape_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_quadricShape :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(quadricShape_SOURCES)

clean_quadricShape:
	-rm -f $(quadricShape_OBJECTS)

Clean_quadricShape:
	-rm -f $(quadricShape_MAKEFILE).bak $(quadricShape_OBJECTS) $(quadricShape_PLUGIN)


plugins: $(quadricShape_PLUGIN)
depend:	 depend_quadricShape
clean:	 clean_quadricShape
Clean:	 Clean_quadricShape

