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

SRCDIR := $(TOP)/splatDeformer
DSTDIR := $(TOP)/splatDeformer

splatDeformer_SOURCES  := $(TOP)/splatDeformer/splatDeformer.cpp
splatDeformer_OBJECTS  := $(TOP)/splatDeformer/splatDeformer.o
splatDeformer_PLUGIN   := $(DSTDIR)/splatDeformer.$(EXT)
splatDeformer_MAKEFILE := $(DSTDIR)/Makefile

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

$(splatDeformer_OBJECTS): CFLAGS   := $(CFLAGS)   $(splatDeformer_EXTRA_CFLAGS)
$(splatDeformer_OBJECTS): C++FLAGS := $(C++FLAGS) $(splatDeformer_EXTRA_C++FLAGS)
$(splatDeformer_OBJECTS): INCLUDES := $(INCLUDES) $(splatDeformer_EXTRA_INCLUDES)

depend_splatDeformer:     INCLUDES := $(INCLUDES) $(splatDeformer_EXTRA_INCLUDES)

$(splatDeformer_PLUGIN):  LFLAGS   := $(LFLAGS) $(splatDeformer_EXTRA_LFLAGS) 
$(splatDeformer_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(splatDeformer_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_splatDeformer clean_splatDeformer Clean_splatDeformer


$(splatDeformer_PLUGIN): $(splatDeformer_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_splatDeformer :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(splatDeformer_SOURCES)

clean_splatDeformer:
	-rm -f $(splatDeformer_OBJECTS)

Clean_splatDeformer:
	-rm -f $(splatDeformer_MAKEFILE).bak $(splatDeformer_OBJECTS) $(splatDeformer_PLUGIN)


plugins: $(splatDeformer_PLUGIN)
depend:	 depend_splatDeformer
clean:	 clean_splatDeformer
Clean:	 Clean_splatDeformer

