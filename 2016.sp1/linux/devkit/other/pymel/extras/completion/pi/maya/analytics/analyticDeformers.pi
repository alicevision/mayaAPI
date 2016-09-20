import maya
import re

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticDeformers(BaseAnalytic):
    """
    Analyze type and usage of single deformers and deformer chains.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine the meshes in the scene for deformation. There will be two
        types of data in the output file under the column headings
        'Deformer','Member','Value':
                - Deformer Name, Member Object, Membership Information, Member Count
                        One line per object being affected by the deformer
                - Deformer Name, '', Name of next deformer in the chain, Deformer Chain length
                        Only if more than one deformer is being applied to the same object
        
        If 'showDetails' is False then the Member Information is omitted,
        otherwise it will be a selection-list format of all members on that
        object subject to deformation by the given deformer.
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'Deformers'
    
    
    __fulldocs__ = "Analyze type and usage of single deformers and deformer chains.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine the meshes in the scene for deformation. There will be two\n\t      types of data in the output file under the column headings\n\t      'Deformer','Member','Value':\n\t      \t- Deformer Name, Member Object, Membership Information, Member Count\n\t      \t\tOne line per object being affected by the deformer\n\t      \t- Deformer Name, '', Name of next deformer in the chain, Deformer Chain length\n\t      \t\tOnly if more than one deformer is being applied to the same object\n\t      \n\t      If 'showDetails' is False then the Member Information is omitted,\n\t      otherwise it will be a selection-list format of all members on that\n\t      object subject to deformation by the given deformer.\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



vtxPairPattern = None

vtxPattern = None

vtxAllPattern = None


