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

SRCDIR := $(TOP)/XmlGeometryCacheDesc
DSTDIR := $(TOP)/XmlGeometryCacheDesc

XmlGeometryCacheDesc_SOURCES  := $(TOP)/XmlGeometryCacheDesc/XmlGeometryCacheDesc.cpp
XmlGeometryCacheDesc_OBJECTS  := $(TOP)/XmlGeometryCacheDesc/XmlGeometryCacheDesc.o
XmlGeometryCacheDesc_PLUGIN   := $(DSTDIR)/XmlGeometryCacheDesc.$(EXT)
XmlGeometryCacheDesc_MAKEFILE := $(DSTDIR)/Makefile

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

$(XmlGeometryCacheDesc_OBJECTS): CFLAGS   := $(CFLAGS)   $(XmlGeometryCacheDesc_EXTRA_CFLAGS)
$(XmlGeometryCacheDesc_OBJECTS): C++FLAGS := $(C++FLAGS) $(XmlGeometryCacheDesc_EXTRA_C++FLAGS)
$(XmlGeometryCacheDesc_OBJECTS): INCLUDES := $(INCLUDES) $(XmlGeometryCacheDesc_EXTRA_INCLUDES)

depend_XmlGeometryCacheDesc:     INCLUDES := $(INCLUDES) $(XmlGeometryCacheDesc_EXTRA_INCLUDES)

$(XmlGeometryCacheDesc_PLUGIN):  LFLAGS   := $(LFLAGS) $(XmlGeometryCacheDesc_EXTRA_LFLAGS) 
$(XmlGeometryCacheDesc_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(XmlGeometryCacheDesc_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_XmlGeometryCacheDesc clean_XmlGeometryCacheDesc Clean_XmlGeometryCacheDesc


$(XmlGeometryCacheDesc_PLUGIN): $(XmlGeometryCacheDesc_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_XmlGeometryCacheDesc :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(XmlGeometryCacheDesc_SOURCES)

clean_XmlGeometryCacheDesc:
	-rm -f $(XmlGeometryCacheDesc_OBJECTS)

Clean_XmlGeometryCacheDesc:
	-rm -f $(XmlGeometryCacheDesc_MAKEFILE).bak $(XmlGeometryCacheDesc_OBJECTS) $(XmlGeometryCacheDesc_PLUGIN)


plugins: $(XmlGeometryCacheDesc_PLUGIN)
depend:	 depend_XmlGeometryCacheDesc
clean:	 clean_XmlGeometryCacheDesc
Clean:	 Clean_XmlGeometryCacheDesc

