from . import BaseAnalytic

from maya.analytics.analyticGPUDeformers import *

class analyticGPUTweaks(maya.analytics.BaseAnalytic.BaseAnalytic):
    """
    Analyze the usage mode of tweak node.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine animated tweak nodes and check how they are used.  It checks
        whether they use the relative or absolute mode and whether individual
        tweaks themselves are actually used.
        
        When 'showDetails' is False, the default, then the CSV columns are:
        
                TweakType               : Description of the usage for the animated tweak node
                Relative                : Value of the relativeTweak attribute of the animated tweak nodes meeting this criteria
                UsesTweaks              : True if tweaks are used in the nodes meeting this criteria
                UsesMesh                : True if some of the geometry processed by animated tweak nodes meeting this criteria is a mesh
                Count                   : Number of animated tweak nodes meeting this criteria
        
        When 'showDetails' is True, then the CSV columns are:
        
                TweakNode               : Name of the animated tweak node
                Relative                : Value of the relativeTweak attribute of the animated tweak node
                UsesTweaks              : True if tweaks are used in the node
                UsesMesh                : True if some of the geometry processed by animated tweak node is a mesh
        
        One row is output for every animated tweak node.
        
        Return True if the analysis succeeded, else False
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'GPUTweaks'
    
    
    __fulldocs__ = "Analyze the usage mode of tweak node.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine animated tweak nodes and check how they are used.  It checks\n\t      whether they use the relative or absolute mode and whether individual\n\t      tweaks themselves are actually used.\n\t      \n\t      When 'showDetails' is False, the default, then the CSV columns are:\n\t      \n\t      \tTweakType\t\t: Description of the usage for the animated tweak node\n\t      \tRelative\t\t: Value of the relativeTweak attribute of the animated tweak nodes meeting this criteria\n\t      \tUsesTweaks\t\t: True if tweaks are used in the nodes meeting this criteria\n\t      \tUsesMesh\t\t: True if some of the geometry processed by animated tweak nodes meeting this criteria is a mesh\n\t      \tCount\t\t\t: Number of animated tweak nodes meeting this criteria\n\t      \n\t      When 'showDetails' is True, then the CSV columns are:\n\t      \n\t      \tTweakNode\t\t: Name of the animated tweak node\n\t      \tRelative\t\t: Value of the relativeTweak attribute of the animated tweak node\n\t      \tUsesTweaks\t\t: True if tweaks are used in the node\n\t      \tUsesMesh\t\t: True if some of the geometry processed by animated tweak node is a mesh\n\t      \n\t      One row is output for every animated tweak node.\n\t      \n\t      Return True if the analysis succeeded, else False\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



