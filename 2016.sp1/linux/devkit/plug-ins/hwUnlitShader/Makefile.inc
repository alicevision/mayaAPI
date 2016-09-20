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

#=================
# NVParse Plugins 
#================

#  The following plugins require the 'nvparse' library from nVIDIA.
#  (http://developer.nvidia.com/object/nvparse.html)
#
# 		hwAnisotropicShader_NV20     
#       hwDecalBumpShader_NV20
#       hwReflectBumpShader_NV20
#       hwRefractReflectShader_NV20
#       hwToonShader_NV20
#       hwUnlitShader
#       pnTriangles
#
#  Set the NVPARSE variable to the directory where nvparse is
#  installed.

ifeq ($(NVPARSE),)
    NVPARSE     = /opt/nvidia/nvparse
endif

ifeq ($(wildcard $(NVPARSE)/include/glh),)
    $(error Cannot find the 'nvparse' library from nVidia. Please	\
            set the NVPARSE environment variable to point to the	\
            directory where the 'nvparse' library is installed.)
endif

NVPARSE_INC = -I$(NVPARSE)/include/glh

hwAnisotropicShader_NV20_EXTRA_INCLUDES    = $(NVPARSE_INC)
hwDecalBumpShader_NV20_EXTRA_INCLUDES      = $(NVPARSE_INC)
hwReflectBumpShader_NV20_EXTRA_INCLUDES    = $(NVPARSE_INC)
hwRefractReflectShader_NV20_EXTRA_INCLUDES = $(NVPARSE_INC)
hwToonShader_NV20_EXTRA_INCLUDES           = $(NVPARSE_INC)
hwUnlitShader_EXTRA_INCLUDES               = $(NVPARSE_INC)
pnTriangles_EXTRA_INCLUDES                 = $(NVPARSE_INC)
