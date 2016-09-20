#-
# ==========================================================================
# Copyright 2012 Autodesk, Inc. All rights reserved.
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

SRCDIR = .
LD		= $(MAYA_LOCATION)/bin/mayald MAYA_LOCATION=$(MAYA_LOCATION)
TARGETS		= helloWorld surfaceCreate surfaceTwist readAndWrite asciiToBinary

.SUFFIXES: .cpp .cc .o .c

.cc.o:
	$(CXX) -c $(C++FLAGS) $(INCLUDES) $<

.cpp.o:
	$(CXX) -c $(C++FLAGS) $(INCLUDES) $<

all: $(TARGETS)

helloWorld: helloWorld.o
	$(LD) -o $@ $@.o

surfaceCreate: surfaceCreate.o
	$(LD) -o $@ $@.o

surfaceTwist: surfaceTwist.o
	$(LD) -o $@ $@.o

readAndWrite: readAndWrite.o
	$(LD) -o $@ $@.o

asciiToBinary: asciiToBinary.o
	$(LD) -o $@ $@.o

clean:
	rm -f *.o

Clean:
	rm -f $(TARGETS) *.o *.bak
