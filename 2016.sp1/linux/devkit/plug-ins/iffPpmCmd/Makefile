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

SRCDIR := $(TOP)/iffPpmCmd
DSTDIR := $(TOP)/iffPpmCmd

iffPpmCmd_SOURCES  := $(TOP)/iffPpmCmd/iffPpmCmd.cpp $(TOP)/iffPpmCmd/iffreader.cpp
iffPpmCmd_OBJECTS  := $(TOP)/iffPpmCmd/iffPpmCmd.o $(TOP)/iffPpmCmd/iffreader.o
iffPpmCmd_PLUGIN   := $(DSTDIR)/iffPpmCmd.$(EXT)
iffPpmCmd_MAKEFILE := $(DSTDIR)/Makefile

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

$(iffPpmCmd_OBJECTS): CFLAGS   := $(CFLAGS)   $(iffPpmCmd_EXTRA_CFLAGS)
$(iffPpmCmd_OBJECTS): C++FLAGS := $(C++FLAGS) $(iffPpmCmd_EXTRA_C++FLAGS)
$(iffPpmCmd_OBJECTS): INCLUDES := $(INCLUDES) $(iffPpmCmd_EXTRA_INCLUDES)

depend_iffPpmCmd:     INCLUDES := $(INCLUDES) $(iffPpmCmd_EXTRA_INCLUDES)

$(iffPpmCmd_PLUGIN):  LFLAGS   := $(LFLAGS) $(iffPpmCmd_EXTRA_LFLAGS) 
$(iffPpmCmd_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lImage -lFoundation $(iffPpmCmd_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_iffPpmCmd clean_iffPpmCmd Clean_iffPpmCmd


$(iffPpmCmd_PLUGIN): $(iffPpmCmd_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_iffPpmCmd :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(iffPpmCmd_SOURCES)

clean_iffPpmCmd:
	-rm -f $(iffPpmCmd_OBJECTS)

Clean_iffPpmCmd:
	-rm -f $(iffPpmCmd_MAKEFILE).bak $(iffPpmCmd_OBJECTS) $(iffPpmCmd_PLUGIN)


plugins: $(iffPpmCmd_PLUGIN)
depend:	 depend_iffPpmCmd
clean:	 clean_iffPpmCmd
Clean:	 Clean_iffPpmCmd

