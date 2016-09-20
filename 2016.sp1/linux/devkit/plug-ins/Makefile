#-
# ==========================================================================
# Copyright 2010 Autodesk, Inc. All rights reserved.
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
ifndef INCL_BUILDRULES
TOP := .
include $(TOP)/buildrules
endif

#
# Also build all plug-ins that are organized in sub-direcories.
#
include Makefile.subdirs


###################
# Top-level rules #
###################

.PHONY: qtPlugins depend_top clean_top Clean_top


qtPlugins:
	$(MAKE) -f Makefile.qt

depend_top :
	makedepend $(INCLUDES) $(MDFLAGS) -f$(TOP)/Makefile $(TOP)/*.cpp

clean_top:
	-rm -f $(TOP)/*.o
	$(MAKE) -f Makefile.qt clean

Clean_top:
	-rm -f $(TOP)/Makefile.bak $(TOP)/*.o $(TOP)/*.so $(TOP)/*.lib $(TOP)/*.bundle
	$(MAKE) -f Makefile.qt Clean


plugins:	qtPlugins				
depend:		depend_top
clean:		clean_top
Clean:		Clean_top


##################
# Specific Rules #
##################

#===============
# Pre-built 
#===============

# Pre-built binaries of some plugins are shipped with Maya. We don't
# want to build them by default with all the others because then there
# would be two versions of the same plugin and unless you know exactly
# what you are doing the wrong one might get loaded.
#
# The top-level makefile of the devkit won't recurse by default into
# the sub-directory of prebuilt plugins. So if you want to build one
# of these you must do so explicitly by invoking make from the given
# sub-directory or invoke the prebuiltPlugins rules from the top-level
# makefile. For example:
#
#  make prebuiltPlugins
#

#======================================
# Plugins with third-party dependencies 
#======================================


#  Some plugins require third-party packages to build so we do not
#  build them by default with all of the others.
#
# The top-level makefile of the devkit won't recurse by default into
# the sub-directory of plugins depending on third-party components. So
# if you want to build one of these you must do so explicitly by
# invoking make from the given sub-directory or invoke the
# thirdPartyPlugins rules from the top-level makefile. For example:
#
#  make thirdPartyPlugins
#

