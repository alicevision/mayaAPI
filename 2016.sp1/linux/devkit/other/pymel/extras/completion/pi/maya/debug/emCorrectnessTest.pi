"""
Utility to verify that the evaluation manager is yielding the same results
as the Maya DG evaluation.

If resultsPath is set then the graph output and differences are dumped to
files using that path as the base name.  For example:

    resultsPath           = MyDirectory/emCorrecteness_animCone
    DG results dump       = MyDirectory/emCorrecteness_animCone.dg.txt
    DG results image      = MyDirectory/emCorrecteness_animCone.dg.png
    ${mode} results dump  = MyDirectory/emCorrecteness_animCone.${mode}.txt
    ${mode} results image = MyDirectory/emCorrecteness_animCone.${mode}.png

If resultsPath is not set then no output is stored, everything is live.

The return value is a list of value pairs indicating number of differences
between the DG evaluation and the EM mode. e.g. if you requested 'ems'
mode then you would get back {'ems' : 0} from a successful comparison.

Legal values for modes are 'ems' and 'emp' with an optional '+XXX' to
indicate that evaluator XXX should be turned on during the test or an
optional '-XXX' to indicate that it should be turned off. Evaluator
states will be returned to their original value afte the test is run.
If an evaluator is not explicitly listed the current state of it will
be used for the test.

e.g. modes=['ems','emp+deformer']  means run the comparison twice, first
against EM Serial mode, the second time against EM Parallel mode with
the 'deformer' evaluator turned on. You can use multiple evaluators if
you wish: modes=['ems+deformer-dynamics'].

For now the images are not compared, only the attribute values.

If fileName is not set then the current scene is analyzed.

Sample usage to run the tests on a single file:

    from maya.debug.emCorrectnessTest import emCorrectnessTest
    serialErrors = emCorrectnessTest(fileName='MyDir/MyFile.ma', resultsPath='MyDir/emCorrectness', modes=['ems'])[1]

Sample usage to run the tests on the current scene in parallel mode with the deformer evaluator and ignore output:

    from maya.debug.emCorrectnessTest import emCorrectnessTest
    parallelErrors = emCorrectnessTest(modes=['emp+deformer'])
"""

import math
import os
import subprocess
import maya.cmds as cmds
import hashlib
import tempfile
import re

from maya.debug.TODO import TODO
from maya.debug.emModeManager import emModeManager

class DGState(object):
    """
    State object containing all data values that come out of the dbpeek
    command using the 'data' operation for simple data values and the
    dbpeek 'mesh' operation for mesh geometry.
    
    resultsFiles:    Where the intermediate results are stored. None means don't store them.
    imageFile:        Where the screenshot is stored. None means don't store it.
    state:            Data state information from the scene.
    """
    
    
    
    def __init__(self, resultsFile=None, imageFile=None, doEval=False, dataTypes=None):
        """
        Create a new state object, potentially saving results offline if
        requested.
        
        resultsFile : Name of file in which to save the results.
                      Do not save anything if None.
        imageFile   : Name of file in which to save the current viewport screenshot.
                      Do not save anything if None.
        doEval      : True means force evaluation of the plugs before checking
                      state. Used in DG mode since not all outputs used for
                      (e.g.) drawing will be in the datablock after evaluation.
        dataTypes   : Type of data to look for - {mesh, vertex, number, screen}
                      If screen is in the list the 'imageFile' argument must also be specified.
        """
    
        pass
    
    
    def compare(self, other, verbose):
        """
        Compare two state information collections and return a count of the
        number of differences. The first two fields (node,plug) are used to
        uniquely identify the line so that we are sure we are comparing the
        same two things.
        
        The 'clean' flag in column 2 is omitted from the comparison since
        the DG does funny things with the flag to maintain the holder/writer
        states of the data.
        
        other:      Other DGstate to compare against
        verbose:    If True then print the differences as they are found
        
        If verbose is False return the 3-tuple of integers (additionCount, changeCount, removalCount)
        If verbose is True return the 3-tuple of lists (additions, changes, removals)
        """
    
        pass
    
    
    def filterState(self, plugFilter):
        """
        Take the current state information and filter out all of the plugs
        not on the plugFilter list. This is used to restrict the output to
        the set of plugs the EM is evaluating.
        
        plugFilter: Dictionary of nodes whose values are dictionaries of
                    root level attributes that are to be used for the
                    purpose of the comparison.
        
                    None means no filter, i.e. accept all plugs.
        """
    
        pass
    
    
    def getMD5(self):
        """
        Get the MD5 checksum from the image file, if it exists.
        Return '' if the image file wasn't generated for an easy match.
        """
    
        pass
    
    
    __dict__ = None
    
    __weakref__ = None
    
    RE_INVERSE_MATRIX = 'InverseMatrix'



def __isMayaFile(path):
    """
    Check to see if the given path is a Maya file. Only looks for native Maya
    files ".ma" and ".mb", not other importable formats such as ".obj" or ".dxf"
    """

    pass


def _playback(maxFrames=200):
    """
    Run a playback sequence, starting at the first frame and going to
    the maxFrame requested.
    
    The only reason this is a separate method is so that the number of
    frames being played can be capped.
    """

    pass


def emCorrectnessTest(fileName=None, resultsPath=None, verbose=False, modes=['ems'], maxFrames=200, dataTypes=['matrix', 'vertex', 'screen']):
    """
    Evaluate the file in multiple modes and compare the results.
    
    fileName:    Name of file to load for comparison. None means use the current scene
    resultsPath: Where to store the results. None means don't store anything
    verbose:     If True then dump the differing values when they are encountered
    modes:       List of modes to run the tests in. 'ems' and 'emp' are the
                 only valid ones. A mode can optionally enable or disable an
                 evaluator as follows:
                     'ems+deformer': Run in EM Serial mode with the deformer evalutor turned on
                     'emp-dynamics': Run in EM Parallel mode with the dynamics evalutor turned off
                     'ems+deformer-dynamics': Run in EM Serial mode with the dynamics evalutor
                                              turned off and the deformer evaluator turned on
    maxFrames:   Maximum number of frames in the playback, to avoid long tests.
    dataTypes:   List of data types to include in the analysis. These are the possibilities:
                 matrix: Any attribute that returns a matrix
                 vertex: Attributes on the mesh shape that hold vertex positions
                 number: Any attribute that returns a number
                 screen: Screenshot after the animation runs
    
    Returns a list of value tuples indicating the run mode and the number of
    (additions,changes,removals) encountered in that mode. e.g. ['ems', (0,0,0)]
    
    If verbose is true then instead of counts return a list of actual changes.
    e.g. ['ems', ([], ["plug1,oldValue,newValue"], [])]
    
    Changed values are a CSV 3-tuple with "plug name", "value in DG mode", "value in the named EM mode"
    in most cases.
    
    In the special case of an image difference the plug name will be one
    of the special ones below and the values will be those generated by the
    comparison method used:
        SCREENSHOT_PLUG_MD5 : MD5 values when the image compare could not be done
        SCREENSHOT_PLUG_MAG : MD5 and image difference values from ImageMagick
        SCREENSHOT_PLUG_IMF : MD5 and image difference values from imf_diff
    """

    pass


def __findEmPlugs(emPlugs):
    """
    Find all of the root level plugs that the EM will be marking
    dirty. The passed-in dictionary will be populated by a list of
    dictionaries.
    
    emPlugs[NODE] = {DIRTY_PLUG_IN_NODE:True}
    """

    pass


def __hasEvaluationManager():
    """
    Check to see if the evaluation manager is available
    """

    pass



RE_ROOT_ATTRIBUTE = None

SCREENSHOT_PLUG_IMF = '__screenShot__.imf'

SCREENSHOT_PLUG_MD5 = '__screenShot__.md5'

NO_SIGNIFICANT_DIGITS_MATCH = 0

RE_IMF_COMPARE = None

SIGNIFICANT_DIGITS = 1.9

MD5_BLOCKSIZE = 33024

SCREENSHOT_NODE = '__screenShot__'

EMCORRECTNESS_MAX_FRAMECOUNT = 200

ALL_SIGNIFICANT_DIGITS_MATCH = 999

RE_IMG_COMPARE = None

SIGNIFICANT_DIGITS_INVERSE = 0.9

IMAGEMAGICK_MATCH_TOLERANCE = 1.0

SCREENSHOT_PLUG_MAG = '__screenShot__.mag'


