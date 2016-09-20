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

SRCDIR := $(TOP)/vp2BlinnShader
DSTDIR := $(TOP)/vp2BlinnShader

vp2BlinnShader_SOURCES  := $(TOP)/vp2BlinnShader/vp2BlinnShader.cpp
vp2BlinnShader_OBJECTS  := $(TOP)/vp2BlinnShader/vp2BlinnShader.o
vp2BlinnShader_PLUGIN   := $(DSTDIR)/vp2BlinnShader.$(EXT)
vp2BlinnShader_MAKEFILE := $(DSTDIR)/Makefile

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

$(vp2BlinnShader_OBJECTS): CFLAGS   := $(CFLAGS)   $(vp2BlinnShader_EXTRA_CFLAGS)
$(vp2BlinnShader_OBJECTS): C++FLAGS := $(C++FLAGS) $(vp2BlinnShader_EXTRA_C++FLAGS)
$(vp2BlinnShader_OBJECTS): INCLUDES := $(INCLUDES) $(vp2BlinnShader_EXTRA_INCLUDES)

depend_vp2BlinnShader:     INCLUDES := $(INCLUDES) $(vp2BlinnShader_EXTRA_INCLUDES)

$(vp2BlinnShader_PLUGIN):  LFLAGS   := $(LFLAGS) $(vp2BlinnShader_EXTRA_LFLAGS) 
$(vp2BlinnShader_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lOpenMayaRender -lFoundation -lGL -lGLU $(vp2BlinnShader_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_vp2BlinnShader clean_vp2BlinnShader Clean_vp2BlinnShader


$(vp2BlinnShader_PLUGIN): $(vp2BlinnShader_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_vp2BlinnShader :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(vp2BlinnShader_SOURCES)

clean_vp2BlinnShader:
	-rm -f $(vp2BlinnShader_OBJECTS)

Clean_vp2BlinnShader:
	-rm -f $(vp2BlinnShader_MAKEFILE).bak $(vp2BlinnShader_OBJECTS) $(vp2BlinnShader_PLUGIN)


plugins: $(vp2BlinnShader_PLUGIN)
depend:	 depend_vp2BlinnShader
clean:	 clean_vp2BlinnShader
Clean:	 Clean_vp2BlinnShader

