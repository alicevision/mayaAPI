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

SRCDIR := $(TOP)/exportSkinClusterDataCmd
DSTDIR := $(TOP)/exportSkinClusterDataCmd

exportSkinClusterDataCmd_SOURCES  := $(TOP)/exportSkinClusterDataCmd/exportSkinClusterDataCmd.cpp
exportSkinClusterDataCmd_OBJECTS  := $(TOP)/exportSkinClusterDataCmd/exportSkinClusterDataCmd.o
exportSkinClusterDataCmd_PLUGIN   := $(DSTDIR)/exportSkinClusterDataCmd.$(EXT)
exportSkinClusterDataCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(exportSkinClusterDataCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(exportSkinClusterDataCmd_EXTRA_CFLAGS)
$(exportSkinClusterDataCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(exportSkinClusterDataCmd_EXTRA_C++FLAGS)
$(exportSkinClusterDataCmd_OBJECTS): INCLUDES := $(INCLUDES) $(exportSkinClusterDataCmd_EXTRA_INCLUDES)

depend_exportSkinClusterDataCmd:     INCLUDES := $(INCLUDES) $(exportSkinClusterDataCmd_EXTRA_INCLUDES)

$(exportSkinClusterDataCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(exportSkinClusterDataCmd_EXTRA_LFLAGS) 
$(exportSkinClusterDataCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(exportSkinClusterDataCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_exportSkinClusterDataCmd clean_exportSkinClusterDataCmd Clean_exportSkinClusterDataCmd


$(exportSkinClusterDataCmd_PLUGIN): $(exportSkinClusterDataCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_exportSkinClusterDataCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(exportSkinClusterDataCmd_SOURCES)

clean_exportSkinClusterDataCmd:
	-rm -f $(exportSkinClusterDataCmd_OBJECTS)

Clean_exportSkinClusterDataCmd:
	-rm -f $(exportSkinClusterDataCmd_MAKEFILE).bak $(exportSkinClusterDataCmd_OBJECTS) $(exportSkinClusterDataCmd_PLUGIN)


plugins: $(exportSkinClusterDataCmd_PLUGIN)
depend:	 depend_exportSkinClusterDataCmd
clean:	 clean_exportSkinClusterDataCmd
Clean:	 Clean_exportSkinClusterDataCmd

