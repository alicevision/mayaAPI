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

SRCDIR := $(TOP)/undoRedoMsgCmd
DSTDIR := $(TOP)/undoRedoMsgCmd

undoRedoMsgCmd_SOURCES  := $(TOP)/undoRedoMsgCmd/undoRedoMsgCmd.cpp
undoRedoMsgCmd_OBJECTS  := $(TOP)/undoRedoMsgCmd/undoRedoMsgCmd.o
undoRedoMsgCmd_PLUGIN   := $(DSTDIR)/undoRedoMsgCmd.$(EXT)
undoRedoMsgCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(undoRedoMsgCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(undoRedoMsgCmd_EXTRA_CFLAGS)
$(undoRedoMsgCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(undoRedoMsgCmd_EXTRA_C++FLAGS)
$(undoRedoMsgCmd_OBJECTS): INCLUDES := $(INCLUDES) $(undoRedoMsgCmd_EXTRA_INCLUDES)

depend_undoRedoMsgCmd:     INCLUDES := $(INCLUDES) $(undoRedoMsgCmd_EXTRA_INCLUDES)

$(undoRedoMsgCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(undoRedoMsgCmd_EXTRA_LFLAGS) 
$(undoRedoMsgCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(undoRedoMsgCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_undoRedoMsgCmd clean_undoRedoMsgCmd Clean_undoRedoMsgCmd


$(undoRedoMsgCmd_PLUGIN): $(undoRedoMsgCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_undoRedoMsgCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(undoRedoMsgCmd_SOURCES)

clean_undoRedoMsgCmd:
	-rm -f $(undoRedoMsgCmd_OBJECTS)

Clean_undoRedoMsgCmd:
	-rm -f $(undoRedoMsgCmd_MAKEFILE).bak $(undoRedoMsgCmd_OBJECTS) $(undoRedoMsgCmd_PLUGIN)


plugins: $(undoRedoMsgCmd_PLUGIN)
depend:	 depend_undoRedoMsgCmd
clean:	 clean_undoRedoMsgCmd
Clean:	 Clean_undoRedoMsgCmd

