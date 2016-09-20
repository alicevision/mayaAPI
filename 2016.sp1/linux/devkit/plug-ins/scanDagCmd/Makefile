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

SRCDIR := $(TOP)/scanDagCmd
DSTDIR := $(TOP)/scanDagCmd

scanDagCmd_SOURCES  := $(TOP)/scanDagCmd/scanDagCmd.cpp
scanDagCmd_OBJECTS  := $(TOP)/scanDagCmd/scanDagCmd.o
scanDagCmd_PLUGIN   := $(DSTDIR)/scanDagCmd.$(EXT)
scanDagCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(scanDagCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(scanDagCmd_EXTRA_CFLAGS)
$(scanDagCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(scanDagCmd_EXTRA_C++FLAGS)
$(scanDagCmd_OBJECTS): INCLUDES := $(INCLUDES) $(scanDagCmd_EXTRA_INCLUDES)

depend_scanDagCmd:     INCLUDES := $(INCLUDES) $(scanDagCmd_EXTRA_INCLUDES)

$(scanDagCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(scanDagCmd_EXTRA_LFLAGS) 
$(scanDagCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(scanDagCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_scanDagCmd clean_scanDagCmd Clean_scanDagCmd


$(scanDagCmd_PLUGIN): $(scanDagCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_scanDagCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(scanDagCmd_SOURCES)

clean_scanDagCmd:
	-rm -f $(scanDagCmd_OBJECTS)

Clean_scanDagCmd:
	-rm -f $(scanDagCmd_MAKEFILE).bak $(scanDagCmd_OBJECTS) $(scanDagCmd_PLUGIN)


plugins: $(scanDagCmd_PLUGIN)
depend:	 depend_scanDagCmd
clean:	 clean_scanDagCmd
Clean:	 Clean_scanDagCmd

