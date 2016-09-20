import operator
import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticNodeTypes(BaseAnalytic):
    """
    This class provides scene stats collection on node types.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Generates the number of nodes of each type in a scene in the
        CSV form "NodeType","Count", ordered from most frequent to least
        frequent.
        
        If 'showDetails' is set to True then insert two extra columns,
        "Depth" containing the number of parents the given node type has,
        and "Hierarchy" containing a "|"-separated string with all of the
        node types above that one in the hierarchy, starting with it and
        working upwards. It will also include lines for all of the node
        types that have no corresponding nodes in the scene, signified by
        a "Count" of 0.
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'NodeTypes'
    
    
    __fulldocs__ = 'This class provides scene stats collection on node types.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled \'MayaAnalytics\' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn\'t openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Generates the number of nodes of each type in a scene in the\n\t      CSV form "NodeType","Count", ordered from most frequent to least\n\t      frequent.\n\t      \n\t      If \'showDetails\' is set to True then insert two extra columns,\n\t      "Depth" containing the number of parents the given node type has,\n\t      and "Hierarchy" containing a "|"-separated string with all of the\n\t      node types above that one in the hierarchy, starting with it and\n\t      working upwards. It will also include lines for all of the node\n\t      types that have no corresponding nodes in the scene, signified by\n\t      a "Count" of 0.\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names \'stdout\' and \'stderr\' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n'
    
    
    isAnalytic = True



