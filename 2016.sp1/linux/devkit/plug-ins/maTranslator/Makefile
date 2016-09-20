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

SRCDIR := $(TOP)/maTranslator
DSTDIR := $(TOP)/maTranslator

maTranslator_SOURCES  := $(TOP)/maTranslator/maTranslator.cpp
maTranslator_OBJECTS  := $(TOP)/maTranslator/maTranslator.o
maTranslator_PLUGIN   := $(DSTDIR)/maTranslator.$(EXT)
maTranslator_MAKEFILE := $(DSTDIR)/Makefile

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

$(maTranslator_OBJECTS): CFLAGS   := $(CFLAGS)   $(maTranslator_EXTRA_CFLAGS)
$(maTranslator_OBJECTS): C++FLAGS := $(C++FLAGS) $(maTranslator_EXTRA_C++FLAGS)
$(maTranslator_OBJECTS): INCLUDES := $(INCLUDES) $(maTranslator_EXTRA_INCLUDES)

depend_maTranslator:     INCLUDES := $(INCLUDES) $(maTranslator_EXTRA_INCLUDES)

$(maTranslator_PLUGIN):  LFLAGS   := $(LFLAGS) $(maTranslator_EXTRA_LFLAGS) 
$(maTranslator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(maTranslator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_maTranslator clean_maTranslator Clean_maTranslator


$(maTranslator_PLUGIN): $(maTranslator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_maTranslator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(maTranslator_SOURCES)

clean_maTranslator:
	-rm -f $(maTranslator_OBJECTS)

Clean_maTranslator:
	-rm -f $(maTranslator_MAKEFILE).bak $(maTranslator_OBJECTS) $(maTranslator_PLUGIN)


plugins: $(maTranslator_PLUGIN)
depend:	 depend_maTranslator
clean:	 clean_maTranslator
Clean:	 Clean_maTranslator

