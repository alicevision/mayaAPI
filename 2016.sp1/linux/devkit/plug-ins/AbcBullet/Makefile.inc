#-
# ==========================================================================
# Copyright (c) 2012 Autodesk, Inc.
# All rights reserved.
# 
# These coded instructions, statements, and computer programs contain
# unpublished proprietary information written by Autodesk, Inc., and are
# protected by Federal copyright law. They may not be disclosed to third
# parties or copied or duplicated in any form, in whole or in part, without
# the prior written consent of Autodesk, Inc.
# ==========================================================================
#+

AbcBullet_EXTRA_C++FLAGS = -ftemplate-depth-60 
AbcBullet_EXTRA_INCLUDES = -I$(DEVKIT_ALEMBIC_INCDIR) \
                           -I$(DEVKIT_ALEMBIC_INCDIR)/AlembicPrivate \
                           -I$(DEVKIT_ALEMBIC_INCDIR)/AlembicPrivate/ilmbase
AbcBullet_EXTRA_LFLAGS   = -L$(DEVKIT_ALEMBIC_LIBDIR)

# Overwrite the default maya plugin LFLAGS 
# since We don't want the linux_plugin.map
LFLAGS = $(C++FLAGS) -Wl,-Bsymbolic -shared
#LFLAGS  += -Wl,--version-script=$(TOP)/linux_plugin.map
