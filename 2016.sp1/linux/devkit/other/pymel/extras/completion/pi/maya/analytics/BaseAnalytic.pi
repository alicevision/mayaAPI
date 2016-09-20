import tempfile
import sys
import os

from maya.analytics.ObjectNamer import ObjectNamer

class BaseAnalytic(object):
    """
    Base class for output for analytics.
    
    The default location for the anlaytic output is in a subdirectory
    called 'MayaAnalytics' in your temp directory. You can change that
    at any time by calling setOutputDirectory().
    
    Class static member:
             ANALYTIC_NAME  : Name of the analytic
    
    Class members:
             director               : Directory the output will go to
             fileName               : Name of the file for this analytic
             fileState              : Is the output stream currently opened?
                                              If FILE_DEFAULT then it isn't openable
             outputFile             : Output File object for writing the results
    """
    
    
    
    def __init__(self):
        pass
    
    
    def outputExists(self):
        """
        Checks to see if this analytic already has existing output at
        the current directory location. If stdout/stderr this is always
        false, otherwise it checks for the existence of a non-zero sized
        output file. (All legal output will at least have a header line,
        and we may have touched the file to create it in the call to
        setOutputDirectory().
        """
    
        pass
    
    
    def setOutputDirectory(self, directory):
        """
        Call this method to set a specific directory as the output location.
        The special names 'stdout' and 'stderr' are recognized as the
        output and error streams respectively rather than a directory.
        """
    
        pass
    
    
    __dict__ = None
    
    __weakref__ = None
    
    ANALYTIC_NAME = 'Unknown'
    
    
    FILE_CLOSED = 2
    
    
    FILE_DEFAULT = 0
    
    
    FILE_OPENED = 1



