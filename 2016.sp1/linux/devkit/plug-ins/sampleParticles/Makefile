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

SRCDIR := $(TOP)/sampleParticles
DSTDIR := $(TOP)/sampleParticles

sampleParticles_SOURCES  := $(TOP)/sampleParticles/sampleParticles.cpp
sampleParticles_OBJECTS  := $(TOP)/sampleParticles/sampleParticles.o
sampleParticles_PLUGIN   := $(DSTDIR)/sampleParticles.$(EXT)
sampleParticles_MAKEFILE := $(DSTDIR)/Makefile

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

$(sampleParticles_OBJECTS): CFLAGS   := $(CFLAGS)   $(sampleParticles_EXTRA_CFLAGS)
$(sampleParticles_OBJECTS): C++FLAGS := $(C++FLAGS) $(sampleParticles_EXTRA_C++FLAGS)
$(sampleParticles_OBJECTS): INCLUDES := $(INCLUDES) $(sampleParticles_EXTRA_INCLUDES)

depend_sampleParticles:     INCLUDES := $(INCLUDES) $(sampleParticles_EXTRA_INCLUDES)

$(sampleParticles_PLUGIN):  LFLAGS   := $(LFLAGS) $(sampleParticles_EXTRA_LFLAGS) 
$(sampleParticles_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation $(sampleParticles_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_sampleParticles clean_sampleParticles Clean_sampleParticles


$(sampleParticles_PLUGIN): $(sampleParticles_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_sampleParticles :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(sampleParticles_SOURCES)

clean_sampleParticles:
	-rm -f $(sampleParticles_OBJECTS)

Clean_sampleParticles:
	-rm -f $(sampleParticles_MAKEFILE).bak $(sampleParticles_OBJECTS) $(sampleParticles_PLUGIN)


plugins: $(sampleParticles_PLUGIN)
depend:	 depend_sampleParticles
clean:	 clean_sampleParticles
Clean:	 Clean_sampleParticles

