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
TOP := ../..
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

SRCDIR := $(TOP)/template/helloTemplateWorld
DSTDIR := $(TOP)/template/helloTemplateWorld

helloTemplateWorld_SOURCES  := $(TOP)/template/helloTemplateWorld/helloTemplateWorld.cpp
helloTemplateWorld_OBJECTS  := $(TOP)/template/helloTemplateWorld/helloTemplateWorld.o
helloTemplateWorld_PLUGIN   := $(DSTDIR)/helloTemplateWorld.$(EXT)
helloTemplateWorld_MAKEFILE := $(DSTDIR)/Makefile

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

$(helloTemplateWorld_OBJECTS): CFLAGS   := $(CFLAGS)   $(helloTemplateWorld_EXTRA_CFLAGS)
$(helloTemplateWorld_OBJECTS): C++FLAGS := $(C++FLAGS) $(helloTemplateWorld_EXTRA_C++FLAGS)
$(helloTemplateWorld_OBJECTS): INCLUDES := $(INCLUDES) $(helloTemplateWorld_EXTRA_INCLUDES)

depend_helloTemplateWorld:     INCLUDES := $(INCLUDES) $(helloTemplateWorld_EXTRA_INCLUDES)

$(helloTemplateWorld_PLUGIN):  LFLAGS   := $(LFLAGS) $(helloTemplateWorld_EXTRA_LFLAGS) 
$(helloTemplateWorld_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(helloTemplateWorld_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_helloTemplateWorld clean_helloTemplateWorld Clean_helloTemplateWorld


$(helloTemplateWorld_PLUGIN): $(helloTemplateWorld_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_helloTemplateWorld :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(helloTemplateWorld_SOURCES)

clean_helloTemplateWorld:
	-rm -f $(helloTemplateWorld_OBJECTS)

Clean_helloTemplateWorld:
	-rm -f $(helloTemplateWorld_MAKEFILE).bak $(helloTemplateWorld_OBJECTS) $(helloTemplateWorld_PLUGIN)


plugins: $(helloTemplateWorld_PLUGIN)
depend:	 depend_helloTemplateWorld
clean:	 clean_helloTemplateWorld
Clean:	 Clean_helloTemplateWorld

