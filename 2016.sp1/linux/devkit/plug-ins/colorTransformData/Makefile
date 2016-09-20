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

SRCDIR := $(TOP)/colorTransformData
DSTDIR := $(TOP)/colorTransformData

colorTransformData_SOURCES  := $(TOP)/colorTransformData/colorTransformData.cpp
colorTransformData_OBJECTS  := $(TOP)/colorTransformData/colorTransformData.o
colorTransformData_PLUGIN   := $(DSTDIR)/colorTransformData.$(EXT)
colorTransformData_MAKEFILE := $(DSTDIR)/Makefile

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

$(colorTransformData_OBJECTS): CFLAGS   := $(CFLAGS)   $(colorTransformData_EXTRA_CFLAGS)
$(colorTransformData_OBJECTS): C++FLAGS := $(C++FLAGS) $(colorTransformData_EXTRA_C++FLAGS)
$(colorTransformData_OBJECTS): INCLUDES := $(INCLUDES) $(colorTransformData_EXTRA_INCLUDES)

depend_colorTransformData:     INCLUDES := $(INCLUDES) $(colorTransformData_EXTRA_INCLUDES)

$(colorTransformData_PLUGIN):  LFLAGS   := $(LFLAGS) $(colorTransformData_EXTRA_LFLAGS) 
$(colorTransformData_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMaya -lFoundation $(colorTransformData_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_colorTransformData clean_colorTransformData Clean_colorTransformData


$(colorTransformData_PLUGIN): $(colorTransformData_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_colorTransformData :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(colorTransformData_SOURCES)

clean_colorTransformData:
	-rm -f $(colorTransformData_OBJECTS)

Clean_colorTransformData:
	-rm -f $(colorTransformData_MAKEFILE).bak $(colorTransformData_OBJECTS) $(colorTransformData_PLUGIN)


plugins: $(colorTransformData_PLUGIN)
depend:	 depend_colorTransformData
clean:	 clean_colorTransformData
Clean:	 Clean_colorTransformData

