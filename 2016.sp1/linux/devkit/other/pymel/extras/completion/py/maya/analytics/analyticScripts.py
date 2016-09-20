import re
import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticScripts(BaseAnalytic):
    """
    Analyze usage of the 'scriptJob' callback.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Generates the number of scriptJobs active in the scene, grouped
        by the type of event that they are watching. No details of the
        actual event are collected. Output is in CSV form with the
        columns 'eventType,count', ordered from most frequent to least
        frequent.
        
        Set the option 'showDetails' flag to True to include the name
        of the script called and detail parameters for certain other
        triggers (e.g. the name of the node whose name change is being
        monitored).
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'Scripts'
    
    
    __fulldocs__ = "Analyze usage of the 'scriptJob' callback.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Generates the number of scriptJobs active in the scene, grouped\n\t      by the type of event that they are watching. No details of the\n\t      actual event are collected. Output is in CSV form with the\n\t      columns 'eventType,count', ordered from most frequent to least\n\t      frequent.\n\t      \n\t      Set the option 'showDetails' flag to True to include the name\n\t      of the script called and detail parameters for certain other\n\t      triggers (e.g. the name of the node whose name change is being\n\t      monitored).\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



jobPattern = None


