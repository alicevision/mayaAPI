#-
#*****************************************************************************
# Copyright 2013 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk license
# agreement provided at the time of installation or download, or which
# otherwise accompanies this software in either electronic or hard copy
# form.
#*****************************************************************************
#+

gpuCache_EXTRA_C++FLAGS = -ftemplate-depth-60 
gpuCache_EXTRA_INCLUDES = -I$(DEVKIT_ALEMBIC_INCDIR) \
                          -I$(DEVKIT_ALEMBIC_INCDIR)/AlembicPrivate \
                          -I$(DEVKIT_ALEMBIC_INCDIR)/AlembicPrivate/ilmbase
gpuCache_EXTRA_LFLAGS   = -L$(DEVKIT_ALEMBIC_LIBDIR)
