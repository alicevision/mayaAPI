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

SRCDIR := $(TOP)/externalDropCallback
DSTDIR := $(TOP)/externalDropCallback

externalDropCallback_SOURCES  := $(TOP)/externalDropCallback/externalDropCallback.cpp
externalDropCallback_OBJECTS  := $(TOP)/externalDropCallback/externalDropCallback.o
externalDropCallback_PLUGIN   := $(DSTDIR)/externalDropCallback.$(EXT)
externalDropCallback_MAKEFILE := $(DSTDIR)/Makefile

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

$(externalDropCallback_OBJECTS): CFLAGS   := $(CFLAGS)   $(externalDropCallback_EXTRA_CFLAGS)
$(externalDropCallback_OBJECTS): C++FLAGS := $(C++FLAGS) $(externalDropCallback_EXTRA_C++FLAGS)
$(externalDropCallback_OBJECTS): INCLUDES := $(INCLUDES) $(externalDropCallback_EXTRA_INCLUDES)

depend_externalDropCallback:     INCLUDES := $(INCLUDES) $(externalDropCallback_EXTRA_INCLUDES)

$(externalDropCallback_PLUGIN):  LFLAGS   := $(LFLAGS) $(externalDropCallback_EXTRA_LFLAGS) 
$(externalDropCallback_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(externalDropCallback_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_externalDropCallback clean_externalDropCallback Clean_externalDropCallback


$(externalDropCallback_PLUGIN): $(externalDropCallback_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_externalDropCallback :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(externalDropCallback_SOURCES)

clean_externalDropCallback:
	-rm -f $(externalDropCallback_OBJECTS)

Clean_externalDropCallback:
	-rm -f $(externalDropCallback_MAKEFILE).bak $(externalDropCallback_OBJECTS) $(externalDropCallback_PLUGIN)


plugins: $(externalDropCallback_PLUGIN)
depend:	 depend_externalDropCallback
clean:	 clean_externalDropCallback
Clean:	 Clean_externalDropCallback

