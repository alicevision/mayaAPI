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

SRCDIR := $(TOP)/lensDistortionCallback
DSTDIR := $(TOP)/lensDistortionCallback

lensDistortionCallback_SOURCES  := $(TOP)/lensDistortionCallback/lensDistortionCallback.cpp
lensDistortionCallback_OBJECTS  := $(TOP)/lensDistortionCallback/lensDistortionCallback.o
lensDistortionCallback_PLUGIN   := $(DSTDIR)/lensDistortionCallback.$(EXT)
lensDistortionCallback_MAKEFILE := $(DSTDIR)/Makefile

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

$(lensDistortionCallback_OBJECTS): CFLAGS   := $(CFLAGS)   $(lensDistortionCallback_EXTRA_CFLAGS)
$(lensDistortionCallback_OBJECTS): C++FLAGS := $(C++FLAGS) $(lensDistortionCallback_EXTRA_C++FLAGS)
$(lensDistortionCallback_OBJECTS): INCLUDES := $(INCLUDES) $(lensDistortionCallback_EXTRA_INCLUDES)

depend_lensDistortionCallback:     INCLUDES := $(INCLUDES) $(lensDistortionCallback_EXTRA_INCLUDES)

$(lensDistortionCallback_PLUGIN):  LFLAGS   := $(LFLAGS) $(lensDistortionCallback_EXTRA_LFLAGS) 
$(lensDistortionCallback_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(lensDistortionCallback_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lensDistortionCallback clean_lensDistortionCallback Clean_lensDistortionCallback


$(lensDistortionCallback_PLUGIN): $(lensDistortionCallback_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lensDistortionCallback :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lensDistortionCallback_SOURCES)

clean_lensDistortionCallback:
	-rm -f $(lensDistortionCallback_OBJECTS)

Clean_lensDistortionCallback:
	-rm -f $(lensDistortionCallback_MAKEFILE).bak $(lensDistortionCallback_OBJECTS) $(lensDistortionCallback_PLUGIN)


plugins: $(lensDistortionCallback_PLUGIN)
depend:	 depend_lensDistortionCallback
clean:	 clean_lensDistortionCallback
Clean:	 Clean_lensDistortionCallback

