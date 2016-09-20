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

SRCDIR := $(TOP)/convertEdgesToFacesCmd
DSTDIR := $(TOP)/convertEdgesToFacesCmd

convertEdgesToFacesCmd_SOURCES  := $(TOP)/convertEdgesToFacesCmd/convertEdgesToFacesCmd.cpp
convertEdgesToFacesCmd_OBJECTS  := $(TOP)/convertEdgesToFacesCmd/convertEdgesToFacesCmd.o
convertEdgesToFacesCmd_PLUGIN   := $(DSTDIR)/convertEdgesToFacesCmd.$(EXT)
convertEdgesToFacesCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(convertEdgesToFacesCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(convertEdgesToFacesCmd_EXTRA_CFLAGS)
$(convertEdgesToFacesCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(convertEdgesToFacesCmd_EXTRA_C++FLAGS)
$(convertEdgesToFacesCmd_OBJECTS): INCLUDES := $(INCLUDES) $(convertEdgesToFacesCmd_EXTRA_INCLUDES)

depend_convertEdgesToFacesCmd:     INCLUDES := $(INCLUDES) $(convertEdgesToFacesCmd_EXTRA_INCLUDES)

$(convertEdgesToFacesCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(convertEdgesToFacesCmd_EXTRA_LFLAGS) 
$(convertEdgesToFacesCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lGL $(convertEdgesToFacesCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_convertEdgesToFacesCmd clean_convertEdgesToFacesCmd Clean_convertEdgesToFacesCmd


$(convertEdgesToFacesCmd_PLUGIN): $(convertEdgesToFacesCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_convertEdgesToFacesCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(convertEdgesToFacesCmd_SOURCES)

clean_convertEdgesToFacesCmd:
	-rm -f $(convertEdgesToFacesCmd_OBJECTS)

Clean_convertEdgesToFacesCmd:
	-rm -f $(convertEdgesToFacesCmd_MAKEFILE).bak $(convertEdgesToFacesCmd_OBJECTS) $(convertEdgesToFacesCmd_PLUGIN)


plugins: $(convertEdgesToFacesCmd_PLUGIN)
depend:	 depend_convertEdgesToFacesCmd
clean:	 clean_convertEdgesToFacesCmd
Clean:	 Clean_convertEdgesToFacesCmd

