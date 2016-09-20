#-
#  ==========================================================================
#  Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
# 
#  Use of this software is subject to the terms of the Autodesk
#  license agreement provided at the time of installation or download,
#  or which otherwise accompanies this software in either electronic
#  or hard copy form.
#  ==========================================================================
#+

C++			= CC

# Uncomment the following line on MIPSpro 7.2 compilers to supress
# warnings about declarations in the X header files
# WARNFLAGS	= -woff 3322

# When the following line is present, the linking of plugins will
# occur much faster.  If you link transitively (the default) then
# a number of "ld Warning 85" messages will be printed by ld.
# They are expected, and can be safely ignored.
NO_TRANS_LINK = -no_transitive_link

CFLAGS		= -g -mips3 -n32
C++FLAGS	= $(CFLAGS) $(WARNFLAGS)
INCLUDES	= -I. -I$(MAYA_LOCATION)/include
LD			= $(C++) -shared $(NO_TRANS_LINK) $(C++FLAGS)
LIBS		= -L$(MAYA_LOCATION)/lib -lOpenMaya

.SUFFIXES: .cpp .cc .o .so .c

.cc.o:
	$(C++) -c $(INCLUDES) $(C++FLAGS) $<

.cpp.o:
	$(C++) -c $(INCLUDES) $(C++FLAGS) $<

.cc.i:
	$(C++) -E $(INCLUDES) $(C++FLAGS) $*.cc > $*.i

.cc.so:
	-rm -f $@
	$(LD) -o $@ $(INCLUDES) $< $(LIBS)

.cpp.so:
	-rm -f $@
	$(LD) -o $@ $(INCLUDES) $< $(LIBS)

.o.so:
	-rm -f $@
	$(LD) -o $@ $< $(LIBS)

IK2BSOLVEROBJS = AwMatrix.o AwPoint.o AwVector.o AwQuaternion.o ik2Bsolver.o
ik2Bsolver.so: $(IK2BSOLVEROBJS)
	-rm -f $@
	$(LD) -o $@ $(IK2BSOLVEROBJS) $(LIBS) -lOpenMayaAnim

depend:
	makedepend $(INCLUDES) -I/usr/include/CC *.cc

clean:
	-rm -f *.o

Clean:
	-rm -f *.o *.so *.bak
