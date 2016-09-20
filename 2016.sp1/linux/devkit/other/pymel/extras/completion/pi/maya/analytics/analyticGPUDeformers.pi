import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticGPUDeformers(BaseAnalytic):
    """
    Checks if the geometry is supported by deformer evaluator.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine animated deformers nodes and check how they are used.
        
        When 'showDetails' is False, the default, then the CSV columns are:
        
                DeformerMode            : Description of the usage for the animated deformer node
                Type                            : Deformer type
                SupportedGeometry       : True if the geometry processed by animated deformer nodes is supported by deformer evaluator
                Count                           : Number of animated deformer nodes in this mode
        
                See _isSupportedGeometry() for what criteria a geometry must meet to be supported.
        
        When 'showDetails' is True, then the CSV columns are:
        
                DeformerNode                    : Name of the animated deformer node
                Type                                    : Type for this node
                SupportedGeometry               : True if the geometry processed by animated deformer node is supported by deformer evaluator
        
        One row is output for every animated deformer node.
        
        Return True if the analysis succeeded, else False
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'GPUDeformers'
    
    
    __fulldocs__ = "Checks if the geometry is supported by deformer evaluator.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine animated deformers nodes and check how they are used.\n\t      \n\t      When 'showDetails' is False, the default, then the CSV columns are:\n\t      \n\t      \tDeformerMode\t\t: Description of the usage for the animated deformer node\n\t      \tType\t\t\t\t: Deformer type\n\t      \tSupportedGeometry\t: True if the geometry processed by animated deformer nodes is supported by deformer evaluator\n\t      \tCount\t\t\t\t: Number of animated deformer nodes in this mode\n\t      \n\t      \tSee _isSupportedGeometry() for what criteria a geometry must meet to be supported.\n\t      \n\t      When 'showDetails' is True, then the CSV columns are:\n\t      \n\t      \tDeformerNode\t\t\t: Name of the animated deformer node\n\t      \tType\t\t\t\t\t: Type for this node\n\t      \tSupportedGeometry\t\t: True if the geometry processed by animated deformer node is supported by deformer evaluator\n\t      \n\t      One row is output for every animated deformer node.\n\t      \n\t      Return True if the analysis succeeded, else False\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



