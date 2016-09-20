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

SRCDIR := $(TOP)/simpleEvaluationDraw
DSTDIR := $(TOP)/simpleEvaluationDraw

simpleEvaluationDraw_SOURCES  := $(TOP)/simpleEvaluationDraw/simpleEvaluationDraw.cpp
simpleEvaluationDraw_OBJECTS  := $(TOP)/simpleEvaluationDraw/simpleEvaluationDraw.o
simpleEvaluationDraw_PLUGIN   := $(DSTDIR)/simpleEvaluationDraw.$(EXT)
simpleEvaluationDraw_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleEvaluationDraw_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleEvaluationDraw_EXTRA_CFLAGS)
$(simpleEvaluationDraw_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleEvaluationDraw_EXTRA_C++FLAGS)
$(simpleEvaluationDraw_OBJECTS): INCLUDES := $(INCLUDES) $(simpleEvaluationDraw_EXTRA_INCLUDES)

depend_simpleEvaluationDraw:     INCLUDES := $(INCLUDES) $(simpleEvaluationDraw_EXTRA_INCLUDES)

$(simpleEvaluationDraw_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleEvaluationDraw_EXTRA_LFLAGS) 
$(simpleEvaluationDraw_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation $(simpleEvaluationDraw_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleEvaluationDraw clean_simpleEvaluationDraw Clean_simpleEvaluationDraw


$(simpleEvaluationDraw_PLUGIN): $(simpleEvaluationDraw_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleEvaluationDraw :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleEvaluationDraw_SOURCES)

clean_simpleEvaluationDraw:
	-rm -f $(simpleEvaluationDraw_OBJECTS)

Clean_simpleEvaluationDraw:
	-rm -f $(simpleEvaluationDraw_MAKEFILE).bak $(simpleEvaluationDraw_OBJECTS) $(simpleEvaluationDraw_PLUGIN)


plugins: $(simpleEvaluationDraw_PLUGIN)
depend:	 depend_simpleEvaluationDraw
clean:	 clean_simpleEvaluationDraw
Clean:	 Clean_simpleEvaluationDraw

