"""
The analytics in this directory are loaded in the bootstrap method, although you
can add analytics from any location using the @makeAnalytic decorator.

    # Get a list of all available analytics
    import maya.analytics
    listAnalytics()
"""

import sys
import importlib
import pkgutil

from maya.analytics.utilities import *

