import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticCharacter(BaseAnalytic):
    """
    Analyze the DG connectivity.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine the characters in the scene for a few basic structure
        elements. The CSV file headings are generic so as to maximize the
        ability to process the data - 'Character','Type','Value'.
        
        When 'showDetails' is False, the default then the following rows are
        reported:
                - Character Name, 'Member', Number of members in the character
                - Character Name, 'Map', Character to which it is mapped
        
        When 'showDetails' is True then the data looks like this:
                - Character Name, 'Member', Character Member Plug name
                - Character Name, 'Map', Character to which it is mapped
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'Character'
    
    
    __fulldocs__ = "Analyze the DG connectivity.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine the characters in the scene for a few basic structure\n\t      elements. The CSV file headings are generic so as to maximize the\n\t      ability to process the data - 'Character','Type','Value'.\n\t      \n\t      When 'showDetails' is False, the default then the following rows are\n\t      reported:\n\t      \t- Character Name, 'Member', Number of members in the character\n\t      \t- Character Name, 'Map', Character to which it is mapped\n\t      \n\t      When 'showDetails' is True then the data looks like this:\n\t      \t- Character Name, 'Member', Character Member Plug name\n\t      \t- Character Name, 'Map', Character to which it is mapped\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



