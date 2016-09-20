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

SRCDIR := $(TOP)/fileTexture
DSTDIR := $(TOP)/fileTexture

fileTexture_SOURCES  := $(TOP)/fileTexture/fileTexture.cpp
fileTexture_OBJECTS  := $(TOP)/fileTexture/fileTexture.o
fileTexture_PLUGIN   := $(DSTDIR)/fileTexture.$(EXT)
fileTexture_MAKEFILE := $(DSTDIR)/Makefile

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

$(fileTexture_OBJECTS): CFLAGS   := $(CFLAGS)   $(fileTexture_EXTRA_CFLAGS)
$(fileTexture_OBJECTS): C++FLAGS := $(C++FLAGS) $(fileTexture_EXTRA_C++FLAGS)
$(fileTexture_OBJECTS): INCLUDES := $(INCLUDES) $(fileTexture_EXTRA_INCLUDES)

depend_fileTexture:     INCLUDES := $(INCLUDES) $(fileTexture_EXTRA_INCLUDES)

$(fileTexture_PLUGIN):  LFLAGS   := $(LFLAGS) $(fileTexture_EXTRA_LFLAGS) 
$(fileTexture_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(fileTexture_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_fileTexture clean_fileTexture Clean_fileTexture


$(fileTexture_PLUGIN): $(fileTexture_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_fileTexture :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(fileTexture_SOURCES)

clean_fileTexture:
	-rm -f $(fileTexture_OBJECTS)

Clean_fileTexture:
	-rm -f $(fileTexture_MAKEFILE).bak $(fileTexture_OBJECTS) $(fileTexture_PLUGIN)


plugins: $(fileTexture_PLUGIN)
depend:	 depend_fileTexture
clean:	 clean_fileTexture
Clean:	 Clean_fileTexture

