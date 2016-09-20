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

SRCDIR := $(TOP)/backfillShader
DSTDIR := $(TOP)/backfillShader

backfillShader_SOURCES  := $(TOP)/backfillShader/backfillShader.cpp
backfillShader_OBJECTS  := $(TOP)/backfillShader/backfillShader.o
backfillShader_PLUGIN   := $(DSTDIR)/backfillShader.$(EXT)
backfillShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(backfillShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(backfillShader_EXTRA_CFLAGS)
$(backfillShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(backfillShader_EXTRA_C++FLAGS)
$(backfillShader_OBJECTS): INCLUDES := $(INCLUDES) $(backfillShader_EXTRA_INCLUDES)

depend_backfillShader:     INCLUDES := $(INCLUDES) $(backfillShader_EXTRA_INCLUDES)

$(backfillShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(backfillShader_EXTRA_LFLAGS) 
$(backfillShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(backfillShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_backfillShader clean_backfillShader Clean_backfillShader


$(backfillShader_PLUGIN): $(backfillShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_backfillShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(backfillShader_SOURCES)

clean_backfillShader:
	-rm -f $(backfillShader_OBJECTS)

Clean_backfillShader:
	-rm -f $(backfillShader_MAKEFILE).bak $(backfillShader_OBJECTS) $(backfillShader_PLUGIN)


plugins: $(backfillShader_PLUGIN)
depend:	 depend_backfillShader
clean:	 clean_backfillShader
Clean:	 Clean_backfillShader

