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

SRCDIR := $(TOP)/cvColorShader
DSTDIR := $(TOP)/cvColorShader

cvColorShader_SOURCES  := $(TOP)/cvColorShader/cvColorShader.cpp
cvColorShader_OBJECTS  := $(TOP)/cvColorShader/cvColorShader.o
cvColorShader_PLUGIN   := $(DSTDIR)/cvColorShader.$(EXT)
cvColorShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(cvColorShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(cvColorShader_EXTRA_CFLAGS)
$(cvColorShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(cvColorShader_EXTRA_C++FLAGS)
$(cvColorShader_OBJECTS): INCLUDES := $(INCLUDES) $(cvColorShader_EXTRA_INCLUDES)

depend_cvColorShader:     INCLUDES := $(INCLUDES) $(cvColorShader_EXTRA_INCLUDES)

$(cvColorShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(cvColorShader_EXTRA_LFLAGS) 
$(cvColorShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMayaRender -lOpenMaya -lFoundation $(cvColorShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_cvColorShader clean_cvColorShader Clean_cvColorShader


$(cvColorShader_PLUGIN): $(cvColorShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_cvColorShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(cvColorShader_SOURCES)

clean_cvColorShader:
	-rm -f $(cvColorShader_OBJECTS)

Clean_cvColorShader:
	-rm -f $(cvColorShader_MAKEFILE).bak $(cvColorShader_OBJECTS) $(cvColorShader_PLUGIN)


plugins: $(cvColorShader_PLUGIN)
depend:	 depend_cvColorShader
clean:	 clean_cvColorShader
Clean:	 Clean_cvColorShader

