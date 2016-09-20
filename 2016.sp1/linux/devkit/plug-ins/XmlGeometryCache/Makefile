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

SRCDIR := $(TOP)/XmlGeometryCache
DSTDIR := $(TOP)/XmlGeometryCache

XmlGeometryCache_SOURCES  := $(TOP)/XmlGeometryCache/XmlGeometryCache.cpp
XmlGeometryCache_OBJECTS  := $(TOP)/XmlGeometryCache/XmlGeometryCache.o
XmlGeometryCache_PLUGIN   := $(DSTDIR)/XmlGeometryCache.$(EXT)
XmlGeometryCache_MAKEFILE := $(DSTDIR)/Makefile

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

$(XmlGeometryCache_OBJECTS): CFLAGS   := $(CFLAGS)   $(XmlGeometryCache_EXTRA_CFLAGS)
$(XmlGeometryCache_OBJECTS): C++FLAGS := $(C++FLAGS) $(XmlGeometryCache_EXTRA_C++FLAGS)
$(XmlGeometryCache_OBJECTS): INCLUDES := $(INCLUDES) $(XmlGeometryCache_EXTRA_INCLUDES)

depend_XmlGeometryCache:     INCLUDES := $(INCLUDES) $(XmlGeometryCache_EXTRA_INCLUDES)

$(XmlGeometryCache_PLUGIN):  LFLAGS   := $(LFLAGS) $(XmlGeometryCache_EXTRA_LFLAGS) 
$(XmlGeometryCache_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(XmlGeometryCache_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_XmlGeometryCache clean_XmlGeometryCache Clean_XmlGeometryCache


$(XmlGeometryCache_PLUGIN): $(XmlGeometryCache_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_XmlGeometryCache :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(XmlGeometryCache_SOURCES)

clean_XmlGeometryCache:
	-rm -f $(XmlGeometryCache_OBJECTS)

Clean_XmlGeometryCache:
	-rm -f $(XmlGeometryCache_MAKEFILE).bak $(XmlGeometryCache_OBJECTS) $(XmlGeometryCache_PLUGIN)


plugins: $(XmlGeometryCache_PLUGIN)
depend:	 depend_XmlGeometryCache
clean:	 clean_XmlGeometryCache
Clean:	 Clean_XmlGeometryCache

