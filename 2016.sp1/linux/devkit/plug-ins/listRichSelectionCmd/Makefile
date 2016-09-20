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

SRCDIR := $(TOP)/listRichSelectionCmd
DSTDIR := $(TOP)/listRichSelectionCmd

listRichSelectionCmd_SOURCES  := $(TOP)/listRichSelectionCmd/listRichSelectionCmd.cpp
listRichSelectionCmd_OBJECTS  := $(TOP)/listRichSelectionCmd/listRichSelectionCmd.o
listRichSelectionCmd_PLUGIN   := $(DSTDIR)/listRichSelectionCmd.$(EXT)
listRichSelectionCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(listRichSelectionCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(listRichSelectionCmd_EXTRA_CFLAGS)
$(listRichSelectionCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(listRichSelectionCmd_EXTRA_C++FLAGS)
$(listRichSelectionCmd_OBJECTS): INCLUDES := $(INCLUDES) $(listRichSelectionCmd_EXTRA_INCLUDES)

depend_listRichSelectionCmd:     INCLUDES := $(INCLUDES) $(listRichSelectionCmd_EXTRA_INCLUDES)

$(listRichSelectionCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(listRichSelectionCmd_EXTRA_LFLAGS) 
$(listRichSelectionCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(listRichSelectionCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_listRichSelectionCmd clean_listRichSelectionCmd Clean_listRichSelectionCmd


$(listRichSelectionCmd_PLUGIN): $(listRichSelectionCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_listRichSelectionCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(listRichSelectionCmd_SOURCES)

clean_listRichSelectionCmd:
	-rm -f $(listRichSelectionCmd_OBJECTS)

Clean_listRichSelectionCmd:
	-rm -f $(listRichSelectionCmd_MAKEFILE).bak $(listRichSelectionCmd_OBJECTS) $(listRichSelectionCmd_PLUGIN)


plugins: $(listRichSelectionCmd_PLUGIN)
depend:	 depend_listRichSelectionCmd
clean:	 clean_listRichSelectionCmd
Clean:	 Clean_listRichSelectionCmd

