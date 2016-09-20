import maya
import re

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticSkinClusters(BaseAnalytic):
    """
    Analyze type and usage of skin cluster deformers to discover usage
    patterns contrary to the assumptions of the code.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine the skin cluster nodes in the scene for connection on the
        driver points attribute. Checks for any connection first, and then for
        the size of the driver versus the size of the driven mesh second. The
        assumption was that the driver would always be much smaller than the
        driven mesh since that's kind of the point of a skin cluster.
        
        The analytics output contains the following columns
                Deformer        : Name of the skin cluster found
                Connection      : Name of the node connected at the driver points
                                          input or '' if none
                DriverSize      : Number of points in the driver points input
                DrivenSize      : Number of points in the driven object
        
        If 'showDetails' is False then the names are anonymized.
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'SkinClusters'
    
    
    __fulldocs__ = "Analyze type and usage of skin cluster deformers to discover usage\npatterns contrary to the assumptions of the code.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine the skin cluster nodes in the scene for connection on the\n\t      driver points attribute. Checks for any connection first, and then for\n\t      the size of the driver versus the size of the driven mesh second. The\n\t      assumption was that the driver would always be much smaller than the\n\t      driven mesh since that's kind of the point of a skin cluster.\n\t      \n\t      The analytics output contains the following columns\n\t      \tDeformer\t: Name of the skin cluster found\n\t      \tConnection\t: Name of the node connected at the driver points\n\t      \t\t\t\t  input or '' if none\n\t      \tDriverSize\t: Number of points in the driver points input\n\t      \tDrivenSize\t: Number of points in the driven object\n\t      \n\t      If 'showDetails' is False then the names are anonymized.\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



vtxPairPattern = None

vtxPattern = None

vtxAllPattern = None


