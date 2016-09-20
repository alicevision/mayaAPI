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

SRCDIR := $(TOP)/simpleImageFile
DSTDIR := $(TOP)/simpleImageFile

simpleImageFile_SOURCES  := $(TOP)/simpleImageFile/simpleImageFile.cpp
simpleImageFile_OBJECTS  := $(TOP)/simpleImageFile/simpleImageFile.o
simpleImageFile_PLUGIN   := $(DSTDIR)/simpleImageFile.$(EXT)
simpleImageFile_MAKEFILE := $(DSTDIR)/Makefile

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

$(simpleImageFile_OBJECTS): CFLAGS   := $(CFLAGS)   $(simpleImageFile_EXTRA_CFLAGS)
$(simpleImageFile_OBJECTS): C++FLAGS := $(C++FLAGS) $(simpleImageFile_EXTRA_C++FLAGS)
$(simpleImageFile_OBJECTS): INCLUDES := $(INCLUDES) $(simpleImageFile_EXTRA_INCLUDES)

depend_simpleImageFile:     INCLUDES := $(INCLUDES) $(simpleImageFile_EXTRA_INCLUDES)

$(simpleImageFile_PLUGIN):  LFLAGS   := $(LFLAGS) $(simpleImageFile_EXTRA_LFLAGS) 
$(simpleImageFile_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL -lGLU $(simpleImageFile_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_simpleImageFile clean_simpleImageFile Clean_simpleImageFile


$(simpleImageFile_PLUGIN): $(simpleImageFile_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_simpleImageFile :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(simpleImageFile_SOURCES)

clean_simpleImageFile:
	-rm -f $(simpleImageFile_OBJECTS)

Clean_simpleImageFile:
	-rm -f $(simpleImageFile_MAKEFILE).bak $(simpleImageFile_OBJECTS) $(simpleImageFile_PLUGIN)


plugins: $(simpleImageFile_PLUGIN)
depend:	 depend_simpleImageFile
clean:	 clean_simpleImageFile
Clean:	 Clean_simpleImageFile

