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

SRCDIR := $(TOP)/listLightLinksCmd
DSTDIR := $(TOP)/listLightLinksCmd

listLightLinksCmd_SOURCES  := $(TOP)/listLightLinksCmd/listLightLinksCmd.cpp
listLightLinksCmd_OBJECTS  := $(TOP)/listLightLinksCmd/listLightLinksCmd.o
listLightLinksCmd_PLUGIN   := $(DSTDIR)/listLightLinksCmd.$(EXT)
listLightLinksCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(listLightLinksCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(listLightLinksCmd_EXTRA_CFLAGS)
$(listLightLinksCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(listLightLinksCmd_EXTRA_C++FLAGS)
$(listLightLinksCmd_OBJECTS): INCLUDES := $(INCLUDES) $(listLightLinksCmd_EXTRA_INCLUDES)

depend_listLightLinksCmd:     INCLUDES := $(INCLUDES) $(listLightLinksCmd_EXTRA_INCLUDES)

$(listLightLinksCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(listLightLinksCmd_EXTRA_LFLAGS) 
$(listLightLinksCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMaya -lFoundation $(listLightLinksCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_listLightLinksCmd clean_listLightLinksCmd Clean_listLightLinksCmd


$(listLightLinksCmd_PLUGIN): $(listLightLinksCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_listLightLinksCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(listLightLinksCmd_SOURCES)

clean_listLightLinksCmd:
	-rm -f $(listLightLinksCmd_OBJECTS)

Clean_listLightLinksCmd:
	-rm -f $(listLightLinksCmd_MAKEFILE).bak $(listLightLinksCmd_OBJECTS) $(listLightLinksCmd_PLUGIN)


plugins: $(listLightLinksCmd_PLUGIN)
depend:	 depend_listLightLinksCmd
clean:	 clean_listLightLinksCmd
Clean:	 Clean_listLightLinksCmd

