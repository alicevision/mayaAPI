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

SRCDIR := $(TOP)/MetadataXML
DSTDIR := $(TOP)/MetadataXML

MetadataXML_SOURCES  := $(TOP)/MetadataXML/metadataXMLPlugin.cpp $(TOP)/MetadataXML/metadataXML.cpp $(TOP)/MetadataXML/associationsSerializerXML.cpp $(TOP)/MetadataXML/channelSerializerXML.cpp $(TOP)/MetadataXML/streamSerializerXML.cpp $(TOP)/MetadataXML/structureSerializerXML.cpp
MetadataXML_OBJECTS  := $(TOP)/MetadataXML/metadataXMLPlugin.o $(TOP)/MetadataXML/metadataXML.o $(TOP)/MetadataXML/associationsSerializerXML.o $(TOP)/MetadataXML/channelSerializerXML.o $(TOP)/MetadataXML/streamSerializerXML.o $(TOP)/MetadataXML/structureSerializerXML.o
MetadataXML_PLUGIN   := $(DSTDIR)/MetadataXML.$(EXT)
MetadataXML_MAKEFILE := $(DSTDIR)/Makefile

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

$(MetadataXML_OBJECTS): CFLAGS   := $(CFLAGS)   $(MetadataXML_EXTRA_CFLAGS)
$(MetadataXML_OBJECTS): C++FLAGS := $(C++FLAGS) $(MetadataXML_EXTRA_C++FLAGS)
$(MetadataXML_OBJECTS): INCLUDES := $(INCLUDES) $(MetadataXML_EXTRA_INCLUDES)

depend_MetadataXML:     INCLUDES := $(INCLUDES) $(MetadataXML_EXTRA_INCLUDES)

$(MetadataXML_PLUGIN):  LFLAGS   := $(LFLAGS) $(MetadataXML_EXTRA_LFLAGS) 
$(MetadataXML_PLUGIN):  LIBS     := $(LIBS)   -lOpenMaya -lFoundation -lMetaData -lxml2 $(MetadataXML_EXTRA_LIBS) 

#
# Rules definitions
#

.PHONY: depend_MetadataXML clean_MetadataXML Clean_MetadataXML


$(MetadataXML_PLUGIN): $(MetadataXML_OBJECTS) 
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $^ $(LIBS)

depend_MetadataXML :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(DSTDIR)/Makefile $(MetadataXML_SOURCES)

clean_MetadataXML:
	-rm -f $(MetadataXML_OBJECTS)

Clean_MetadataXML:
	-rm -f $(MetadataXML_MAKEFILE).bak $(MetadataXML_OBJECTS) $(MetadataXML_PLUGIN)


plugins: $(MetadataXML_PLUGIN)
depend:	 depend_MetadataXML
clean:	 clean_MetadataXML
Clean:	 Clean_MetadataXML

