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

SRCDIR := $(TOP)/instanceCallbackCmd
DSTDIR := $(TOP)/instanceCallbackCmd

instanceCallbackCmd_SOURCES  := $(TOP)/instanceCallbackCmd/instanceCallbackCmd.cpp
instanceCallbackCmd_OBJECTS  := $(TOP)/instanceCallbackCmd/instanceCallbackCmd.o
instanceCallbackCmd_PLUGIN   := $(DSTDIR)/instanceCallbackCmd.$(EXT)
instanceCallbackCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(instanceCallbackCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(instanceCallbackCmd_EXTRA_CFLAGS)
$(instanceCallbackCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(instanceCallbackCmd_EXTRA_C++FLAGS)
$(instanceCallbackCmd_OBJECTS): INCLUDES := $(INCLUDES) $(instanceCallbackCmd_EXTRA_INCLUDES)

depend_instanceCallbackCmd:     INCLUDES := $(INCLUDES) $(instanceCallbackCmd_EXTRA_INCLUDES)

$(instanceCallbackCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(instanceCallbackCmd_EXTRA_LFLAGS) 
$(instanceCallbackCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(instanceCallbackCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_instanceCallbackCmd clean_instanceCallbackCmd Clean_instanceCallbackCmd


$(instanceCallbackCmd_PLUGIN): $(instanceCallbackCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_instanceCallbackCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(instanceCallbackCmd_SOURCES)

clean_instanceCallbackCmd:
	-rm -f $(instanceCallbackCmd_OBJECTS)

Clean_instanceCallbackCmd:
	-rm -f $(instanceCallbackCmd_MAKEFILE).bak $(instanceCallbackCmd_OBJECTS) $(instanceCallbackCmd_PLUGIN)


plugins: $(instanceCallbackCmd_PLUGIN)
depend:	 depend_instanceCallbackCmd
clean:	 clean_instanceCallbackCmd
Clean:	 Clean_instanceCallbackCmd

