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

SRCDIR := $(TOP)/renderViewInteractiveRenderCmd
DSTDIR := $(TOP)/renderViewInteractiveRenderCmd

renderViewInteractiveRenderCmd_SOURCES  := $(TOP)/renderViewInteractiveRenderCmd/renderViewInteractiveRenderCmd.cpp
renderViewInteractiveRenderCmd_OBJECTS  := $(TOP)/renderViewInteractiveRenderCmd/renderViewInteractiveRenderCmd.o
renderViewInteractiveRenderCmd_PLUGIN   := $(DSTDIR)/renderViewInteractiveRenderCmd.$(EXT)
renderViewInteractiveRenderCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(renderViewInteractiveRenderCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(renderViewInteractiveRenderCmd_EXTRA_CFLAGS)
$(renderViewInteractiveRenderCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(renderViewInteractiveRenderCmd_EXTRA_C++FLAGS)
$(renderViewInteractiveRenderCmd_OBJECTS): INCLUDES := $(INCLUDES) $(renderViewInteractiveRenderCmd_EXTRA_INCLUDES)

depend_renderViewInteractiveRenderCmd:     INCLUDES := $(INCLUDES) $(renderViewInteractiveRenderCmd_EXTRA_INCLUDES)

$(renderViewInteractiveRenderCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(renderViewInteractiveRenderCmd_EXTRA_LFLAGS) 
$(renderViewInteractiveRenderCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMayaUI -lOpenMaya -lFoundation -lOpenMayaAnim $(renderViewInteractiveRenderCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_renderViewInteractiveRenderCmd clean_renderViewInteractiveRenderCmd Clean_renderViewInteractiveRenderCmd


$(renderViewInteractiveRenderCmd_PLUGIN): $(renderViewInteractiveRenderCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_renderViewInteractiveRenderCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(renderViewInteractiveRenderCmd_SOURCES)

clean_renderViewInteractiveRenderCmd:
	-rm -f $(renderViewInteractiveRenderCmd_OBJECTS)

Clean_renderViewInteractiveRenderCmd:
	-rm -f $(renderViewInteractiveRenderCmd_MAKEFILE).bak $(renderViewInteractiveRenderCmd_OBJECTS) $(renderViewInteractiveRenderCmd_PLUGIN)


plugins: $(renderViewInteractiveRenderCmd_PLUGIN)
depend:	 depend_renderViewInteractiveRenderCmd
clean:	 clean_renderViewInteractiveRenderCmd
Clean:	 Clean_renderViewInteractiveRenderCmd

