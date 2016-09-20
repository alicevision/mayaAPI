import os
import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticEvalManager(BaseAnalytic):
    """
    Analyze the DG connectivity.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Run the playback in the scene in DG mode, EM serial mode, and EM
        parallel mode and check to see that the values for all nodes at the
        end of the playback are the same in all three cases.
        
        Values dumped to the analytic CSV file will be anything that is
        different between the three runs in the columns 'Name', 'DG',
        'EMS', 'EMP'.
        
        When 'showDetails' is True then one row per plug inspected is reported
        where:
                Name = Name of the node being reported
                Diff = 0 if all three values are identical,
                           1 if EMS and DG differ but DG and EMP are the same
                           2 if EMP and DG differ but DG and EMS are the same
                           3 if EMS and DG differ and DG and EMP differ
                EMS  = Plug value after EM Serial evaluation
                EMP  = Plug value after EM Parallel evaluation
                DG   = Plug value after DG evaluation
        
        At the end of the rows a summary row is inserted with the totals of
        all nodes in the file, even when 'showDetails' is False:
                Name = ''
                Diff = Total number of plugs different from DG values in both EM modes
                EMS  = Total number of plugs different from DG values in EM serial mode
                EMP  = Total number of plugs different from DG values in EM parallel mode
                DG   = Total number of plugs evaluated in the DG
        
        Return True if the evaluations succeeded and all EMS and EMP evaluations were
        the same as the DG evaluations.
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'EvalManager'
    
    
    __fulldocs__ = "Analyze the DG connectivity.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Run the playback in the scene in DG mode, EM serial mode, and EM\n\t      parallel mode and check to see that the values for all nodes at the\n\t      end of the playback are the same in all three cases.\n\t      \n\t      Values dumped to the analytic CSV file will be anything that is\n\t      different between the three runs in the columns 'Name', 'DG',\n\t      'EMS', 'EMP'.\n\t      \n\t      When 'showDetails' is True then one row per plug inspected is reported\n\t      where:\n\t      \tName = Name of the node being reported\n\t      \tDiff = 0 if all three values are identical,\n\t      \t\t   1 if EMS and DG differ but DG and EMP are the same\n\t      \t\t   2 if EMP and DG differ but DG and EMS are the same\n\t      \t\t   3 if EMS and DG differ and DG and EMP differ\n\t      \tEMS  = Plug value after EM Serial evaluation\n\t      \tEMP  = Plug value after EM Parallel evaluation\n\t      \tDG   = Plug value after DG evaluation\n\t      \n\t      At the end of the rows a summary row is inserted with the totals of\n\t      all nodes in the file, even when 'showDetails' is False:\n\t      \tName = ''\n\t      \tDiff = Total number of plugs different from DG values in both EM modes\n\t      \tEMS  = Total number of plugs different from DG values in EM serial mode\n\t      \tEMP  = Total number of plugs different from DG values in EM parallel mode\n\t      \tDG   = Total number of plugs evaluated in the DG\n\t      \n\t      Return True if the evaluations succeeded and all EMS and EMP evaluations were\n\t      the same as the DG evaluations.\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True


class DGState:
    def __init__(self, resultsFile=None, imageFile=None, eval=False):
        """
        Create a new state object, potentially saving results offline if
        requested.
        
        resultsFile : Name of file in which to save the results.
                                  Do not save anything if None.
        imageFile   : Name of file in which to save the current viewport screenshot.
                                  Do not save anything if None.
        eval            : True means force evaluation of the plugs before checking
                                  state. Used in DG mode since not all outputs used for
                                  (e.g.) drawing will be in the datablock after evaluation.
        """
    
        pass
    
    
    def allPlugs(self):
        """
        Return a list of all plugs in the results.
        """
    
        pass
    
    
    def compare(self, other):
        """
        Compare two state information collections to see if they are the same
        or not. Return False if they differ in any way. This is a quick check;
        other methods should be used for more details
        """
    
        pass
    
    
    def extractDetails(self):
        """
        Pull the details of the results string out into lists for easier
        processing.
        """
    
        pass



EM_IDENTICAL = 0

EM_BOTH_DIFFER = 3

EM_EMS_DIFFERS = 1

EM_EMP_DIFFERS = 2

stateNames = []

ANALYTIC_EM_MAX_FRAMECOUNT = 200


