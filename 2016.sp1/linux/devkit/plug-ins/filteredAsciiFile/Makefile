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

SRCDIR := $(TOP)/filteredAsciiFile
DSTDIR := $(TOP)/filteredAsciiFile

filteredAsciiFile_SOURCES  := $(TOP)/filteredAsciiFile/filteredAsciiFile.cpp
filteredAsciiFile_OBJECTS  := $(TOP)/filteredAsciiFile/filteredAsciiFile.o
filteredAsciiFile_PLUGIN   := $(DSTDIR)/filteredAsciiFile.$(EXT)
filteredAsciiFile_MAKEFILE := $(DSTDIR)/Makefile

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

$(filteredAsciiFile_OBJECTS): CFLAGS   := $(CFLAGS)   $(filteredAsciiFile_EXTRA_CFLAGS)
$(filteredAsciiFile_OBJECTS): C++FLAGS := $(C++FLAGS) $(filteredAsciiFile_EXTRA_C++FLAGS)
$(filteredAsciiFile_OBJECTS): INCLUDES := $(INCLUDES) $(filteredAsciiFile_EXTRA_INCLUDES)

depend_filteredAsciiFile:     INCLUDES := $(INCLUDES) $(filteredAsciiFile_EXTRA_INCLUDES)

$(filteredAsciiFile_PLUGIN):  LFLAGS   := $(LFLAGS) $(filteredAsciiFile_EXTRA_LFLAGS) 
$(filteredAsciiFile_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(filteredAsciiFile_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_filteredAsciiFile clean_filteredAsciiFile Clean_filteredAsciiFile


$(filteredAsciiFile_PLUGIN): $(filteredAsciiFile_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_filteredAsciiFile :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(filteredAsciiFile_SOURCES)

clean_filteredAsciiFile:
	-rm -f $(filteredAsciiFile_OBJECTS)

Clean_filteredAsciiFile:
	-rm -f $(filteredAsciiFile_MAKEFILE).bak $(filteredAsciiFile_OBJECTS) $(filteredAsciiFile_PLUGIN)


plugins: $(filteredAsciiFile_PLUGIN)
depend:	 depend_filteredAsciiFile
clean:	 clean_filteredAsciiFile
Clean:	 Clean_filteredAsciiFile

