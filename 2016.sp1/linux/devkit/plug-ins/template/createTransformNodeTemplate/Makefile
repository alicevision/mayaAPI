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
TOP := ../..
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

SRCDIR := $(TOP)/template/createTransformNodeTemplate
DSTDIR := $(TOP)/template/createTransformNodeTemplate

createTransformNodeTemplate_SOURCES  := $(TOP)/template/createTransformNodeTemplate/createTransformNodeTemplate.cpp
createTransformNodeTemplate_OBJECTS  := $(TOP)/template/createTransformNodeTemplate/createTransformNodeTemplate.o
createTransformNodeTemplate_PLUGIN   := $(DSTDIR)/createTransformNodeTemplate.$(EXT)
createTransformNodeTemplate_MAKEFILE := $(DSTDIR)/Makefile

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

$(createTransformNodeTemplate_OBJECTS): CFLAGS   := $(CFLAGS)   $(createTransformNodeTemplate_EXTRA_CFLAGS)
$(createTransformNodeTemplate_OBJECTS): C++FLAGS := $(C++FLAGS) $(createTransformNodeTemplate_EXTRA_C++FLAGS)
$(createTransformNodeTemplate_OBJECTS): INCLUDES := $(INCLUDES) $(createTransformNodeTemplate_EXTRA_INCLUDES)

depend_createTransformNodeTemplate:     INCLUDES := $(INCLUDES) $(createTransformNodeTemplate_EXTRA_INCLUDES)

$(createTransformNodeTemplate_PLUGIN):  LFLAGS   := $(LFLAGS) $(createTransformNodeTemplate_EXTRA_LFLAGS) 
$(createTransformNodeTemplate_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(createTransformNodeTemplate_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_createTransformNodeTemplate clean_createTransformNodeTemplate Clean_createTransformNodeTemplate


$(createTransformNodeTemplate_PLUGIN): $(createTransformNodeTemplate_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_createTransformNodeTemplate :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(createTransformNodeTemplate_SOURCES)

clean_createTransformNodeTemplate:
	-rm -f $(createTransformNodeTemplate_OBJECTS)

Clean_createTransformNodeTemplate:
	-rm -f $(createTransformNodeTemplate_MAKEFILE).bak $(createTransformNodeTemplate_OBJECTS) $(createTransformNodeTemplate_PLUGIN)


plugins: $(createTransformNodeTemplate_PLUGIN)
depend:	 depend_createTransformNodeTemplate
clean:	 clean_createTransformNodeTemplate
Clean:	 Clean_createTransformNodeTemplate

