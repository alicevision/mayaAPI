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

SRCDIR := $(TOP)/cvExpandCmd
DSTDIR := $(TOP)/cvExpandCmd

cvExpandCmd_SOURCES  := $(TOP)/cvExpandCmd/cvExpandCmd.cpp
cvExpandCmd_OBJECTS  := $(TOP)/cvExpandCmd/cvExpandCmd.o
cvExpandCmd_PLUGIN   := $(DSTDIR)/cvExpandCmd.$(EXT)
cvExpandCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(cvExpandCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(cvExpandCmd_EXTRA_CFLAGS)
$(cvExpandCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(cvExpandCmd_EXTRA_C++FLAGS)
$(cvExpandCmd_OBJECTS): INCLUDES := $(INCLUDES) $(cvExpandCmd_EXTRA_INCLUDES)

depend_cvExpandCmd:     INCLUDES := $(INCLUDES) $(cvExpandCmd_EXTRA_INCLUDES)

$(cvExpandCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(cvExpandCmd_EXTRA_LFLAGS) 
$(cvExpandCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(cvExpandCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cvExpandCmd clean_cvExpandCmd Clean_cvExpandCmd


$(cvExpandCmd_PLUGIN): $(cvExpandCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cvExpandCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cvExpandCmd_SOURCES)

clean_cvExpandCmd:
	-rm -f $(cvExpandCmd_OBJECTS)

Clean_cvExpandCmd:
	-rm -f $(cvExpandCmd_MAKEFILE).bak $(cvExpandCmd_OBJECTS) $(cvExpandCmd_PLUGIN)


plugins: $(cvExpandCmd_PLUGIN)
depend:	 depend_cvExpandCmd
clean:	 clean_cvExpandCmd
Clean:	 Clean_cvExpandCmd

