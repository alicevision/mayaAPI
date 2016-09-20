import maya
import math

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticValues(BaseAnalytic):
    """
    Analyze use of plug values that make some simple algorithms complex.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Here is a complete list of what will be counted and reported:
                - transforms using each of the non-standard rotation orders
                - transforms using scale limits, min and/or max
                - transforms using rotate limits, min and/or max
                - transforms using translation limits, min and/or max
                - joints with incoming connections on their scale attribute(s)
                - joints with incoming connections on their shear attribute(s)
                - joints with incoming connections on their translate attribute(s)
                - joints with non-uniform scale values (and no incoming connection)
                - joints with non-default shear values (and no incoming connection)
        
        If you enable the showDetails flag then instead of showing one line
        per type of match with the number of matches found there will be a line
        for every match showing the node name matched.
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'Values'
    
    
    __fulldocs__ = "Analyze use of plug values that make some simple algorithms complex.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Here is a complete list of what will be counted and reported:\n\t      \t- transforms using each of the non-standard rotation orders\n\t      \t- transforms using scale limits, min and/or max\n\t      \t- transforms using rotate limits, min and/or max\n\t      \t- transforms using translation limits, min and/or max\n\t      \t- joints with incoming connections on their scale attribute(s)\n\t      \t- joints with incoming connections on their shear attribute(s)\n\t      \t- joints with incoming connections on their translate attribute(s)\n\t      \t- joints with non-uniform scale values (and no incoming connection)\n\t      \t- joints with non-default shear values (and no incoming connection)\n\t      \n\t      If you enable the showDetails flag then instead of showing one line\n\t      per type of match with the number of matches found there will be a line\n\t      for every match showing the node name matched.\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



