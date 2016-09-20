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

SRCDIR := $(TOP)/renderViewRenderRegionCmd
DSTDIR := $(TOP)/renderViewRenderRegionCmd

renderViewRenderRegionCmd_SOURCES  := $(TOP)/renderViewRenderRegionCmd/renderViewRenderRegionCmd.cpp
renderViewRenderRegionCmd_OBJECTS  := $(TOP)/renderViewRenderRegionCmd/renderViewRenderRegionCmd.o
renderViewRenderRegionCmd_PLUGIN   := $(DSTDIR)/renderViewRenderRegionCmd.$(EXT)
renderViewRenderRegionCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(renderViewRenderRegionCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(renderViewRenderRegionCmd_EXTRA_CFLAGS)
$(renderViewRenderRegionCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(renderViewRenderRegionCmd_EXTRA_C++FLAGS)
$(renderViewRenderRegionCmd_OBJECTS): INCLUDES := $(INCLUDES) $(renderViewRenderRegionCmd_EXTRA_INCLUDES)

depend_renderViewRenderRegionCmd:     INCLUDES := $(INCLUDES) $(renderViewRenderRegionCmd_EXTRA_INCLUDES)

$(renderViewRenderRegionCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(renderViewRenderRegionCmd_EXTRA_LFLAGS) 
$(renderViewRenderRegionCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMayaUI -lOpenMaya -lFoundation $(renderViewRenderRegionCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_renderViewRenderRegionCmd clean_renderViewRenderRegionCmd Clean_renderViewRenderRegionCmd


$(renderViewRenderRegionCmd_PLUGIN): $(renderViewRenderRegionCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_renderViewRenderRegionCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(renderViewRenderRegionCmd_SOURCES)

clean_renderViewRenderRegionCmd:
	-rm -f $(renderViewRenderRegionCmd_OBJECTS)

Clean_renderViewRenderRegionCmd:
	-rm -f $(renderViewRenderRegionCmd_MAKEFILE).bak $(renderViewRenderRegionCmd_OBJECTS) $(renderViewRenderRegionCmd_PLUGIN)


plugins: $(renderViewRenderRegionCmd_PLUGIN)
depend:	 depend_renderViewRenderRegionCmd
clean:	 clean_renderViewRenderRegionCmd
Clean:	 Clean_renderViewRenderRegionCmd

