"""
Utility to read and analyze dependency graph dirty state information.
Allows you to produce a comparision of two sets of state information.

        from dirtyState import *

        # Get the current scene's dirty data state information
        stateBefore = dirtyState( checkData=True )

        # Perform some operation that may change the dirty state
        doMyOperation()

        # Get the new dirty data state information
        stateAfter = dirtyState( checkData=True )

        # Compare them to see if they are the same
        stateBefore.compare(stateAfter)
"""

import sys
import maya.cmds as cmds
import re

class dirtyState:
    """
    Provides access and manipulation of dirty state data that has been
    produced by various invocations of the 'dgdirty' command.
    """
    
    
    
    def __init__(self, stateFileName=None, longNames=False, checkPlugs=True, checkData=True, checkConnections=True):
        """
        Create a dirty state object from a file or the current scene.
        
        The dirty data is read in and stored internally in a format that makes
        formatting and comparison easy.
        
                stateFileName   : If None then the current scene will be used,
                                                  otherwise the file will be read.
                longNames               : If True then don't attempt to shorten the node
                                                  names by removing namespaces and DAG path elements.
                checkPlugs              : If True then check for plugs that are dirty
                checkData               : If True then check for plug data that is dirty
                checkConnections: If True then check for connections that are dirty
        """
    
        pass
    
    
    def compare(self, other):
        """
        Compare this dirty state against another one and generate a
        summary of how the two sets differ. Differences will be returned
        as a string list consisting of difference descriptions. That way
        when testing, an empty return means the graphs are the same.
        
        The difference type formats are:
        
                plug dirty N                    Plug was dirty in other but not in self
                plug clean N                    Plug was dirty in self but not in other
                data dirty N                    Data was dirty in other but not in self
                data clean N                    Data was dirty in self but not in other
                connection dirty S D    Connection was dirty in other but not in self
                connection clean S D    Connection was dirty in self but not in other
        """
    
        pass
    
    
    def compareOneType(self, other, requestType, madeDirty):
        """
        Compare this dirty state against another one and return the values
        that differ in the way proscribed by the parameters:
        
                requestType     : Type of dirty state to check [plug/data/connection]
                madeDirty       : If true return things that became dirty, otherwise
                                          return things that became clean
        
        Nothing is returned for items that did not change.
        """
    
        pass
    
    
    def name(self):
        """
        Return an identifying name for this collection of dirty states
        """
    
        pass
    
    
    def write(self, fileName=None):
        """
        Dump the states in the .dirty format it uses for reading. Useful for
        creating a dump file from the current scene, or just viewing the
        dirty state generated from the current scene. If the fileName is
        specified then the output is sent to that file, otherwise it goes
        to stdout.
        """
    
        pass
    
    
    cleanType = 'clean'
    
    
    connectionType = 'connection'
    
    
    dataType = 'data'
    
    
    dirtyType = 'dirty'
    
    
    plugType = 'plug'



def checkMaya():
    pass



_mayaIsAvailable = True


