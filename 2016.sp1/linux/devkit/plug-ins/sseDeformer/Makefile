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

SRCDIR := $(TOP)/sseDeformer
DSTDIR := $(TOP)/sseDeformer

sseDeformer_SOURCES  := $(TOP)/sseDeformer/sseDeformer.cpp
sseDeformer_OBJECTS  := $(TOP)/sseDeformer/sseDeformer.o
sseDeformer_PLUGIN   := $(DSTDIR)/sseDeformer.$(EXT)
sseDeformer_MAKEFILE := $(DSTDIR)/Makefile

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

$(sseDeformer_OBJECTS): CFLAGS   := $(CFLAGS)   $(sseDeformer_EXTRA_CFLAGS)
$(sseDeformer_OBJECTS): C++FLAGS := $(C++FLAGS) $(sseDeformer_EXTRA_C++FLAGS)
$(sseDeformer_OBJECTS): INCLUDES := $(INCLUDES) $(sseDeformer_EXTRA_INCLUDES)

depend_sseDeformer:     INCLUDES := $(INCLUDES) $(sseDeformer_EXTRA_INCLUDES)

$(sseDeformer_PLUGIN):  LFLAGS   := $(LFLAGS) $(sseDeformer_EXTRA_LFLAGS) 
$(sseDeformer_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(sseDeformer_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_sseDeformer clean_sseDeformer Clean_sseDeformer


$(sseDeformer_PLUGIN): $(sseDeformer_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_sseDeformer :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(sseDeformer_SOURCES)

clean_sseDeformer:
	-rm -f $(sseDeformer_OBJECTS)

Clean_sseDeformer:
	-rm -f $(sseDeformer_MAKEFILE).bak $(sseDeformer_OBJECTS) $(sseDeformer_PLUGIN)


plugins: $(sseDeformer_PLUGIN)
depend:	 depend_sseDeformer
clean:	 clean_sseDeformer
Clean:	 Clean_sseDeformer

