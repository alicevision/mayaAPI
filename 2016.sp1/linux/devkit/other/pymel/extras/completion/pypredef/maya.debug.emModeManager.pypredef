"""
Helper class that maintains the EM mode information. Given a string
to specifies an EM mode combination (type +/- evaluators) it will
handle the details regarding translating the mode description into
actions and turning the mode on and off.

String syntax is an abbreviated evaluation mode followed by zero or
more evaluator directives. Regex is MODE{[+-]EVALUTOR}*. Examples:
        dg                       : Turn the EM off and go back to DG mode
        ems                      : Turn the EM on and put it into serial mode
        emp                      : Turn the EM on and put it into parallel mode
        emp+null         : Turn the EM on and enable the 'null' evaluator
        emp-dynamics : Turn the EM on and disable the 'dynamics' evaluator
        emp-dynamics+deformer
                                 : Turn the EM on, disable the 'dynamics' evaluator, and
                                   enable the 'deformer' evaluator

Calling the setMode() method will put the EM into the named mode.
Calling it again will exit that mode and put it into the new mode,
including unloading any plugins that had to be loaded. Destruction
or reassignment of the manager will restore the EM to the state it
had just before the first time the mode was set.

The plugin loading is not magic, it's a hardcoded list in this file.
Update it if you want to handle any new plugins.

The object is set up to use the Python "with" syntax as follows:

        with emModeManager() as mgr:
                mgr.setMode( someMode )

That will ensure the original states are all restored. There's no other
reliable way to do it in Python. If you need different syntax you can
manually call the method to complete the sequence:

        mgr = emModeManager()
        mgr.setMode( someMode )
        mgr.restore()
"""

import maya.cmds as cmds
import os
import re

class emModeManager(object):
    def __enter__(self):
        """
        #----------------------------------------------------------------------
        """
    
        pass
    
    
    def __exit__(self, type, value, traceback):
        """
        Ensure the state is restored if this object goes out of scope
        """
    
        pass
    
    
    def __init__(self):
        """
        Defining both __enter__ and __init__ so that either one can be used
        """
    
        pass
    
    
    def restore(self):
        """
        Restore the evaluation manager to its original mode prior to enabling
        this one.  Not necessary to call this when using the "with emModeManager()"
        syntax. Only needed when you explicitly instantiate the mode manager.
        Then you have to call this if you want your original state restored.
        """
    
        pass
    
    
    def setMode(self, modeName):
        """
        Put the EM into a named mode. See class docs for details on mode names.
        
        raises SyntaxError if the mode name is not legal
        """
    
        pass
    
    
    __dict__ = None
    
    __weakref__ = None



def _dbg(message):
    """
    #======================================================================
    """

    pass


def _hasEvaluationManager():
    """
    Check to see if the evaluation manager is available
    """

    pass



inDebugMode = False

reEvaluators = None

evaluatorPlugins = {}

reMode = None


