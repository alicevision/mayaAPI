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

SRCDIR := $(TOP)/viewImageBlitOverride
DSTDIR := $(TOP)/viewImageBlitOverride

viewImageBlitOverride_SOURCES  := $(TOP)/viewImageBlitOverride/viewImageBlitOverride.cpp $(TOP)/viewImageBlitOverride/viewImageBlitOverrideMain.cpp
viewImageBlitOverride_OBJECTS  := $(TOP)/viewImageBlitOverride/viewImageBlitOverride.o $(TOP)/viewImageBlitOverride/viewImageBlitOverrideMain.o
viewImageBlitOverride_PLUGIN   := $(DSTDIR)/viewImageBlitOverride.$(EXT)
viewImageBlitOverride_MAKEFILE := $(DSTDIR)/Makefile

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

$(viewImageBlitOverride_OBJECTS): CFLAGS   := $(CFLAGS)   $(viewImageBlitOverride_EXTRA_CFLAGS)
$(viewImageBlitOverride_OBJECTS): C++FLAGS := $(C++FLAGS) $(viewImageBlitOverride_EXTRA_C++FLAGS)
$(viewImageBlitOverride_OBJECTS): INCLUDES := $(INCLUDES) $(viewImageBlitOverride_EXTRA_INCLUDES)

depend_viewImageBlitOverride:     INCLUDES := $(INCLUDES) $(viewImageBlitOverride_EXTRA_INCLUDES)

$(viewImageBlitOverride_PLUGIN):  LFLAGS   := $(LFLAGS) $(viewImageBlitOverride_EXTRA_LFLAGS) 
$(viewImageBlitOverride_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation $(viewImageBlitOverride_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_viewImageBlitOverride clean_viewImageBlitOverride Clean_viewImageBlitOverride


$(viewImageBlitOverride_PLUGIN): $(viewImageBlitOverride_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_viewImageBlitOverride :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(viewImageBlitOverride_SOURCES)

clean_viewImageBlitOverride:
	-rm -f $(viewImageBlitOverride_OBJECTS)

Clean_viewImageBlitOverride:
	-rm -f $(viewImageBlitOverride_MAKEFILE).bak $(viewImageBlitOverride_OBJECTS) $(viewImageBlitOverride_PLUGIN)


plugins: $(viewImageBlitOverride_PLUGIN)
depend:	 depend_viewImageBlitOverride
clean:	 clean_viewImageBlitOverride
Clean:	 Clean_viewImageBlitOverride

