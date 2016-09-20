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

SRCDIR := $(TOP)/MayaPluginForSpreticle
DSTDIR := $(TOP)/MayaPluginForSpreticle

MayaPluginForSpreticle_SOURCES  := $(TOP)/MayaPluginForSpreticle/spReticleLoc.cpp
MayaPluginForSpreticle_OBJECTS  := $(TOP)/MayaPluginForSpreticle/spReticleLoc.o
MayaPluginForSpreticle_PLUGIN   := $(DSTDIR)/MayaPluginForSpreticle.$(EXT)
MayaPluginForSpreticle_MAKEFILE := $(DSTDIR)/Makefile

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

$(MayaPluginForSpreticle_OBJECTS): CFLAGS   := $(CFLAGS)   $(MayaPluginForSpreticle_EXTRA_CFLAGS)
$(MayaPluginForSpreticle_OBJECTS): C++FLAGS := $(C++FLAGS) $(MayaPluginForSpreticle_EXTRA_C++FLAGS)
$(MayaPluginForSpreticle_OBJECTS): INCLUDES := $(INCLUDES) $(MayaPluginForSpreticle_EXTRA_INCLUDES)

depend_MayaPluginForSpreticle:     INCLUDES := $(INCLUDES) $(MayaPluginForSpreticle_EXTRA_INCLUDES)

$(MayaPluginForSpreticle_PLUGIN):  LFLAGS   := $(LFLAGS) $(MayaPluginForSpreticle_EXTRA_LFLAGS) 
$(MayaPluginForSpreticle_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL $(MayaPluginForSpreticle_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_MayaPluginForSpreticle clean_MayaPluginForSpreticle Clean_MayaPluginForSpreticle


$(MayaPluginForSpreticle_PLUGIN): $(MayaPluginForSpreticle_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_MayaPluginForSpreticle :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(MayaPluginForSpreticle_SOURCES)

clean_MayaPluginForSpreticle:
	-rm -f $(MayaPluginForSpreticle_OBJECTS)

Clean_MayaPluginForSpreticle:
	-rm -f $(MayaPluginForSpreticle_MAKEFILE).bak $(MayaPluginForSpreticle_OBJECTS) $(MayaPluginForSpreticle_PLUGIN)


plugins: $(MayaPluginForSpreticle_PLUGIN)
depend:	 depend_MayaPluginForSpreticle
clean:	 clean_MayaPluginForSpreticle
Clean:	 Clean_MayaPluginForSpreticle

