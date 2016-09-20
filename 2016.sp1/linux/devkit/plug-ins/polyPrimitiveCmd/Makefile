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

SRCDIR := $(TOP)/polyPrimitiveCmd
DSTDIR := $(TOP)/polyPrimitiveCmd

polyPrimitiveCmd_SOURCES  := $(TOP)/polyPrimitiveCmd/polyPrimitiveCmd.cpp
polyPrimitiveCmd_OBJECTS  := $(TOP)/polyPrimitiveCmd/polyPrimitiveCmd.o
polyPrimitiveCmd_PLUGIN   := $(DSTDIR)/polyPrimitiveCmd.$(EXT)
polyPrimitiveCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(polyPrimitiveCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(polyPrimitiveCmd_EXTRA_CFLAGS)
$(polyPrimitiveCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(polyPrimitiveCmd_EXTRA_C++FLAGS)
$(polyPrimitiveCmd_OBJECTS): INCLUDES := $(INCLUDES) $(polyPrimitiveCmd_EXTRA_INCLUDES)

depend_polyPrimitiveCmd:     INCLUDES := $(INCLUDES) $(polyPrimitiveCmd_EXTRA_INCLUDES)

$(polyPrimitiveCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(polyPrimitiveCmd_EXTRA_LFLAGS) 
$(polyPrimitiveCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(polyPrimitiveCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_polyPrimitiveCmd clean_polyPrimitiveCmd Clean_polyPrimitiveCmd


$(polyPrimitiveCmd_PLUGIN): $(polyPrimitiveCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_polyPrimitiveCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(polyPrimitiveCmd_SOURCES)

clean_polyPrimitiveCmd:
	-rm -f $(polyPrimitiveCmd_OBJECTS)

Clean_polyPrimitiveCmd:
	-rm -f $(polyPrimitiveCmd_MAKEFILE).bak $(polyPrimitiveCmd_OBJECTS) $(polyPrimitiveCmd_PLUGIN)


plugins: $(polyPrimitiveCmd_PLUGIN)
depend:	 depend_polyPrimitiveCmd
clean:	 clean_polyPrimitiveCmd
Clean:	 Clean_polyPrimitiveCmd

