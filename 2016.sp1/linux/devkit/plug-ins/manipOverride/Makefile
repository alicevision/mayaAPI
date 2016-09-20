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

SRCDIR := $(TOP)/manipOverride
DSTDIR := $(TOP)/manipOverride

manipOverride_SOURCES  := $(TOP)/manipOverride/manipOverride.cpp $(TOP)/manipOverride/customTriadManip.cpp $(TOP)/manipOverride/rockingTransform2.cpp
manipOverride_OBJECTS  := $(TOP)/manipOverride/manipOverride.o $(TOP)/manipOverride/customTriadManip.o $(TOP)/manipOverride/rockingTransform2.o
manipOverride_PLUGIN   := $(DSTDIR)/manipOverride.$(EXT)
manipOverride_MAKEFILE := $(DSTDIR)/Makefile

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

$(manipOverride_OBJECTS): CFLAGS   := $(CFLAGS)   $(manipOverride_EXTRA_CFLAGS)
$(manipOverride_OBJECTS): C++FLAGS := $(C++FLAGS) $(manipOverride_EXTRA_C++FLAGS)
$(manipOverride_OBJECTS): INCLUDES := $(INCLUDES) $(manipOverride_EXTRA_INCLUDES)

depend_manipOverride:     INCLUDES := $(INCLUDES) $(manipOverride_EXTRA_INCLUDES)

$(manipOverride_PLUGIN):  LFLAGS   := $(LFLAGS) $(manipOverride_EXTRA_LFLAGS) 
$(manipOverride_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(manipOverride_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_manipOverride clean_manipOverride Clean_manipOverride


$(manipOverride_PLUGIN): $(manipOverride_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_manipOverride :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(manipOverride_SOURCES)

clean_manipOverride:
	-rm -f $(manipOverride_OBJECTS)

Clean_manipOverride:
	-rm -f $(manipOverride_MAKEFILE).bak $(manipOverride_OBJECTS) $(manipOverride_PLUGIN)


plugins: $(manipOverride_PLUGIN)
depend:	 depend_manipOverride
clean:	 clean_manipOverride
Clean:	 Clean_manipOverride

