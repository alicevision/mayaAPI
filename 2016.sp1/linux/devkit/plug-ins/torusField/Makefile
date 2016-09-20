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

SRCDIR := $(TOP)/torusField
DSTDIR := $(TOP)/torusField

torusField_SOURCES  := $(TOP)/torusField/torusField.cpp
torusField_OBJECTS  := $(TOP)/torusField/torusField.o
torusField_PLUGIN   := $(DSTDIR)/torusField.$(EXT)
torusField_MAKEFILE := $(DSTDIR)/Makefile

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

$(torusField_OBJECTS): CFLAGS   := $(CFLAGS)   $(torusField_EXTRA_CFLAGS)
$(torusField_OBJECTS): C++FLAGS := $(C++FLAGS) $(torusField_EXTRA_C++FLAGS)
$(torusField_OBJECTS): INCLUDES := $(INCLUDES) $(torusField_EXTRA_INCLUDES)

depend_torusField:     INCLUDES := $(INCLUDES) $(torusField_EXTRA_INCLUDES)

$(torusField_PLUGIN):  LFLAGS   := $(LFLAGS) $(torusField_EXTRA_LFLAGS) 
$(torusField_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaFX -lOpenMayaUI -lFoundation -lGL -lGLU $(torusField_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_torusField clean_torusField Clean_torusField


$(torusField_PLUGIN): $(torusField_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_torusField :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(torusField_SOURCES)

clean_torusField:
	-rm -f $(torusField_OBJECTS)

Clean_torusField:
	-rm -f $(torusField_MAKEFILE).bak $(torusField_OBJECTS) $(torusField_PLUGIN)


plugins: $(torusField_PLUGIN)
depend:	 depend_torusField
clean:	 clean_torusField
Clean:	 Clean_torusField

