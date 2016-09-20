==========================================================================
// Copyright 2012 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
==========================================================================

Alembic library and its dependencies
====================================

Alembic
OpenEXR
Boost (headers only)
HDF5


NOTE: The HDF5 library shipped with Maya is not thread-safe (Compiled 
      with --disable-threadsafe). Profiling shows that non-thread-safe
      HDF5 has performance advantage over the thread-safe one. It is 
      safe because Maya currently invokes Alembic in a single threaded
      fashion.

