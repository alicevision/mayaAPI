import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from operator import itemgetter
from maya.analytics.decorators import makeAnalytic

class analyticAnimation(BaseAnalytic):
    """
    Analyze the volume and distribution of animation data.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Examine the animation in the system and gather some basic statistics
        about it. There are two types of animation to find:
        
                1) Anim curves, which animate in the usual manner
                   Care is taken to make sure either time is either an explicit or
                   implicit input since anim curves could be used for reasons
                   other than animation (e.g. setDrivenKey)
                2) Any other node which has the time node as input
                   Since these are pretty generic we can only take note of how
                   many of these there are, and how many output connections they
                   have.
        
        When 'showDetails' is True then the CSV columns are:
        
                AnimationNode           : Name of node providing animation (e.g. animCurve)
                AnimationNodeType       : Type of node providing animation (e.g. animCurveTL)
                DrivenNode                      : Name of node it animates (e.g. transform1)
                DrivenNodeType          : Type of node being animated (e.g. transform)
                Keyframes                       : Number of keyframes in the AnimationNode
                                                          Count is 0 if the AnimationNode is not an anim curve.
                ReallyAnimated          : 1 if there is an anim curve and it is not flat, else 0
        
        One row is output for every AnimationNode -> DrivenNode connection.
        This can be an M -> N correspondence so this can be spread out over
        multiple rows.
        
        The above rows are skipped when 'showDetails' is False and then in
        either case some summary rows are printed, one per animation node type:
        
                AnimationNode           : # of nodes of type 'AnimationNodeType'
                AnimationNodeType       : Type of node providing animation (e.g. animCurveTL)
                DrivenNode                      : ''
                DrivenNodeType          : ''
                Keyframes                       : Sum of number of keyframes in all nodes of
                                                          that type. Keyframe count is 0 for non-param
                                                          curve nodes.
                ReallyAnimated          : Number of non-flat curves on nodes of the given type
        
        Return True if the analysis succeeded, else False
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'Animation'
    
    
    __fulldocs__ = "Analyze the volume and distribution of animation data.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled 'MayaAnalytics' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn't openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Examine the animation in the system and gather some basic statistics\n\t      about it. There are two types of animation to find:\n\t      \n\t      \t1) Anim curves, which animate in the usual manner\n\t      \t   Care is taken to make sure either time is either an explicit or\n\t      \t   implicit input since anim curves could be used for reasons\n\t      \t   other than animation (e.g. setDrivenKey)\n\t      \t2) Any other node which has the time node as input\n\t      \t   Since these are pretty generic we can only take note of how\n\t      \t   many of these there are, and how many output connections they\n\t      \t   have.\n\t      \n\t      When 'showDetails' is True then the CSV columns are:\n\t      \n\t      \tAnimationNode\t\t: Name of node providing animation (e.g. animCurve)\n\t      \tAnimationNodeType\t: Type of node providing animation (e.g. animCurveTL)\n\t      \tDrivenNode\t\t\t: Name of node it animates (e.g. transform1)\n\t      \tDrivenNodeType\t\t: Type of node being animated (e.g. transform)\n\t      \tKeyframes\t\t\t: Number of keyframes in the AnimationNode\n\t      \t\t\t\t\t\t  Count is 0 if the AnimationNode is not an anim curve.\n\t      \tReallyAnimated\t\t: 1 if there is an anim curve and it is not flat, else 0\n\t      \n\t      One row is output for every AnimationNode -> DrivenNode connection.\n\t      This can be an M -> N correspondence so this can be spread out over\n\t      multiple rows.\n\t      \n\t      The above rows are skipped when 'showDetails' is False and then in\n\t      either case some summary rows are printed, one per animation node type:\n\t      \n\t      \tAnimationNode\t\t: # of nodes of type 'AnimationNodeType'\n\t      \tAnimationNodeType\t: Type of node providing animation (e.g. animCurveTL)\n\t      \tDrivenNode\t\t\t: ''\n\t      \tDrivenNodeType\t\t: ''\n\t      \tKeyframes\t\t\t: Sum of number of keyframes in all nodes of\n\t      \t\t\t\t\t\t  that type. Keyframe count is 0 for non-param\n\t      \t\t\t\t\t\t  curve nodes.\n\t      \tReallyAnimated\t\t: Number of non-flat curves on nodes of the given type\n\t      \n\t      Return True if the analysis succeeded, else False\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names 'stdout' and 'stderr' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n"
    
    
    isAnalytic = True



