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

SRCDIR := $(TOP)/footPrintNode_GeometryOverride
DSTDIR := $(TOP)/footPrintNode_GeometryOverride

footPrintNode_GeometryOverride_SOURCES  := $(TOP)/footPrintNode_GeometryOverride/footPrintNode_GeometryOverride.cpp
footPrintNode_GeometryOverride_OBJECTS  := $(TOP)/footPrintNode_GeometryOverride/footPrintNode_GeometryOverride.o
footPrintNode_GeometryOverride_PLUGIN   := $(DSTDIR)/footPrintNode_GeometryOverride.$(EXT)
footPrintNode_GeometryOverride_MAKEFILE := $(DSTDIR)/Makefile

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

$(footPrintNode_GeometryOverride_OBJECTS): CFLAGS   := $(CFLAGS)   $(footPrintNode_GeometryOverride_EXTRA_CFLAGS)
$(footPrintNode_GeometryOverride_OBJECTS): C++FLAGS := $(C++FLAGS) $(footPrintNode_GeometryOverride_EXTRA_C++FLAGS)
$(footPrintNode_GeometryOverride_OBJECTS): INCLUDES := $(INCLUDES) $(footPrintNode_GeometryOverride_EXTRA_INCLUDES)

depend_footPrintNode_GeometryOverride:     INCLUDES := $(INCLUDES) $(footPrintNode_GeometryOverride_EXTRA_INCLUDES)

$(footPrintNode_GeometryOverride_PLUGIN):  LFLAGS   := $(LFLAGS) $(footPrintNode_GeometryOverride_EXTRA_LFLAGS) 
$(footPrintNode_GeometryOverride_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lOpenMayaUI -lFoundation -lGL $(footPrintNode_GeometryOverride_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_footPrintNode_GeometryOverride clean_footPrintNode_GeometryOverride Clean_footPrintNode_GeometryOverride


$(footPrintNode_GeometryOverride_PLUGIN): $(footPrintNode_GeometryOverride_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_footPrintNode_GeometryOverride :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(footPrintNode_GeometryOverride_SOURCES)

clean_footPrintNode_GeometryOverride:
	-rm -f $(footPrintNode_GeometryOverride_OBJECTS)

Clean_footPrintNode_GeometryOverride:
	-rm -f $(footPrintNode_GeometryOverride_MAKEFILE).bak $(footPrintNode_GeometryOverride_OBJECTS) $(footPrintNode_GeometryOverride_PLUGIN)


plugins: $(footPrintNode_GeometryOverride_PLUGIN)
depend:	 depend_footPrintNode_GeometryOverride
clean:	 clean_footPrintNode_GeometryOverride
Clean:	 Clean_footPrintNode_GeometryOverride

