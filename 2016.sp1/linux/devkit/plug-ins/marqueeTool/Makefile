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

SRCDIR := $(TOP)/marqueeTool
DSTDIR := $(TOP)/marqueeTool

marqueeTool_SOURCES  := $(TOP)/marqueeTool/marqueeTool.cpp
marqueeTool_OBJECTS  := $(TOP)/marqueeTool/marqueeTool.o
marqueeTool_PLUGIN   := $(DSTDIR)/marqueeTool.$(EXT)
marqueeTool_MAKEFILE := $(DSTDIR)/Makefile

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

$(marqueeTool_OBJECTS): CFLAGS   := $(CFLAGS)   $(marqueeTool_EXTRA_CFLAGS)
$(marqueeTool_OBJECTS): C++FLAGS := $(C++FLAGS) $(marqueeTool_EXTRA_C++FLAGS)
$(marqueeTool_OBJECTS): INCLUDES := $(INCLUDES) $(marqueeTool_EXTRA_INCLUDES)

depend_marqueeTool:     INCLUDES := $(INCLUDES) $(marqueeTool_EXTRA_INCLUDES)

$(marqueeTool_PLUGIN):  LFLAGS   := $(LFLAGS) $(marqueeTool_EXTRA_LFLAGS) 
$(marqueeTool_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(marqueeTool_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_marqueeTool clean_marqueeTool Clean_marqueeTool


$(marqueeTool_PLUGIN): $(marqueeTool_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_marqueeTool :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(marqueeTool_SOURCES)

clean_marqueeTool:
	-rm -f $(marqueeTool_OBJECTS)

Clean_marqueeTool:
	-rm -f $(marqueeTool_MAKEFILE).bak $(marqueeTool_OBJECTS) $(marqueeTool_PLUGIN)


plugins: $(marqueeTool_PLUGIN)
depend:	 depend_marqueeTool
clean:	 clean_marqueeTool
Clean:	 Clean_marqueeTool

