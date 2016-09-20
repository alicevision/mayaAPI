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

SRCDIR := $(TOP)/helloCmd
DSTDIR := $(TOP)/helloCmd

helloCmd_SOURCES  := $(TOP)/helloCmd/helloCmd.cpp
helloCmd_OBJECTS  := $(TOP)/helloCmd/helloCmd.o
helloCmd_PLUGIN   := $(DSTDIR)/helloCmd.$(EXT)
helloCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(helloCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(helloCmd_EXTRA_CFLAGS)
$(helloCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(helloCmd_EXTRA_C++FLAGS)
$(helloCmd_OBJECTS): INCLUDES := $(INCLUDES) $(helloCmd_EXTRA_INCLUDES)

depend_helloCmd:     INCLUDES := $(INCLUDES) $(helloCmd_EXTRA_INCLUDES)

$(helloCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(helloCmd_EXTRA_LFLAGS) 
$(helloCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(helloCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_helloCmd clean_helloCmd Clean_helloCmd


$(helloCmd_PLUGIN): $(helloCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_helloCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(helloCmd_SOURCES)

clean_helloCmd:
	-rm -f $(helloCmd_OBJECTS)

Clean_helloCmd:
	-rm -f $(helloCmd_MAKEFILE).bak $(helloCmd_OBJECTS) $(helloCmd_PLUGIN)


plugins: $(helloCmd_PLUGIN)
depend:	 depend_helloCmd
clean:	 clean_helloCmd
Clean:	 Clean_helloCmd

