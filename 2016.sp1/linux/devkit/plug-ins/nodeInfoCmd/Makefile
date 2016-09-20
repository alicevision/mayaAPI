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

SRCDIR := $(TOP)/nodeInfoCmd
DSTDIR := $(TOP)/nodeInfoCmd

nodeInfoCmd_SOURCES  := $(TOP)/nodeInfoCmd/nodeInfoCmd.cpp
nodeInfoCmd_OBJECTS  := $(TOP)/nodeInfoCmd/nodeInfoCmd.o
nodeInfoCmd_PLUGIN   := $(DSTDIR)/nodeInfoCmd.$(EXT)
nodeInfoCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(nodeInfoCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(nodeInfoCmd_EXTRA_CFLAGS)
$(nodeInfoCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(nodeInfoCmd_EXTRA_C++FLAGS)
$(nodeInfoCmd_OBJECTS): INCLUDES := $(INCLUDES) $(nodeInfoCmd_EXTRA_INCLUDES)

depend_nodeInfoCmd:     INCLUDES := $(INCLUDES) $(nodeInfoCmd_EXTRA_INCLUDES)

$(nodeInfoCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(nodeInfoCmd_EXTRA_LFLAGS) 
$(nodeInfoCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(nodeInfoCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_nodeInfoCmd clean_nodeInfoCmd Clean_nodeInfoCmd


$(nodeInfoCmd_PLUGIN): $(nodeInfoCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_nodeInfoCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(nodeInfoCmd_SOURCES)

clean_nodeInfoCmd:
	-rm -f $(nodeInfoCmd_OBJECTS)

Clean_nodeInfoCmd:
	-rm -f $(nodeInfoCmd_MAKEFILE).bak $(nodeInfoCmd_OBJECTS) $(nodeInfoCmd_PLUGIN)


plugins: $(nodeInfoCmd_PLUGIN)
depend:	 depend_nodeInfoCmd
clean:	 clean_nodeInfoCmd
Clean:	 Clean_nodeInfoCmd

