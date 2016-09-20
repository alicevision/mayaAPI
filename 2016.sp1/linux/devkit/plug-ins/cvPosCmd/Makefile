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

SRCDIR := $(TOP)/cvPosCmd
DSTDIR := $(TOP)/cvPosCmd

cvPosCmd_SOURCES  := $(TOP)/cvPosCmd/cvPosCmd.cpp
cvPosCmd_OBJECTS  := $(TOP)/cvPosCmd/cvPosCmd.o
cvPosCmd_PLUGIN   := $(DSTDIR)/cvPosCmd.$(EXT)
cvPosCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(cvPosCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(cvPosCmd_EXTRA_CFLAGS)
$(cvPosCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(cvPosCmd_EXTRA_C++FLAGS)
$(cvPosCmd_OBJECTS): INCLUDES := $(INCLUDES) $(cvPosCmd_EXTRA_INCLUDES)

depend_cvPosCmd:     INCLUDES := $(INCLUDES) $(cvPosCmd_EXTRA_INCLUDES)

$(cvPosCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(cvPosCmd_EXTRA_LFLAGS) 
$(cvPosCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(cvPosCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cvPosCmd clean_cvPosCmd Clean_cvPosCmd


$(cvPosCmd_PLUGIN): $(cvPosCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cvPosCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cvPosCmd_SOURCES)

clean_cvPosCmd:
	-rm -f $(cvPosCmd_OBJECTS)

Clean_cvPosCmd:
	-rm -f $(cvPosCmd_MAKEFILE).bak $(cvPosCmd_OBJECTS) $(cvPosCmd_PLUGIN)


plugins: $(cvPosCmd_PLUGIN)
depend:	 depend_cvPosCmd
clean:	 clean_cvPosCmd
Clean:	 Clean_cvPosCmd

