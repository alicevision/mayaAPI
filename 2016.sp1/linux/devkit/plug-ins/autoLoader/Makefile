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

SRCDIR := $(TOP)/autoLoader
DSTDIR := $(TOP)/autoLoader

autoLoader_SOURCES  := $(TOP)/autoLoader/autoLoader.cpp $(TOP)/autoLoader/moduleLogic.cpp $(TOP)/autoLoader/moduleLogicCmd.cpp $(TOP)/autoLoader/threadData.cpp
autoLoader_OBJECTS  := $(TOP)/autoLoader/autoLoader.o $(TOP)/autoLoader/moduleLogic.o $(TOP)/autoLoader/moduleLogicCmd.o $(TOP)/autoLoader/threadData.o
autoLoader_PLUGIN   := $(DSTDIR)/autoLoader.$(EXT)
autoLoader_MAKEFILE := $(DSTDIR)/Makefile

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

$(autoLoader_OBJECTS): CFLAGS   := $(CFLAGS)   $(autoLoader_EXTRA_CFLAGS)
$(autoLoader_OBJECTS): C++FLAGS := $(C++FLAGS) $(autoLoader_EXTRA_C++FLAGS)
$(autoLoader_OBJECTS): INCLUDES := $(INCLUDES) $(autoLoader_EXTRA_INCLUDES)

depend_autoLoader:     INCLUDES := $(INCLUDES) $(autoLoader_EXTRA_INCLUDES)

$(autoLoader_PLUGIN):  LFLAGS   := $(LFLAGS) $(autoLoader_EXTRA_LFLAGS) 
$(autoLoader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lxml2 $(autoLoader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_autoLoader clean_autoLoader Clean_autoLoader


$(autoLoader_PLUGIN): $(autoLoader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_autoLoader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(autoLoader_SOURCES)

clean_autoLoader:
	-rm -f $(autoLoader_OBJECTS)

Clean_autoLoader:
	-rm -f $(autoLoader_MAKEFILE).bak $(autoLoader_OBJECTS) $(autoLoader_PLUGIN)


plugins: $(autoLoader_PLUGIN)
depend:	 depend_autoLoader
clean:	 clean_autoLoader
Clean:	 Clean_autoLoader

