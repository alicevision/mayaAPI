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

SRCDIR := $(TOP)/scanDagSyntax
DSTDIR := $(TOP)/scanDagSyntax

scanDagSyntax_SOURCES  := $(TOP)/scanDagSyntax/scanDagSyntax.cpp
scanDagSyntax_OBJECTS  := $(TOP)/scanDagSyntax/scanDagSyntax.o
scanDagSyntax_PLUGIN   := $(DSTDIR)/scanDagSyntax.$(EXT)
scanDagSyntax_MAKEFILE := $(DSTDIR)/Makefile

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

$(scanDagSyntax_OBJECTS): CFLAGS   := $(CFLAGS)   $(scanDagSyntax_EXTRA_CFLAGS)
$(scanDagSyntax_OBJECTS): C++FLAGS := $(C++FLAGS) $(scanDagSyntax_EXTRA_C++FLAGS)
$(scanDagSyntax_OBJECTS): INCLUDES := $(INCLUDES) $(scanDagSyntax_EXTRA_INCLUDES)

depend_scanDagSyntax:     INCLUDES := $(INCLUDES) $(scanDagSyntax_EXTRA_INCLUDES)

$(scanDagSyntax_PLUGIN):  LFLAGS   := $(LFLAGS) $(scanDagSyntax_EXTRA_LFLAGS) 
$(scanDagSyntax_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(scanDagSyntax_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_scanDagSyntax clean_scanDagSyntax Clean_scanDagSyntax


$(scanDagSyntax_PLUGIN): $(scanDagSyntax_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_scanDagSyntax :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(scanDagSyntax_SOURCES)

clean_scanDagSyntax:
	-rm -f $(scanDagSyntax_OBJECTS)

Clean_scanDagSyntax:
	-rm -f $(scanDagSyntax_MAKEFILE).bak $(scanDagSyntax_OBJECTS) $(scanDagSyntax_PLUGIN)


plugins: $(scanDagSyntax_PLUGIN)
depend:	 depend_scanDagSyntax
clean:	 clean_scanDagSyntax
Clean:	 Clean_scanDagSyntax

