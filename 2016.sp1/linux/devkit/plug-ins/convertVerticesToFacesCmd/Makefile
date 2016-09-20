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

SRCDIR := $(TOP)/convertVerticesToFacesCmd
DSTDIR := $(TOP)/convertVerticesToFacesCmd

convertVerticesToFacesCmd_SOURCES  := $(TOP)/convertVerticesToFacesCmd/convertVerticesToFacesCmd.cpp
convertVerticesToFacesCmd_OBJECTS  := $(TOP)/convertVerticesToFacesCmd/convertVerticesToFacesCmd.o
convertVerticesToFacesCmd_PLUGIN   := $(DSTDIR)/convertVerticesToFacesCmd.$(EXT)
convertVerticesToFacesCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(convertVerticesToFacesCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(convertVerticesToFacesCmd_EXTRA_CFLAGS)
$(convertVerticesToFacesCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(convertVerticesToFacesCmd_EXTRA_C++FLAGS)
$(convertVerticesToFacesCmd_OBJECTS): INCLUDES := $(INCLUDES) $(convertVerticesToFacesCmd_EXTRA_INCLUDES)

depend_convertVerticesToFacesCmd:     INCLUDES := $(INCLUDES) $(convertVerticesToFacesCmd_EXTRA_INCLUDES)

$(convertVerticesToFacesCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(convertVerticesToFacesCmd_EXTRA_LFLAGS) 
$(convertVerticesToFacesCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL $(convertVerticesToFacesCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_convertVerticesToFacesCmd clean_convertVerticesToFacesCmd Clean_convertVerticesToFacesCmd


$(convertVerticesToFacesCmd_PLUGIN): $(convertVerticesToFacesCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_convertVerticesToFacesCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(convertVerticesToFacesCmd_SOURCES)

clean_convertVerticesToFacesCmd:
	-rm -f $(convertVerticesToFacesCmd_OBJECTS)

Clean_convertVerticesToFacesCmd:
	-rm -f $(convertVerticesToFacesCmd_MAKEFILE).bak $(convertVerticesToFacesCmd_OBJECTS) $(convertVerticesToFacesCmd_PLUGIN)


plugins: $(convertVerticesToFacesCmd_PLUGIN)
depend:	 depend_convertVerticesToFacesCmd
clean:	 clean_convertVerticesToFacesCmd
Clean:	 Clean_convertVerticesToFacesCmd

