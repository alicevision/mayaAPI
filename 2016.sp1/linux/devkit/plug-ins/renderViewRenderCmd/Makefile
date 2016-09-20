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

SRCDIR := $(TOP)/renderViewRenderCmd
DSTDIR := $(TOP)/renderViewRenderCmd

renderViewRenderCmd_SOURCES  := $(TOP)/renderViewRenderCmd/renderViewRenderCmd.cpp
renderViewRenderCmd_OBJECTS  := $(TOP)/renderViewRenderCmd/renderViewRenderCmd.o
renderViewRenderCmd_PLUGIN   := $(DSTDIR)/renderViewRenderCmd.$(EXT)
renderViewRenderCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(renderViewRenderCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(renderViewRenderCmd_EXTRA_CFLAGS)
$(renderViewRenderCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(renderViewRenderCmd_EXTRA_C++FLAGS)
$(renderViewRenderCmd_OBJECTS): INCLUDES := $(INCLUDES) $(renderViewRenderCmd_EXTRA_INCLUDES)

depend_renderViewRenderCmd:     INCLUDES := $(INCLUDES) $(renderViewRenderCmd_EXTRA_INCLUDES)

$(renderViewRenderCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(renderViewRenderCmd_EXTRA_LFLAGS) 
$(renderViewRenderCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMayaUI -lOpenMaya -lFoundation $(renderViewRenderCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_renderViewRenderCmd clean_renderViewRenderCmd Clean_renderViewRenderCmd


$(renderViewRenderCmd_PLUGIN): $(renderViewRenderCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_renderViewRenderCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(renderViewRenderCmd_SOURCES)

clean_renderViewRenderCmd:
	-rm -f $(renderViewRenderCmd_OBJECTS)

Clean_renderViewRenderCmd:
	-rm -f $(renderViewRenderCmd_MAKEFILE).bak $(renderViewRenderCmd_OBJECTS) $(renderViewRenderCmd_PLUGIN)


plugins: $(renderViewRenderCmd_PLUGIN)
depend:	 depend_renderViewRenderCmd
clean:	 clean_renderViewRenderCmd
Clean:	 Clean_renderViewRenderCmd

