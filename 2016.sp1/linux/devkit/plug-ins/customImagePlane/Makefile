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

SRCDIR := $(TOP)/customImagePlane
DSTDIR := $(TOP)/customImagePlane

customImagePlane_SOURCES  := $(TOP)/customImagePlane/customImagePlane.cpp
customImagePlane_OBJECTS  := $(TOP)/customImagePlane/customImagePlane.o
customImagePlane_PLUGIN   := $(DSTDIR)/customImagePlane.$(EXT)
customImagePlane_MAKEFILE := $(DSTDIR)/Makefile

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

$(customImagePlane_OBJECTS): CFLAGS   := $(CFLAGS)   $(customImagePlane_EXTRA_CFLAGS)
$(customImagePlane_OBJECTS): C++FLAGS := $(C++FLAGS) $(customImagePlane_EXTRA_C++FLAGS)
$(customImagePlane_OBJECTS): INCLUDES := $(INCLUDES) $(customImagePlane_EXTRA_INCLUDES)

depend_customImagePlane:     INCLUDES := $(INCLUDES) $(customImagePlane_EXTRA_INCLUDES)

$(customImagePlane_PLUGIN):  LFLAGS   := $(LFLAGS) $(customImagePlane_EXTRA_LFLAGS) 
$(customImagePlane_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(customImagePlane_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_customImagePlane clean_customImagePlane Clean_customImagePlane


$(customImagePlane_PLUGIN): $(customImagePlane_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_customImagePlane :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(customImagePlane_SOURCES)

clean_customImagePlane:
	-rm -f $(customImagePlane_OBJECTS)

Clean_customImagePlane:
	-rm -f $(customImagePlane_MAKEFILE).bak $(customImagePlane_OBJECTS) $(customImagePlane_PLUGIN)


plugins: $(customImagePlane_PLUGIN)
depend:	 depend_customImagePlane
clean:	 clean_customImagePlane
Clean:	 Clean_customImagePlane

