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

SRCDIR := $(TOP)/nodeMessageCmd
DSTDIR := $(TOP)/nodeMessageCmd

nodeMessageCmd_SOURCES  := $(TOP)/nodeMessageCmd/nodeMessageCmd.cpp
nodeMessageCmd_OBJECTS  := $(TOP)/nodeMessageCmd/nodeMessageCmd.o
nodeMessageCmd_PLUGIN   := $(DSTDIR)/nodeMessageCmd.$(EXT)
nodeMessageCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(nodeMessageCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(nodeMessageCmd_EXTRA_CFLAGS)
$(nodeMessageCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(nodeMessageCmd_EXTRA_C++FLAGS)
$(nodeMessageCmd_OBJECTS): INCLUDES := $(INCLUDES) $(nodeMessageCmd_EXTRA_INCLUDES)

depend_nodeMessageCmd:     INCLUDES := $(INCLUDES) $(nodeMessageCmd_EXTRA_INCLUDES)

$(nodeMessageCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(nodeMessageCmd_EXTRA_LFLAGS) 
$(nodeMessageCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(nodeMessageCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_nodeMessageCmd clean_nodeMessageCmd Clean_nodeMessageCmd


$(nodeMessageCmd_PLUGIN): $(nodeMessageCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_nodeMessageCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(nodeMessageCmd_SOURCES)

clean_nodeMessageCmd:
	-rm -f $(nodeMessageCmd_OBJECTS)

Clean_nodeMessageCmd:
	-rm -f $(nodeMessageCmd_MAKEFILE).bak $(nodeMessageCmd_OBJECTS) $(nodeMessageCmd_PLUGIN)


plugins: $(nodeMessageCmd_PLUGIN)
depend:	 depend_nodeMessageCmd
clean:	 clean_nodeMessageCmd
Clean:	 Clean_nodeMessageCmd

