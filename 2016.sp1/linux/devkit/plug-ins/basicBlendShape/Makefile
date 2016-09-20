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

SRCDIR := $(TOP)/basicBlendShape
DSTDIR := $(TOP)/basicBlendShape

basicBlendShape_SOURCES  := $(TOP)/basicBlendShape/basicBlendShape.cpp
basicBlendShape_OBJECTS  := $(TOP)/basicBlendShape/basicBlendShape.o
basicBlendShape_PLUGIN   := $(DSTDIR)/basicBlendShape.$(EXT)
basicBlendShape_MAKEFILE := $(DSTDIR)/Makefile

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

$(basicBlendShape_OBJECTS): CFLAGS   := $(CFLAGS)   $(basicBlendShape_EXTRA_CFLAGS)
$(basicBlendShape_OBJECTS): C++FLAGS := $(C++FLAGS) $(basicBlendShape_EXTRA_C++FLAGS)
$(basicBlendShape_OBJECTS): INCLUDES := $(INCLUDES) $(basicBlendShape_EXTRA_INCLUDES)

depend_basicBlendShape:     INCLUDES := $(INCLUDES) $(basicBlendShape_EXTRA_INCLUDES)

$(basicBlendShape_PLUGIN):  LFLAGS   := $(LFLAGS) $(basicBlendShape_EXTRA_LFLAGS) 
$(basicBlendShape_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(basicBlendShape_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_basicBlendShape clean_basicBlendShape Clean_basicBlendShape


$(basicBlendShape_PLUGIN): $(basicBlendShape_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_basicBlendShape :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(basicBlendShape_SOURCES)

clean_basicBlendShape:
	-rm -f $(basicBlendShape_OBJECTS)

Clean_basicBlendShape:
	-rm -f $(basicBlendShape_MAKEFILE).bak $(basicBlendShape_OBJECTS) $(basicBlendShape_PLUGIN)


plugins: $(basicBlendShape_PLUGIN)
depend:	 depend_basicBlendShape
clean:	 clean_basicBlendShape
Clean:	 Clean_basicBlendShape

