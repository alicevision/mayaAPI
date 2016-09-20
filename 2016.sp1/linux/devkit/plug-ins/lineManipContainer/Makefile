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

SRCDIR := $(TOP)/lineManipContainer
DSTDIR := $(TOP)/lineManipContainer

lineManipContainer_SOURCES  := $(TOP)/lineManipContainer/lineManipContainer.cpp $(TOP)/lineManipContainer/manipulatorMath.cpp
lineManipContainer_OBJECTS  := $(TOP)/lineManipContainer/lineManipContainer.o $(TOP)/lineManipContainer/manipulatorMath.o
lineManipContainer_PLUGIN   := $(DSTDIR)/lineManipContainer.$(EXT)
lineManipContainer_MAKEFILE := $(DSTDIR)/Makefile

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

$(lineManipContainer_OBJECTS): CFLAGS   := $(CFLAGS)   $(lineManipContainer_EXTRA_CFLAGS)
$(lineManipContainer_OBJECTS): C++FLAGS := $(C++FLAGS) $(lineManipContainer_EXTRA_C++FLAGS)
$(lineManipContainer_OBJECTS): INCLUDES := $(INCLUDES) $(lineManipContainer_EXTRA_INCLUDES)

depend_lineManipContainer:     INCLUDES := $(INCLUDES) $(lineManipContainer_EXTRA_INCLUDES)

$(lineManipContainer_PLUGIN):  LFLAGS   := $(LFLAGS) $(lineManipContainer_EXTRA_LFLAGS) 
$(lineManipContainer_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lOpenMayaUI -lOpenMayaRender $(lineManipContainer_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_lineManipContainer clean_lineManipContainer Clean_lineManipContainer


$(lineManipContainer_PLUGIN): $(lineManipContainer_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_lineManipContainer :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(lineManipContainer_SOURCES)

clean_lineManipContainer:
	-rm -f $(lineManipContainer_OBJECTS)

Clean_lineManipContainer:
	-rm -f $(lineManipContainer_MAKEFILE).bak $(lineManipContainer_OBJECTS) $(lineManipContainer_PLUGIN)


plugins: $(lineManipContainer_PLUGIN)
depend:	 depend_lineManipContainer
clean:	 clean_lineManipContainer
Clean:	 Clean_lineManipContainer

