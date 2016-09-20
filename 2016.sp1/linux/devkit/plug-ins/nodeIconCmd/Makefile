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

SRCDIR := $(TOP)/nodeIconCmd
DSTDIR := $(TOP)/nodeIconCmd

nodeIconCmd_SOURCES  := $(TOP)/nodeIconCmd/nodeIconCmd.cpp
nodeIconCmd_OBJECTS  := $(TOP)/nodeIconCmd/nodeIconCmd.o
nodeIconCmd_PLUGIN   := $(DSTDIR)/nodeIconCmd.$(EXT)
nodeIconCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(nodeIconCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(nodeIconCmd_EXTRA_CFLAGS)
$(nodeIconCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(nodeIconCmd_EXTRA_C++FLAGS)
$(nodeIconCmd_OBJECTS): INCLUDES := $(INCLUDES) $(nodeIconCmd_EXTRA_INCLUDES)

depend_nodeIconCmd:     INCLUDES := $(INCLUDES) $(nodeIconCmd_EXTRA_INCLUDES)

$(nodeIconCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(nodeIconCmd_EXTRA_LFLAGS) 
$(nodeIconCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(nodeIconCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_nodeIconCmd clean_nodeIconCmd Clean_nodeIconCmd


$(nodeIconCmd_PLUGIN): $(nodeIconCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_nodeIconCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(nodeIconCmd_SOURCES)

clean_nodeIconCmd:
	-rm -f $(nodeIconCmd_OBJECTS)

Clean_nodeIconCmd:
	-rm -f $(nodeIconCmd_MAKEFILE).bak $(nodeIconCmd_OBJECTS) $(nodeIconCmd_PLUGIN)


plugins: $(nodeIconCmd_PLUGIN)
depend:	 depend_nodeIconCmd
clean:	 clean_nodeIconCmd
Clean:	 Clean_nodeIconCmd

