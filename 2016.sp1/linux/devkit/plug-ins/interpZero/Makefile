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

SRCDIR := $(TOP)/interpZero
DSTDIR := $(TOP)/interpZero

interpZero_SOURCES  := $(TOP)/interpZero/interpZero.cpp
interpZero_OBJECTS  := $(TOP)/interpZero/interpZero.o
interpZero_PLUGIN   := $(DSTDIR)/interpZero.$(EXT)
interpZero_MAKEFILE := $(DSTDIR)/Makefile

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

$(interpZero_OBJECTS): CFLAGS   := $(CFLAGS)   $(interpZero_EXTRA_CFLAGS)
$(interpZero_OBJECTS): C++FLAGS := $(C++FLAGS) $(interpZero_EXTRA_C++FLAGS)
$(interpZero_OBJECTS): INCLUDES := $(INCLUDES) $(interpZero_EXTRA_INCLUDES)

depend_interpZero:     INCLUDES := $(INCLUDES) $(interpZero_EXTRA_INCLUDES)

$(interpZero_PLUGIN):  LFLAGS   := $(LFLAGS) $(interpZero_EXTRA_LFLAGS) 
$(interpZero_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaAnim -lOpenMaya -lFoundation $(interpZero_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_interpZero clean_interpZero Clean_interpZero


$(interpZero_PLUGIN): $(interpZero_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_interpZero :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(interpZero_SOURCES)

clean_interpZero:
	-rm -f $(interpZero_OBJECTS)

Clean_interpZero:
	-rm -f $(interpZero_MAKEFILE).bak $(interpZero_OBJECTS) $(interpZero_PLUGIN)


plugins: $(interpZero_PLUGIN)
depend:	 depend_interpZero
clean:	 clean_interpZero
Clean:	 Clean_interpZero

