import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticIK(BaseAnalytic):
    """
    Analyze structure and usage of standard IK system.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Scan all of the standard IK connections to pull out usage statistics.
        "standard" means "not HIK". See analyticHIK() for specific details on
        that IK subsystem.
        
        The CSV output provides columns for the name of the statistic
        collected and the count of occurences of that statistic with the
        headings 'Handle', 'Parameter', 'Value'. When 'showDetails' is off any
        node names in the output are replaced by their generic form
        'NODETYPEXXX' where 'NODETYPE' is the type of node and 'XXX' is a
        unique ID per node type. The following are collected:
                - IK Handle Name, 'Solver', Name of the solver the handle uses
                - IK Handle Name, 'Chain Start', Starting node of chain
                - IK Handle Name, 'Chain End', Ending node of chain
                - IK Handle Name, 'Chain Length', Number of nodes in the chain
                - IK Handle Name, 'End Effector', Name of chain's end effector
                - "", 'Solver', Name/Type of solver with no associated handles
                - "", 'Chain Link', Number of Joint nodes with no associated Handle
                        (Not reported if 'showDetails' is True.)
        
        if 'showDetails' is turned extra lines of this form are also output:
                - IK Handle Name, 'Chain Link', Node in the middle of a chain
                - "", 'Chain Link', Joint node with no associated Handle
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'IK'
    
    
    __fulldocs__ = 'Analyze structure and usage of standard IK system.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled \'MayaAnalytics\' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn\'t openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Scan all of the standard IK connections to pull out usage statistics.\n\t      "standard" means "not HIK". See analyticHIK() for specific details on\n\t      that IK subsystem.\n\t      \n\t      The CSV output provides columns for the name of the statistic\n\t      collected and the count of occurences of that statistic with the\n\t      headings \'Handle\', \'Parameter\', \'Value\'. When \'showDetails\' is off any\n\t      node names in the output are replaced by their generic form\n\t      \'NODETYPEXXX\' where \'NODETYPE\' is the type of node and \'XXX\' is a\n\t      unique ID per node type. The following are collected:\n\t      \t- IK Handle Name, \'Solver\', Name of the solver the handle uses\n\t      \t- IK Handle Name, \'Chain Start\', Starting node of chain\n\t      \t- IK Handle Name, \'Chain End\', Ending node of chain\n\t      \t- IK Handle Name, \'Chain Length\', Number of nodes in the chain\n\t      \t- IK Handle Name, \'End Effector\', Name of chain\'s end effector\n\t      \t- "", \'Solver\', Name/Type of solver with no associated handles\n\t      \t- "", \'Chain Link\', Number of Joint nodes with no associated Handle\n\t      \t\t(Not reported if \'showDetails\' is True.)\n\t      \n\t      if \'showDetails\' is turned extra lines of this form are also output:\n\t      \t- IK Handle Name, \'Chain Link\', Node in the middle of a chain\n\t      \t- "", \'Chain Link\', Joint node with no associated Handle\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names \'stdout\' and \'stderr\' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n'
    
    
    isAnalytic = True



