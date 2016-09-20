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

SRCDIR := $(TOP)/MetadataSample
DSTDIR := $(TOP)/MetadataSample

MetadataSample_SOURCES  := $(TOP)/MetadataSample/metadataPlugin.cpp $(TOP)/MetadataSample/metadataBase.cpp $(TOP)/MetadataSample/createMetadataCmd.cpp $(TOP)/MetadataSample/exportMetadataCmd.cpp $(TOP)/MetadataSample/importMetadataCmd.cpp $(TOP)/MetadataSample/tweakMetadataNode.cpp
MetadataSample_OBJECTS  := $(TOP)/MetadataSample/metadataPlugin.o $(TOP)/MetadataSample/metadataBase.o $(TOP)/MetadataSample/createMetadataCmd.o $(TOP)/MetadataSample/exportMetadataCmd.o $(TOP)/MetadataSample/importMetadataCmd.o $(TOP)/MetadataSample/tweakMetadataNode.o
MetadataSample_PLUGIN   := $(DSTDIR)/MetadataSample.$(EXT)
MetadataSample_MAKEFILE := $(DSTDIR)/Makefile

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

$(MetadataSample_OBJECTS): CFLAGS   := $(CFLAGS)   $(MetadataSample_EXTRA_CFLAGS)
$(MetadataSample_OBJECTS): C++FLAGS := $(C++FLAGS) $(MetadataSample_EXTRA_C++FLAGS)
$(MetadataSample_OBJECTS): INCLUDES := $(INCLUDES) $(MetadataSample_EXTRA_INCLUDES)

depend_MetadataSample:     INCLUDES := $(INCLUDES) $(MetadataSample_EXTRA_INCLUDES)

$(MetadataSample_PLUGIN):  LFLAGS   := $(LFLAGS) $(MetadataSample_EXTRA_LFLAGS) 
$(MetadataSample_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lMetaData $(MetadataSample_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_MetadataSample clean_MetadataSample Clean_MetadataSample


$(MetadataSample_PLUGIN): $(MetadataSample_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_MetadataSample :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(MetadataSample_SOURCES)

clean_MetadataSample:
	-rm -f $(MetadataSample_OBJECTS)

Clean_MetadataSample:
	-rm -f $(MetadataSample_MAKEFILE).bak $(MetadataSample_OBJECTS) $(MetadataSample_PLUGIN)


plugins: $(MetadataSample_PLUGIN)
depend:	 depend_MetadataSample
clean:	 clean_MetadataSample
Clean:	 Clean_MetadataSample

