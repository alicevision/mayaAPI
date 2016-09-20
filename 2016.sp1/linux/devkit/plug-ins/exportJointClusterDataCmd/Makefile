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

SRCDIR := $(TOP)/exportJointClusterDataCmd
DSTDIR := $(TOP)/exportJointClusterDataCmd

exportJointClusterDataCmd_SOURCES  := $(TOP)/exportJointClusterDataCmd/exportJointClusterDataCmd.cpp
exportJointClusterDataCmd_OBJECTS  := $(TOP)/exportJointClusterDataCmd/exportJointClusterDataCmd.o
exportJointClusterDataCmd_PLUGIN   := $(DSTDIR)/exportJointClusterDataCmd.$(EXT)
exportJointClusterDataCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(exportJointClusterDataCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(exportJointClusterDataCmd_EXTRA_CFLAGS)
$(exportJointClusterDataCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(exportJointClusterDataCmd_EXTRA_C++FLAGS)
$(exportJointClusterDataCmd_OBJECTS): INCLUDES := $(INCLUDES) $(exportJointClusterDataCmd_EXTRA_INCLUDES)

depend_exportJointClusterDataCmd:     INCLUDES := $(INCLUDES) $(exportJointClusterDataCmd_EXTRA_INCLUDES)

$(exportJointClusterDataCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(exportJointClusterDataCmd_EXTRA_LFLAGS) 
$(exportJointClusterDataCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaAnim -lOpenMaya -lFoundation $(exportJointClusterDataCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_exportJointClusterDataCmd clean_exportJointClusterDataCmd Clean_exportJointClusterDataCmd


$(exportJointClusterDataCmd_PLUGIN): $(exportJointClusterDataCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_exportJointClusterDataCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(exportJointClusterDataCmd_SOURCES)

clean_exportJointClusterDataCmd:
	-rm -f $(exportJointClusterDataCmd_OBJECTS)

Clean_exportJointClusterDataCmd:
	-rm -f $(exportJointClusterDataCmd_MAKEFILE).bak $(exportJointClusterDataCmd_OBJECTS) $(exportJointClusterDataCmd_PLUGIN)


plugins: $(exportJointClusterDataCmd_PLUGIN)
depend:	 depend_exportJointClusterDataCmd
clean:	 clean_exportJointClusterDataCmd
Clean:	 Clean_exportJointClusterDataCmd

