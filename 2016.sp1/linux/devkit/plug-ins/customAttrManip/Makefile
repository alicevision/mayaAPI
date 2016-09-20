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

SRCDIR := $(TOP)/customAttrManip
DSTDIR := $(TOP)/customAttrManip

customAttrManip_SOURCES  := $(TOP)/customAttrManip/customAttrManip.cpp
customAttrManip_OBJECTS  := $(TOP)/customAttrManip/customAttrManip.o
customAttrManip_PLUGIN   := $(DSTDIR)/customAttrManip.$(EXT)
customAttrManip_MAKEFILE := $(DSTDIR)/Makefile

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

$(customAttrManip_OBJECTS): CFLAGS   := $(CFLAGS)   $(customAttrManip_EXTRA_CFLAGS)
$(customAttrManip_OBJECTS): C++FLAGS := $(C++FLAGS) $(customAttrManip_EXTRA_C++FLAGS)
$(customAttrManip_OBJECTS): INCLUDES := $(INCLUDES) $(customAttrManip_EXTRA_INCLUDES)

depend_customAttrManip:     INCLUDES := $(INCLUDES) $(customAttrManip_EXTRA_INCLUDES)

$(customAttrManip_PLUGIN):  LFLAGS   := $(LFLAGS) $(customAttrManip_EXTRA_LFLAGS) 
$(customAttrManip_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaUI -lFoundation $(customAttrManip_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_customAttrManip clean_customAttrManip Clean_customAttrManip


$(customAttrManip_PLUGIN): $(customAttrManip_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_customAttrManip :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(customAttrManip_SOURCES)

clean_customAttrManip:
	-rm -f $(customAttrManip_OBJECTS)

Clean_customAttrManip:
	-rm -f $(customAttrManip_MAKEFILE).bak $(customAttrManip_OBJECTS) $(customAttrManip_PLUGIN)


plugins: $(customAttrManip_PLUGIN)
depend:	 depend_customAttrManip
clean:	 clean_customAttrManip
Clean:	 Clean_customAttrManip

