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

SRCDIR := $(TOP)/userMsgCmd
DSTDIR := $(TOP)/userMsgCmd

userMsgCmd_SOURCES  := $(TOP)/userMsgCmd/userMsgCmd.cpp
userMsgCmd_OBJECTS  := $(TOP)/userMsgCmd/userMsgCmd.o
userMsgCmd_PLUGIN   := $(DSTDIR)/userMsgCmd.$(EXT)
userMsgCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(userMsgCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(userMsgCmd_EXTRA_CFLAGS)
$(userMsgCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(userMsgCmd_EXTRA_C++FLAGS)
$(userMsgCmd_OBJECTS): INCLUDES := $(INCLUDES) $(userMsgCmd_EXTRA_INCLUDES)

depend_userMsgCmd:     INCLUDES := $(INCLUDES) $(userMsgCmd_EXTRA_INCLUDES)

$(userMsgCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(userMsgCmd_EXTRA_LFLAGS) 
$(userMsgCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(userMsgCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_userMsgCmd clean_userMsgCmd Clean_userMsgCmd


$(userMsgCmd_PLUGIN): $(userMsgCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_userMsgCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(userMsgCmd_SOURCES)

clean_userMsgCmd:
	-rm -f $(userMsgCmd_OBJECTS)

Clean_userMsgCmd:
	-rm -f $(userMsgCmd_MAKEFILE).bak $(userMsgCmd_OBJECTS) $(userMsgCmd_PLUGIN)


plugins: $(userMsgCmd_PLUGIN)
depend:	 depend_userMsgCmd
clean:	 clean_userMsgCmd
Clean:	 Clean_userMsgCmd

