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

SRCDIR := $(TOP)/nodeCreatedCBCmd
DSTDIR := $(TOP)/nodeCreatedCBCmd

nodeCreatedCBCmd_SOURCES  := $(TOP)/nodeCreatedCBCmd/nodeCreatedCBCmd.cpp
nodeCreatedCBCmd_OBJECTS  := $(TOP)/nodeCreatedCBCmd/nodeCreatedCBCmd.o
nodeCreatedCBCmd_PLUGIN   := $(DSTDIR)/nodeCreatedCBCmd.$(EXT)
nodeCreatedCBCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(nodeCreatedCBCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(nodeCreatedCBCmd_EXTRA_CFLAGS)
$(nodeCreatedCBCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(nodeCreatedCBCmd_EXTRA_C++FLAGS)
$(nodeCreatedCBCmd_OBJECTS): INCLUDES := $(INCLUDES) $(nodeCreatedCBCmd_EXTRA_INCLUDES)

depend_nodeCreatedCBCmd:     INCLUDES := $(INCLUDES) $(nodeCreatedCBCmd_EXTRA_INCLUDES)

$(nodeCreatedCBCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(nodeCreatedCBCmd_EXTRA_LFLAGS) 
$(nodeCreatedCBCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(nodeCreatedCBCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_nodeCreatedCBCmd clean_nodeCreatedCBCmd Clean_nodeCreatedCBCmd


$(nodeCreatedCBCmd_PLUGIN): $(nodeCreatedCBCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_nodeCreatedCBCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(nodeCreatedCBCmd_SOURCES)

clean_nodeCreatedCBCmd:
	-rm -f $(nodeCreatedCBCmd_OBJECTS)

Clean_nodeCreatedCBCmd:
	-rm -f $(nodeCreatedCBCmd_MAKEFILE).bak $(nodeCreatedCBCmd_OBJECTS) $(nodeCreatedCBCmd_PLUGIN)


plugins: $(nodeCreatedCBCmd_PLUGIN)
depend:	 depend_nodeCreatedCBCmd
clean:	 clean_nodeCreatedCBCmd
Clean:	 Clean_nodeCreatedCBCmd

