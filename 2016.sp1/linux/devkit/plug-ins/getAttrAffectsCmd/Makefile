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

SRCDIR := $(TOP)/getAttrAffectsCmd
DSTDIR := $(TOP)/getAttrAffectsCmd

getAttrAffectsCmd_SOURCES  := $(TOP)/getAttrAffectsCmd/getAttrAffectsCmd.cpp
getAttrAffectsCmd_OBJECTS  := $(TOP)/getAttrAffectsCmd/getAttrAffectsCmd.o
getAttrAffectsCmd_PLUGIN   := $(DSTDIR)/getAttrAffectsCmd.$(EXT)
getAttrAffectsCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(getAttrAffectsCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(getAttrAffectsCmd_EXTRA_CFLAGS)
$(getAttrAffectsCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(getAttrAffectsCmd_EXTRA_C++FLAGS)
$(getAttrAffectsCmd_OBJECTS): INCLUDES := $(INCLUDES) $(getAttrAffectsCmd_EXTRA_INCLUDES)

depend_getAttrAffectsCmd:     INCLUDES := $(INCLUDES) $(getAttrAffectsCmd_EXTRA_INCLUDES)

$(getAttrAffectsCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(getAttrAffectsCmd_EXTRA_LFLAGS) 
$(getAttrAffectsCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(getAttrAffectsCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_getAttrAffectsCmd clean_getAttrAffectsCmd Clean_getAttrAffectsCmd


$(getAttrAffectsCmd_PLUGIN): $(getAttrAffectsCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_getAttrAffectsCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(getAttrAffectsCmd_SOURCES)

clean_getAttrAffectsCmd:
	-rm -f $(getAttrAffectsCmd_OBJECTS)

Clean_getAttrAffectsCmd:
	-rm -f $(getAttrAffectsCmd_MAKEFILE).bak $(getAttrAffectsCmd_OBJECTS) $(getAttrAffectsCmd_PLUGIN)


plugins: $(getAttrAffectsCmd_PLUGIN)
depend:	 depend_getAttrAffectsCmd
clean:	 clean_getAttrAffectsCmd
Clean:	 Clean_getAttrAffectsCmd

