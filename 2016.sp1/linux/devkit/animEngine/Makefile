#! make

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

CC			= cc
CFLAGS		= -g -n32

INCLUDES = -I.
LD	 = $(CC) $(CFLAGS)
LIBS = -lm

.SUFFIXES: .o .c

.c.o:
	$(CC) -c $(INCLUDES) $(CFLAGS) $<

animDemo: animDemo.o animEngine.o fileEngine.o utilEngine.o
	-rm -f $@
	$(LD) -o $@ animDemo.o animEngine.o fileEngine.o utilEngine.o $(LIBS)
