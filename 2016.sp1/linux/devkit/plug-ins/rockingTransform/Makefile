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

SRCDIR := $(TOP)/rockingTransform
DSTDIR := $(TOP)/rockingTransform

rockingTransform_SOURCES  := $(TOP)/rockingTransform/rockingTransform.cpp $(TOP)/rockingTransform/rockingTransformMain.cpp
rockingTransform_OBJECTS  := $(TOP)/rockingTransform/rockingTransform.o $(TOP)/rockingTransform/rockingTransformMain.o
rockingTransform_PLUGIN   := $(DSTDIR)/rockingTransform.$(EXT)
rockingTransform_MAKEFILE := $(DSTDIR)/Makefile

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

$(rockingTransform_OBJECTS): CFLAGS   := $(CFLAGS)   $(rockingTransform_EXTRA_CFLAGS)
$(rockingTransform_OBJECTS): C++FLAGS := $(C++FLAGS) $(rockingTransform_EXTRA_C++FLAGS)
$(rockingTransform_OBJECTS): INCLUDES := $(INCLUDES) $(rockingTransform_EXTRA_INCLUDES)

depend_rockingTransform:     INCLUDES := $(INCLUDES) $(rockingTransform_EXTRA_INCLUDES)

$(rockingTransform_PLUGIN):  LFLAGS   := $(LFLAGS) $(rockingTransform_EXTRA_LFLAGS) 
$(rockingTransform_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(rockingTransform_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_rockingTransform clean_rockingTransform Clean_rockingTransform


$(rockingTransform_PLUGIN): $(rockingTransform_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_rockingTransform :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(rockingTransform_SOURCES)

clean_rockingTransform:
	-rm -f $(rockingTransform_OBJECTS)

Clean_rockingTransform:
	-rm -f $(rockingTransform_MAKEFILE).bak $(rockingTransform_OBJECTS) $(rockingTransform_PLUGIN)


plugins: $(rockingTransform_PLUGIN)
depend:	 depend_rockingTransform
clean:	 clean_rockingTransform
Clean:	 Clean_rockingTransform

