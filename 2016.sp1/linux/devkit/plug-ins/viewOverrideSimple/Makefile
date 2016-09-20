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

SRCDIR := $(TOP)/viewOverrideSimple
DSTDIR := $(TOP)/viewOverrideSimple

viewOverrideSimple_SOURCES  := $(TOP)/viewOverrideSimple/viewOverrideSimple.cpp $(TOP)/viewOverrideSimple/viewOverrideSimpleMain.cpp
viewOverrideSimple_OBJECTS  := $(TOP)/viewOverrideSimple/viewOverrideSimple.o $(TOP)/viewOverrideSimple/viewOverrideSimpleMain.o
viewOverrideSimple_PLUGIN   := $(DSTDIR)/viewOverrideSimple.$(EXT)
viewOverrideSimple_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewOverrideSimple_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewOverrideSimple_EXTRA_CFLAGS)
$(viewOverrideSimple_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewOverrideSimple_EXTRA_C++FLAGS)
$(viewOverrideSimple_OBJECTS): INCLUDES := $(INCLUDES) $(viewOverrideSimple_EXTRA_INCLUDES)

depend_viewOverrideSimple:     INCLUDES := $(INCLUDES) $(viewOverrideSimple_EXTRA_INCLUDES)

$(viewOverrideSimple_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewOverrideSimple_EXTRA_LFLAGS) 
$(viewOverrideSimple_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(viewOverrideSimple_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewOverrideSimple clean_viewOverrideSimple Clean_viewOverrideSimple


$(viewOverrideSimple_PLUGIN): $(viewOverrideSimple_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewOverrideSimple :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewOverrideSimple_SOURCES)

clean_viewOverrideSimple:
	-rm -f $(viewOverrideSimple_OBJECTS)

Clean_viewOverrideSimple:
	-rm -f $(viewOverrideSimple_MAKEFILE).bak $(viewOverrideSimple_OBJECTS) $(viewOverrideSimple_PLUGIN)


plugins: $(viewOverrideSimple_PLUGIN)
depend:	 depend_viewOverrideSimple
clean:	 clean_viewOverrideSimple
Clean:	 Clean_viewOverrideSimple

