==========================================================================
// Copyright 2013 Autodesk, Inc. All rights reserved. 
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
==========================================================================


gpuCache : A memory cache for non-editable geometry
===================================================

Introduction
------------

This is a plug-in for showing how non-editable geometry can be
integrated into Maya. The gpuCache node is a Shape node that holds
baked animated geometry in memory. The node can stream the animated
geometry to the viewport without triggering any DG evaluation of
geometry attributes (mesh, nurbs or subd surface data).


Features
--------

The gpuCache works with any of the viewport renderers (Default,
Hardware and Viewport 2.0). It is not possible to assign a Maya
material to the gpuCache node. When in Viewport 2.0, the gpuCache 
will use the material exported to the Alembic file.
Note that only a subset of Maya's materials are currently supported.
When in Default or Hardware viewport, only the exported diffuse color
is used. Textures are not supported yet.

It is currently possible to select the non-editable geometry. Its
transform can be edited and animated using any of the Maya tools.

Non-editable geometry can be marked as being templated or referenced.

Non-editable geometry can be filtered out. A gpuCache can also be 
marked as non-visible.
 

Building the gpuCache plug-in
-----------------------------

Before building the gpuCache plug-in, one needs to manually
unzip the following file:
    devkit/Alembic/include/AlembicPrivate/boost.zip
to create the hierarchy:
    devkit/Alembic/include/AlembicPrivate/boost/...

The boost.zip contains the headers of the Boost C++ library that are
necessary for succesfully compiling the plug-in.

Afterward, you can use either the Makefile, the MSVC project or the
Xcode project to build this plug-in using the procedure normally used
for building any other devkit plug-in.

NOTE: The HDF5 library shipped with Maya is not thread-safe (Compiled 
      with --disable-threadsafe). Profiling shows that non-thread-safe
      HDF5 has performance advantage over the thread-safe one. It is 
      safe because Maya currently invokes Alembic in a single threaded
      fashion.


Implementation notes
--------------------

o The gpuCache node is currently implemented using the
  MPxSurfaceShape, MPxSurfaceShapeUI, MPxSubSceneOverride interfaces.

o The class GPUCache::Geometry abstracts the baked animated
  geometry. It holds the geometry in a format that can be streamed
  directly to OpenGL or Viewport 2.0 (both OpenGL or DirectX) for
  performance purposes.

o The gpuCache node implements MPxSubSceneOverride to draw into
  Viewport 2.0. MPxSubSceneOverride API supports shadows, MSAA, post
  effects, etc..  

o An earlier implementation using MPxDrawOverride has been left in
  as an example and can be enabled by setting an environment variable.
  Look in the code for the details.

o The gpuCache can write materials to the Alembic files in addition
  to the custom diffuse color property. The Alembic material object
  is introduced in Alembic 1.1.0. The gpuCache writes the network
  materials in "adskMayaGpuCache" target. The network material nodes
  have the same name and properties as Maya shading nodes.
  Currently, only Maya lambert and phong materials are supported.
  The materials are only used to display gpuCache nodes in shaded mode.

o The gpuCache supports loading Alembic files in a worker thread.
  The main thread will not be blocked after it schedules to load an
  Alembic file. Maya GUI is available for use while the Alembic file is
  being loaded in background.
  The background loading consists of 2 stages:
  Loading Hierarchy: This stage loads the hierarchy (transform) information
                     from the Alembic file.
  Loading Shapes: This stage loads shapes one by one from the Alembic file.
  When the hierarchy is loaded, the gpuCache node will show a bounding box
  hierarchy in the viewport. The bounding box will be replaced by the real
  shape later. In-view shapes will be loaded in priority.
