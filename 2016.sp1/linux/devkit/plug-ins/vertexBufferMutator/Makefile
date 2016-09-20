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

SRCDIR := $(TOP)/vertexBufferMutator
DSTDIR := $(TOP)/vertexBufferMutator

vertexBufferMutator_SOURCES  := $(TOP)/vertexBufferMutator/vertexBufferMutator.cpp
vertexBufferMutator_OBJECTS  := $(TOP)/vertexBufferMutator/vertexBufferMutator.o
vertexBufferMutator_PLUGIN   := $(DSTDIR)/vertexBufferMutator.$(EXT)
vertexBufferMutator_MAKEFILE := $(DSTDIR)/Makefile

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

$(vertexBufferMutator_OBJECTS): CFLAGS   := $(CFLAGS)   $(vertexBufferMutator_EXTRA_CFLAGS)
$(vertexBufferMutator_OBJECTS): C++FLAGS := $(C++FLAGS) $(vertexBufferMutator_EXTRA_C++FLAGS)
$(vertexBufferMutator_OBJECTS): INCLUDES := $(INCLUDES) $(vertexBufferMutator_EXTRA_INCLUDES)

depend_vertexBufferMutator:     INCLUDES := $(INCLUDES) $(vertexBufferMutator_EXTRA_INCLUDES)

$(vertexBufferMutator_PLUGIN):  LFLAGS   := $(LFLAGS) $(vertexBufferMutator_EXTRA_LFLAGS) 
$(vertexBufferMutator_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lOpenMayaRender -lFoundation $(vertexBufferMutator_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_vertexBufferMutator clean_vertexBufferMutator Clean_vertexBufferMutator


$(vertexBufferMutator_PLUGIN): $(vertexBufferMutator_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_vertexBufferMutator :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(vertexBufferMutator_SOURCES)

clean_vertexBufferMutator:
	-rm -f $(vertexBufferMutator_OBJECTS)

Clean_vertexBufferMutator:
	-rm -f $(vertexBufferMutator_MAKEFILE).bak $(vertexBufferMutator_OBJECTS) $(vertexBufferMutator_PLUGIN)


plugins: $(vertexBufferMutator_PLUGIN)
depend:	 depend_vertexBufferMutator
clean:	 clean_vertexBufferMutator
Clean:	 Clean_vertexBufferMutator

