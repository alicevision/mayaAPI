#-
# ==========================================================================
# Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.
# ==========================================================================
#+

#
# Include platform specific build settings
#
include ../plug-ins/buildconfig

SRCDIR	= .
LIBS	+= -lImage -lIMFbase -lc 

# IMF plugins export a different set of global symbols than normal Maya API
# plug-ins, so they need their own symbol map.
ifneq ($(findstring linux_plugin.map,$(LFLAGS)),)
	LFLAGS := $(filter-out %linux_plugin.map,$(LFLAGS)) -Wl,--version-script=imf_plugin.map
endif

PLATFORM_SPECIFIC_PLUGINS = $(LINUX_PLUGINS)

.SUFFIXES: .cpp .cc .o .so .lib .c

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<
	
.cc.o:
	$(C++) -c $(INCLUDES) $(C++FLAGS) $<

.cpp.o:
	$(C++) -c $(INCLUDES) $(C++FLAGS) $<

.cc.i:
	$(C++) -E $(INCLUDES) $(C++FLAGS) $*.cc > $*.i

.cc.so:
	-rm -f $@
	$(LD) -o $@ $(INCLUDES) $(LFLAGS) $< $(LIBS)

.cpp.so:
	-rm -f $@
	$(LD) -o $@ $(INCLUDES) $(LFLAGS) $< $(LIBS)

.o.so:
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $< $(LIBS)
    
.o.lib:
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $< $(LIBS)

plugins: xmplImageFormat.$(EXT) 

#
# Specific rules
#

xmplImageFormat.$(EXT): xmplImageFormat.o
	-rm -f $@
	$(LD) -o $@ $(LFLAGS) $? $(LIBS)


