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

SRCDIR := $(TOP)/listPolyHolesCmd
DSTDIR := $(TOP)/listPolyHolesCmd

listPolyHolesCmd_SOURCES  := $(TOP)/listPolyHolesCmd/listPolyHolesCmd.cpp
listPolyHolesCmd_OBJECTS  := $(TOP)/listPolyHolesCmd/listPolyHolesCmd.o
listPolyHolesCmd_PLUGIN   := $(DSTDIR)/listPolyHolesCmd.$(EXT)
listPolyHolesCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(listPolyHolesCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(listPolyHolesCmd_EXTRA_CFLAGS)
$(listPolyHolesCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(listPolyHolesCmd_EXTRA_C++FLAGS)
$(listPolyHolesCmd_OBJECTS): INCLUDES := $(INCLUDES) $(listPolyHolesCmd_EXTRA_INCLUDES)

depend_listPolyHolesCmd:     INCLUDES := $(INCLUDES) $(listPolyHolesCmd_EXTRA_INCLUDES)

$(listPolyHolesCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(listPolyHolesCmd_EXTRA_LFLAGS) 
$(listPolyHolesCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(listPolyHolesCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_listPolyHolesCmd clean_listPolyHolesCmd Clean_listPolyHolesCmd


$(listPolyHolesCmd_PLUGIN): $(listPolyHolesCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_listPolyHolesCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(listPolyHolesCmd_SOURCES)

clean_listPolyHolesCmd:
	-rm -f $(listPolyHolesCmd_OBJECTS)

Clean_listPolyHolesCmd:
	-rm -f $(listPolyHolesCmd_MAKEFILE).bak $(listPolyHolesCmd_OBJECTS) $(listPolyHolesCmd_PLUGIN)


plugins: $(listPolyHolesCmd_PLUGIN)
depend:	 depend_listPolyHolesCmd
clean:	 clean_listPolyHolesCmd
Clean:	 Clean_listPolyHolesCmd

