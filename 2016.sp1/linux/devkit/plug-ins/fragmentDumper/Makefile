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

SRCDIR := $(TOP)/fragmentDumper
DSTDIR := $(TOP)/fragmentDumper

fragmentDumper_SOURCES  := $(TOP)/fragmentDumper/fragmentDumper.cpp
fragmentDumper_OBJECTS  := $(TOP)/fragmentDumper/fragmentDumper.o
fragmentDumper_PLUGIN   := $(DSTDIR)/fragmentDumper.$(EXT)
fragmentDumper_MAKEFILE := $(DSTDIR)/Makefile

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

$(fragmentDumper_OBJECTS): CFLAGS   := $(CFLAGS)   $(fragmentDumper_EXTRA_CFLAGS)
$(fragmentDumper_OBJECTS): C++FLAGS := $(C++FLAGS) $(fragmentDumper_EXTRA_C++FLAGS)
$(fragmentDumper_OBJECTS): INCLUDES := $(INCLUDES) $(fragmentDumper_EXTRA_INCLUDES)

depend_fragmentDumper:     INCLUDES := $(INCLUDES) $(fragmentDumper_EXTRA_INCLUDES)

$(fragmentDumper_PLUGIN):  LFLAGS   := $(LFLAGS) $(fragmentDumper_EXTRA_LFLAGS) 
$(fragmentDumper_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(fragmentDumper_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_fragmentDumper clean_fragmentDumper Clean_fragmentDumper


$(fragmentDumper_PLUGIN): $(fragmentDumper_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_fragmentDumper :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(fragmentDumper_SOURCES)

clean_fragmentDumper:
	-rm -f $(fragmentDumper_OBJECTS)

Clean_fragmentDumper:
	-rm -f $(fragmentDumper_MAKEFILE).bak $(fragmentDumper_OBJECTS) $(fragmentDumper_PLUGIN)


plugins: $(fragmentDumper_PLUGIN)
depend:	 depend_fragmentDumper
clean:	 clean_fragmentDumper
Clean:	 Clean_fragmentDumper

