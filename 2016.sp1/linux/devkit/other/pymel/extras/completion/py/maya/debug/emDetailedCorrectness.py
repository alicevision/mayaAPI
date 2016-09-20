"""
Utility to dump the correctness data for some failing files in more detail
with summaries including the type of nodes and attributes that are failing.

If these environment variables are defined they will be used to locate the
files for analyzing, otherwise a file browser will open to locate them.

    EM_CORRECTNESS_RESULTS  : Directory in which to store the results
    EM_CORRECTNESS_FILES    : Directory at which to find the files to analyze
"""

import maya.cmds as cmds
import shutil
import re
import os

from maya.debug.emCorrectnessTest import emCorrectnessTest

def __getFileLocations():
    """
    Find the file locations using the environment variables if they exist and
    calls to the file dialog if they do not.
    
    Returns a 2-tuple if (RESULTS_ROOT, DATA_LOCATION_ROOT)
    """

    pass


def __summarizeCorrectnessInScene(resultsDir, dataRoot):
    """
    Summarize the current scene's emCorrectnessTest results, placing the
    detailed results into "resultsDir" in a subdirectory corresponding to the
    scene's name, relative to the dataRoot.
        e.g. if dataRoot = "~/Files/Weta/Avatar/AvatarScene.ma"
        and resultsDir = "~/Results" then the results will be stored
        in "~/Results/Weta/Avatar/AvatarScene.ma/"
    
    resultsDir : Root directory where the results files will be stored
    dataRoot   : Root directory where all of the data files are stored
    """

    pass


def __summarizeCorrectness(resultsDir='.', dataRoot='.', files=None):
    """
    Summarize the correctness results.
    Returns a list of the directories in which results can be found.
    """

    pass


def onFile():
    """
    Run the detailed correctness tests with summary on the selected scene.
    Returns a list of the directories in which results can be found.
    """

    pass


def onDirectory():
    """
    Run the detailed correctness tests with summary on all of the scenes
    below the selected directory.
    Returns a list of the directories in which results can be found.
    """

    pass



DATA_LOCATION_ROOT = None

RESULTS_ENV = 'EM_CORRECTNESS_RESULTS'

MAYA_FILE_FILTER = 'Maya Files (*.ma *.mb);;Maya ASCII (*.ma);;Maya Binary (*.mb)'

DATA_LOCATION_ENV = 'EM_CORRECTNESS_FILES'

SCREENSHOT_NODE = '__screenShot__'


