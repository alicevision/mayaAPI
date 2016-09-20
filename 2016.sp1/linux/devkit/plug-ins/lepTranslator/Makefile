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

SRCDIR := $(TOP)/lepTranslator
DSTDIR := $(TOP)/lepTranslator

lepTranslator_SOURCES  := $(TOP)/lepTranslator/lepTranslator.cpp
lepTranslator_OBJECTS  := $(TOP)/lepTranslator/lepTranslator.o
lepTranslator_PLUGIN   := $(DSTDIR)/lepTranslator.$(EXT)
lepTranslator_MAKEFILE := $(DSTDIR)/Makefile

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

$(lepTranslator_OBJECTS): CFLAGS   := $(CFLAGS)   $(lepTranslator_EXTRA_CFLAGS)
$(lepTranslator_OBJECTS): C++FLAGS := $(C++FLAGS) $(lepTranslator_EXTRA_C++FLAGS)
$(lepTranslator_OBJECTS): INCLUDES := $(INCLUDES) $(lepTranslator_EXTRA_INCLUDES)

depend_lepTranslator:     INCLUDES := $(INCLUDES) $(lepTranslator_EXTRA_INCLUDES)

$(lepTranslator_PLUGIN):  LFLAGS   := $(LFLAGS) $(lepTranslator_EXTRA_LFLAGS) 
$(lepTranslator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(lepTranslator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lepTranslator clean_lepTranslator Clean_lepTranslator


$(lepTranslator_PLUGIN): $(lepTranslator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lepTranslator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lepTranslator_SOURCES)

clean_lepTranslator:
	-rm -f $(lepTranslator_OBJECTS)

Clean_lepTranslator:
	-rm -f $(lepTranslator_MAKEFILE).bak $(lepTranslator_OBJECTS) $(lepTranslator_PLUGIN)


plugins: $(lepTranslator_PLUGIN)
depend:	 depend_lepTranslator
clean:	 clean_lepTranslator
Clean:	 Clean_lepTranslator

