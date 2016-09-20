"""
ile ToDo.py
A utility to help identify areas of the code that need work.

This is just a syntactically similar interface of the TODO macro
in C++ code that uses the toDo command to register hits of
incomplete code and mark areas of Python source for extraction
to the code health summary.

This should only be called from tests as we obviously don't want
to pollute customer-visible files with this sort of thing.

Example usage:

        from maya.debug.TODO import *
        def myIncompleteFunction(self, case):
                if case == 0:
                        handleCase0()
                elif case == 1:
                        handleCase1()
                else:
                        TODO( 'Finish', 'Unhandled case value %s' % case, 'MAYA-99999' )
                        # Currently only cases 0 and 1 are handled but there are
                        # situations in which values of 2 or 3 can be passed in. Those
                        # represent edge cases at the moment and they will be handled
                        # once our customer feedback lets us know exactly what we
                        # should be doing in those situations.

\sa ToDo.h
\sa TODO.mel
"""

import maya.cmds as cmds
import traceback

def TODO(toDoType, description, jiraEntry):
    """
    Register a "TODO" with the system. This is used to track when running code
    hits an area that has work to be done. This can help track down bugs,
    inefficiencies, and just generally make incomplete work more visible.
    
            toDoType
                    What type of ToDo is it. Entries are grouped by this
                    value. There are some hardcoded values to choose from, or you can use
                    your own.
                            Refactor        : Code works but needs some sort of refactoring.
                            Hack            : Ugly shortcuts were taken to get something working quickly.
                            Finish          : Code doesn't handle all cases yet.
                            Bug                     : There is a known problem with the code.
                            Performance     : The code could be made faster or more scalable.
    
            description
                    Short description of what work needs to be done to remedy the problem.
    
            jiraEntry
                    Link to the name of a JIRA entry referencing this code.
    """

    pass



