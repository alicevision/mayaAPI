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

SRCDIR := $(TOP)/clusterWeightFunction
DSTDIR := $(TOP)/clusterWeightFunction

clusterWeightFunction_SOURCES  := $(TOP)/clusterWeightFunction/clusterWeightFunction.cpp
clusterWeightFunction_OBJECTS  := $(TOP)/clusterWeightFunction/clusterWeightFunction.o
clusterWeightFunction_PLUGIN   := $(DSTDIR)/clusterWeightFunction.$(EXT)
clusterWeightFunction_MAKEFILE := $(DSTDIR)/Makefile

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

$(clusterWeightFunction_OBJECTS): CFLAGS   := $(CFLAGS)   $(clusterWeightFunction_EXTRA_CFLAGS)
$(clusterWeightFunction_OBJECTS): C++FLAGS := $(C++FLAGS) $(clusterWeightFunction_EXTRA_C++FLAGS)
$(clusterWeightFunction_OBJECTS): INCLUDES := $(INCLUDES) $(clusterWeightFunction_EXTRA_INCLUDES)

depend_clusterWeightFunction:     INCLUDES := $(INCLUDES) $(clusterWeightFunction_EXTRA_INCLUDES)

$(clusterWeightFunction_PLUGIN):  LFLAGS   := $(LFLAGS) $(clusterWeightFunction_EXTRA_LFLAGS) 
$(clusterWeightFunction_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaAnim -lFoundation $(clusterWeightFunction_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_clusterWeightFunction clean_clusterWeightFunction Clean_clusterWeightFunction


$(clusterWeightFunction_PLUGIN): $(clusterWeightFunction_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_clusterWeightFunction :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(clusterWeightFunction_SOURCES)

clean_clusterWeightFunction:
	-rm -f $(clusterWeightFunction_OBJECTS)

Clean_clusterWeightFunction:
	-rm -f $(clusterWeightFunction_MAKEFILE).bak $(clusterWeightFunction_OBJECTS) $(clusterWeightFunction_PLUGIN)


plugins: $(clusterWeightFunction_PLUGIN)
depend:	 depend_clusterWeightFunction
clean:	 clean_clusterWeightFunction
Clean:	 Clean_clusterWeightFunction

