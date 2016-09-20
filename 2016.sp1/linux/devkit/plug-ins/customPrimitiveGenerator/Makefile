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

SRCDIR := $(TOP)/customPrimitiveGenerator
DSTDIR := $(TOP)/customPrimitiveGenerator

customPrimitiveGenerator_SOURCES  := $(TOP)/customPrimitiveGenerator/customPrimitiveGenerator.cpp
customPrimitiveGenerator_OBJECTS  := $(TOP)/customPrimitiveGenerator/customPrimitiveGenerator.o
customPrimitiveGenerator_PLUGIN   := $(DSTDIR)/customPrimitiveGenerator.$(EXT)
customPrimitiveGenerator_MAKEFILE := $(DSTDIR)/Makefile

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

$(customPrimitiveGenerator_OBJECTS): CFLAGS   := $(CFLAGS)   $(customPrimitiveGenerator_EXTRA_CFLAGS)
$(customPrimitiveGenerator_OBJECTS): C++FLAGS := $(C++FLAGS) $(customPrimitiveGenerator_EXTRA_C++FLAGS)
$(customPrimitiveGenerator_OBJECTS): INCLUDES := $(INCLUDES) $(customPrimitiveGenerator_EXTRA_INCLUDES)

depend_customPrimitiveGenerator:     INCLUDES := $(INCLUDES) $(customPrimitiveGenerator_EXTRA_INCLUDES)

$(customPrimitiveGenerator_PLUGIN):  LFLAGS   := $(LFLAGS) $(customPrimitiveGenerator_EXTRA_LFLAGS) 
$(customPrimitiveGenerator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(customPrimitiveGenerator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_customPrimitiveGenerator clean_customPrimitiveGenerator Clean_customPrimitiveGenerator


$(customPrimitiveGenerator_PLUGIN): $(customPrimitiveGenerator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_customPrimitiveGenerator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(customPrimitiveGenerator_SOURCES)

clean_customPrimitiveGenerator:
	-rm -f $(customPrimitiveGenerator_OBJECTS)

Clean_customPrimitiveGenerator:
	-rm -f $(customPrimitiveGenerator_MAKEFILE).bak $(customPrimitiveGenerator_OBJECTS) $(customPrimitiveGenerator_PLUGIN)


plugins: $(customPrimitiveGenerator_PLUGIN)
depend:	 depend_customPrimitiveGenerator
clean:	 clean_customPrimitiveGenerator
Clean:	 Clean_customPrimitiveGenerator

