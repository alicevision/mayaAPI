from . import BaseAnalytic

from maya.analytics.analyticGPUDeformers import *

class analyticGPUClusters(maya.analytics.BaseAnalytic.BaseAnalytic):
    """
    Analyze the usage mode of cluster node.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine animated cluster nodes and check how they are used.  It checks
        whether they are used for fixed rigid transform, weighted rigid transform
        or per-vertex-weighted transform.
        
        When 'showDetails' is False, the default, then the CSV columns are:
        
                ClusterMode                     : Description of the usage for the animated cluster node
                Mode                            : Mode for animated cluster nodes meeting this criteria
                SupportedGeometry       : True if the geometry processed by animated cluster nodes meeting this criteria is supported by deformer evaluator
                Count                           : Number of animated cluster nodes in this mode
        
                See _isSupportedGeometry() for what criteria a geometry must meet to be supported.
        
        When 'showDetails' is True, then the CSV columns are:
        
                ClusterNode                             : Name of the animated cluster node
                EnvelopeIsStaticOne             : True if the envelope is not animated and its value is 1
                UsesWeights                             : True if weights are used in the node
                UsesSameWeight                  : True if weight is the same for all vertices
                Mode                                    : Mode for this node
                SupportedGeometry               : True if the geometry processed by animated cluster node is supported by deformer evaluator
        
        One row is output for every animated cluster node.
        
        The "Mode" is an integer value with the following meaning:
        - 1 => Rigid transform                  : cluster node only performs a rigid transform
        - 2 => Weighted rigid transform : cluster node performs a rigid transform, but it is weighted down by a factor
        - 3 => Per-vertex transform             : cluster node computes a different transform for each individually-weighted vertex
        
        Return True if the analysis succeeded, else False
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'GPUClusters'
    
    
    __fulldocs__ = 'Analyze the usage mode of cluster node.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled \'MayaAnalytics\' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn\'t openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine animated cluster nodes and check how they are used.  It checks\n\t      whether they are used for fixed rigid transform, weighted rigid transform\n\t      or per-vertex-weighted transform.\n\t      \n\t      When \'showDetails\' is False, the default, then the CSV columns are:\n\t      \n\t      \tClusterMode\t\t\t: Description of the usage for the animated cluster node\n\t      \tMode\t\t\t\t: Mode for animated cluster nodes meeting this criteria\n\t      \tSupportedGeometry\t: True if the geometry processed by animated cluster nodes meeting this criteria is supported by deformer evaluator\n\t      \tCount\t\t\t\t: Number of animated cluster nodes in this mode\n\t      \n\t      \tSee _isSupportedGeometry() for what criteria a geometry must meet to be supported.\n\t      \n\t      When \'showDetails\' is True, then the CSV columns are:\n\t      \n\t      \tClusterNode\t\t\t\t: Name of the animated cluster node\n\t      \tEnvelopeIsStaticOne\t\t: True if the envelope is not animated and its value is 1\n\t      \tUsesWeights\t\t\t\t: True if weights are used in the node\n\t      \tUsesSameWeight\t\t\t: True if weight is the same for all vertices\n\t      \tMode\t\t\t\t\t: Mode for this node\n\t      \tSupportedGeometry\t\t: True if the geometry processed by animated cluster node is supported by deformer evaluator\n\t      \n\t      One row is output for every animated cluster node.\n\t      \n\t      The "Mode" is an integer value with the following meaning:\n\t      - 1 => Rigid transform\t\t\t: cluster node only performs a rigid transform\n\t      - 2 => Weighted rigid transform\t: cluster node performs a rigid transform, but it is weighted down by a factor\n\t      - 3 => Per-vertex transform\t\t: cluster node computes a different transform for each individually-weighted vertex\n\t      \n\t      Return True if the analysis succeeded, else False\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names \'stdout\' and \'stderr\' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n'
    
    
    isAnalytic = True



