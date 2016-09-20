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

SRCDIR := $(TOP)/rockingTransformCheck
DSTDIR := $(TOP)/rockingTransformCheck

rockingTransformCheck_SOURCES  := $(TOP)/rockingTransformCheck/rockingTransformCheck.cpp $(TOP)/rockingTransformCheck/rockingTransformCheckMain.cpp $(TOP)/rockingTransformCheck/rockingTransform.cpp
rockingTransformCheck_OBJECTS  := $(TOP)/rockingTransformCheck/rockingTransformCheck.o $(TOP)/rockingTransformCheck/rockingTransformCheckMain.o $(TOP)/rockingTransformCheck/rockingTransform.o
rockingTransformCheck_PLUGIN   := $(DSTDIR)/rockingTransformCheck.$(EXT)
rockingTransformCheck_MAKEFILE := $(DSTDIR)/Makefile

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

$(rockingTransformCheck_OBJECTS): CFLAGS   := $(CFLAGS)   $(rockingTransformCheck_EXTRA_CFLAGS)
$(rockingTransformCheck_OBJECTS): C++FLAGS := $(C++FLAGS) $(rockingTransformCheck_EXTRA_C++FLAGS)
$(rockingTransformCheck_OBJECTS): INCLUDES := $(INCLUDES) $(rockingTransformCheck_EXTRA_INCLUDES)

depend_rockingTransformCheck:     INCLUDES := $(INCLUDES) $(rockingTransformCheck_EXTRA_INCLUDES)

$(rockingTransformCheck_PLUGIN):  LFLAGS   := $(LFLAGS) $(rockingTransformCheck_EXTRA_LFLAGS) 
$(rockingTransformCheck_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(rockingTransformCheck_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_rockingTransformCheck clean_rockingTransformCheck Clean_rockingTransformCheck


$(rockingTransformCheck_PLUGIN): $(rockingTransformCheck_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_rockingTransformCheck :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(rockingTransformCheck_SOURCES)

clean_rockingTransformCheck:
	-rm -f $(rockingTransformCheck_OBJECTS)

Clean_rockingTransformCheck:
	-rm -f $(rockingTransformCheck_MAKEFILE).bak $(rockingTransformCheck_OBJECTS) $(rockingTransformCheck_PLUGIN)


plugins: $(rockingTransformCheck_PLUGIN)
depend:	 depend_rockingTransformCheck
clean:	 clean_rockingTransformCheck
Clean:	 Clean_rockingTransformCheck

