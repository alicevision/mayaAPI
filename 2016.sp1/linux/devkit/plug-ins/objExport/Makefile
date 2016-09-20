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
all : prebuiltPlugins

endif

#
# Variable definitions
#

SRCDIR := $(TOP)/objExport
DSTDIR := $(TOP)/objExport

objExport_SOURCES  := $(TOP)/objExport/objExport.cpp
objExport_OBJECTS  := $(TOP)/objExport/objExport.o
objExport_PLUGIN   := $(DSTDIR)/objExport.$(EXT)
objExport_MAKEFILE := $(DSTDIR)/Makefile

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

$(objExport_OBJECTS): CFLAGS   := $(CFLAGS)   $(objExport_EXTRA_CFLAGS)
$(objExport_OBJECTS): C++FLAGS := $(C++FLAGS) $(objExport_EXTRA_C++FLAGS)
$(objExport_OBJECTS): INCLUDES := $(INCLUDES) $(objExport_EXTRA_INCLUDES)

depend_objExport:     INCLUDES := $(INCLUDES) $(objExport_EXTRA_INCLUDES)

$(objExport_PLUGIN):  LFLAGS   := $(LFLAGS) $(objExport_EXTRA_LFLAGS) 
$(objExport_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation $(objExport_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_objExport clean_objExport Clean_objExport


$(objExport_PLUGIN): $(objExport_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_objExport :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(objExport_SOURCES)

clean_objExport:
	-rm -f $(objExport_OBJECTS)

Clean_objExport:
	-rm -f $(objExport_MAKEFILE).bak $(objExport_OBJECTS) $(objExport_PLUGIN)


prebuiltPlugins: $(objExport_PLUGIN)
depend:	 depend_objExport
clean:	 clean_objExport
Clean:	 Clean_objExport

