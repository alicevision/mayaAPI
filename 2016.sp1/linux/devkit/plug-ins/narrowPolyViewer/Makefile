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

SRCDIR := $(TOP)/narrowPolyViewer
DSTDIR := $(TOP)/narrowPolyViewer

narrowPolyViewer_SOURCES  := $(TOP)/narrowPolyViewer/narrowPolyViewer.cpp $(TOP)/narrowPolyViewer/narrowPolyViewerCmd.cpp $(TOP)/narrowPolyViewer/narrowPolyViewerMain.cpp
narrowPolyViewer_OBJECTS  := $(TOP)/narrowPolyViewer/narrowPolyViewer.o $(TOP)/narrowPolyViewer/narrowPolyViewerCmd.o $(TOP)/narrowPolyViewer/narrowPolyViewerMain.o
narrowPolyViewer_PLUGIN   := $(DSTDIR)/narrowPolyViewer.$(EXT)
narrowPolyViewer_MAKEFILE := $(DSTDIR)/Makefile

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

$(narrowPolyViewer_OBJECTS): CFLAGS   := $(CFLAGS)   $(narrowPolyViewer_EXTRA_CFLAGS)
$(narrowPolyViewer_OBJECTS): C++FLAGS := $(C++FLAGS) $(narrowPolyViewer_EXTRA_C++FLAGS)
$(narrowPolyViewer_OBJECTS): INCLUDES := $(INCLUDES) $(narrowPolyViewer_EXTRA_INCLUDES)

depend_narrowPolyViewer:     INCLUDES := $(INCLUDES) $(narrowPolyViewer_EXTRA_INCLUDES)

$(narrowPolyViewer_PLUGIN):  LFLAGS   := $(LFLAGS) $(narrowPolyViewer_EXTRA_LFLAGS) 
$(narrowPolyViewer_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation -lGL -lGLU $(narrowPolyViewer_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_narrowPolyViewer clean_narrowPolyViewer Clean_narrowPolyViewer


$(narrowPolyViewer_PLUGIN): $(narrowPolyViewer_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_narrowPolyViewer :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(narrowPolyViewer_SOURCES)

clean_narrowPolyViewer:
	-rm -f $(narrowPolyViewer_OBJECTS)

Clean_narrowPolyViewer:
	-rm -f $(narrowPolyViewer_MAKEFILE).bak $(narrowPolyViewer_OBJECTS) $(narrowPolyViewer_PLUGIN)


plugins: $(narrowPolyViewer_PLUGIN)
depend:	 depend_narrowPolyViewer
clean:	 clean_narrowPolyViewer
Clean:	 Clean_narrowPolyViewer

