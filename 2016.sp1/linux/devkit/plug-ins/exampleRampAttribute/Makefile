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

SRCDIR := $(TOP)/exampleRampAttribute
DSTDIR := $(TOP)/exampleRampAttribute

exampleRampAttribute_SOURCES  := $(TOP)/exampleRampAttribute/exampleMRampAttribute.cpp
exampleRampAttribute_OBJECTS  := $(TOP)/exampleRampAttribute/exampleMRampAttribute.o
exampleRampAttribute_PLUGIN   := $(DSTDIR)/exampleRampAttribute.$(EXT)
exampleRampAttribute_MAKEFILE := $(DSTDIR)/Makefile

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

$(exampleRampAttribute_OBJECTS): CFLAGS   := $(CFLAGS)   $(exampleRampAttribute_EXTRA_CFLAGS)
$(exampleRampAttribute_OBJECTS): C++FLAGS := $(C++FLAGS) $(exampleRampAttribute_EXTRA_C++FLAGS)
$(exampleRampAttribute_OBJECTS): INCLUDES := $(INCLUDES) $(exampleRampAttribute_EXTRA_INCLUDES)

depend_exampleRampAttribute:     INCLUDES := $(INCLUDES) $(exampleRampAttribute_EXTRA_INCLUDES)

$(exampleRampAttribute_PLUGIN):  LFLAGS   := $(LFLAGS) $(exampleRampAttribute_EXTRA_LFLAGS) 
$(exampleRampAttribute_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(exampleRampAttribute_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_exampleRampAttribute clean_exampleRampAttribute Clean_exampleRampAttribute


$(exampleRampAttribute_PLUGIN): $(exampleRampAttribute_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_exampleRampAttribute :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(exampleRampAttribute_SOURCES)

clean_exampleRampAttribute:
	-rm -f $(exampleRampAttribute_OBJECTS)

Clean_exampleRampAttribute:
	-rm -f $(exampleRampAttribute_MAKEFILE).bak $(exampleRampAttribute_OBJECTS) $(exampleRampAttribute_PLUGIN)


plugins: $(exampleRampAttribute_PLUGIN)
depend:	 depend_exampleRampAttribute
clean:	 clean_exampleRampAttribute
Clean:	 Clean_exampleRampAttribute

