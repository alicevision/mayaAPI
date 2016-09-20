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

SRCDIR := $(TOP)/instancerListCmd
DSTDIR := $(TOP)/instancerListCmd

instancerListCmd_SOURCES  := $(TOP)/instancerListCmd/instancerListCmd.cpp
instancerListCmd_OBJECTS  := $(TOP)/instancerListCmd/instancerListCmd.o
instancerListCmd_PLUGIN   := $(DSTDIR)/instancerListCmd.$(EXT)
instancerListCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(instancerListCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(instancerListCmd_EXTRA_CFLAGS)
$(instancerListCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(instancerListCmd_EXTRA_C++FLAGS)
$(instancerListCmd_OBJECTS): INCLUDES := $(INCLUDES) $(instancerListCmd_EXTRA_INCLUDES)

depend_instancerListCmd:     INCLUDES := $(INCLUDES) $(instancerListCmd_EXTRA_INCLUDES)

$(instancerListCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(instancerListCmd_EXTRA_LFLAGS) 
$(instancerListCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaFX $(instancerListCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_instancerListCmd clean_instancerListCmd Clean_instancerListCmd


$(instancerListCmd_PLUGIN): $(instancerListCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_instancerListCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(instancerListCmd_SOURCES)

clean_instancerListCmd:
	-rm -f $(instancerListCmd_OBJECTS)

Clean_instancerListCmd:
	-rm -f $(instancerListCmd_MAKEFILE).bak $(instancerListCmd_OBJECTS) $(instancerListCmd_PLUGIN)


plugins: $(instancerListCmd_PLUGIN)
depend:	 depend_instancerListCmd
clean:	 clean_instancerListCmd
Clean:	 Clean_instancerListCmd

