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

SRCDIR := $(TOP)/interpPlugins
DSTDIR := $(TOP)/interpPlugins

interpPlugins_SOURCES  := $(TOP)/interpPlugins/interpFlat.cpp $(TOP)/interpPlugins/interpHalf.cpp $(TOP)/interpPlugins/interpPlugins.cpp
interpPlugins_OBJECTS  := $(TOP)/interpPlugins/interpFlat.o $(TOP)/interpPlugins/interpHalf.o $(TOP)/interpPlugins/interpPlugins.o
interpPlugins_PLUGIN   := $(DSTDIR)/interpPlugins.$(EXT)
interpPlugins_MAKEFILE := $(DSTDIR)/Makefile

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

$(interpPlugins_OBJECTS): CFLAGS   := $(CFLAGS)   $(interpPlugins_EXTRA_CFLAGS)
$(interpPlugins_OBJECTS): C++FLAGS := $(C++FLAGS) $(interpPlugins_EXTRA_C++FLAGS)
$(interpPlugins_OBJECTS): INCLUDES := $(INCLUDES) $(interpPlugins_EXTRA_INCLUDES)

depend_interpPlugins:     INCLUDES := $(INCLUDES) $(interpPlugins_EXTRA_INCLUDES)

$(interpPlugins_PLUGIN):  LFLAGS   := $(LFLAGS) $(interpPlugins_EXTRA_LFLAGS) 
$(interpPlugins_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaAnim -lOpenMaya -lFoundation $(interpPlugins_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_interpPlugins clean_interpPlugins Clean_interpPlugins


$(interpPlugins_PLUGIN): $(interpPlugins_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_interpPlugins :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(interpPlugins_SOURCES)

clean_interpPlugins:
	-rm -f $(interpPlugins_OBJECTS)

Clean_interpPlugins:
	-rm -f $(interpPlugins_MAKEFILE).bak $(interpPlugins_OBJECTS) $(interpPlugins_PLUGIN)


plugins: $(interpPlugins_PLUGIN)
depend:	 depend_interpPlugins
clean:	 clean_interpPlugins
Clean:	 Clean_interpPlugins

