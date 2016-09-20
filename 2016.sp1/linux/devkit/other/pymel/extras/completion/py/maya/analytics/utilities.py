"""
Collection of general utilities for use with Maya analytics. See the help
string for each method for more details.

        listAnalytics                           : List all of the available analytics
        runAllAnalytics                         : Run all of the analytics on the current scene
        runAnalytics                            : Run a ist of analytics on the current scene
        runAllAnalyticsOnDirectory      : Run all analytics on all files in the directory
        runAnalyticsOnDirectory         : Run a list of analytics on all files in the directory
"""

import sys
import math
import importlib
import os
import pkgutil
import maya

from maya.analytics.MayaFileIterator import MayaFileIterator
from genericpath import isfile
from posixpath import join
from maya.analytics.MayaFileIterator import getMayaFilesByDirectory

def __analyticByName(analyticName):
    """
    Get an analytic class object by name. If no anaytic of that name exists
    then a KeyError exception is raised.
    """

    pass


def addAnalytic(name, cls):
    """
    Add a new analytic to the global list. Used by the decorator
    'makeAnalytic' to mark a class as being an analytic.
    """

    pass


def runAnalytics(analyticNames, details=False, outputDir=None):
    """
    This method will run the listed analytics in their default configurations.
    
    analyticNames = List of analytics to be run
    details = Instruct the analytics to dump all of their details. This
                      will contain specific information about the scene such as
                      node names so only use if you wish to expose those details.
                      See the help() for each analytic to see exactly what they
                      will expose when the details are turned on.
    outputDir = Destination directory for the output files. If not specified
                            then all output will go to your 'temporary' directory.
    """

    pass


def runAllAnalytics(details=False, outputDir=None):
    """
    This method will run all of the available analytics in their default configurations.
    
    details = Instruct the analytics to dump all of their details. This
                      will contain specific information about the scene such as
                      node names so only use if you wish to expose those details.
                      See the help() for each analytic to see exactly what they
                      will expose when the details are turned on.
    outputDir = Destination directory for the output files. If not specified
                            then all output will go to your 'temporary' directory.
    """

    pass


def runAllAnalyticsOnDirectory(srcDir=None, dstDir=None, doSubdirectories=True, skipReferences=True, details=False, force=False, listOnly=False, skipList=None):
    """
    Run all of the currently available Animation Analytics on every file in a
    directory. See runAnalyticsOnDirectory for a detailed description of what
    is to be run.
    """

    pass


def bootstrapAnalytics():
    """
    Bootstrap loading of the analytics in the same directory as this script.
    It only looks for files with the prefix "analytic" but you can add any
    analytics at other locations by using the @makeAnalytic decorator and
    importing them before calling listAnalytics.
    """

    pass


def listAnalytics():
    """
    List all of the objects in this packages that perform analysis of the
    Maya scene for output. They were gleaned from the list collected by
    the use of the @makeAnalytic decorator.
    
    The actual module names are returned. If you imported the module with a
    shorter alias use that instead.
    """

    pass


def runAnalyticsOnDirectory(analyticNames, srcDir=None, dstDir=None, doSubdirectories=True, skipReferences=True, details=False, force=False, listOnly=False, skipList=None):
    """
    Run all of the currently available Animation Analytics on every file in a
    directory.
    
    analyticNames = List of analytics to be run
    srcDir  = Directory in which the test files (.ma/.mb) can be found
    dstDir  = Root directory where the results are to be stored. Every file's
                      results will be stored in a subdirectory of this main directory
                      whose name is the same as the file (including the .ma/.mb).
                      If not specified it will default to srcDir/MayaAnalytics.
    doSubdirectories = Recurse down into the subdirectories of srcDir. If
                      dstDir is specified then all results go into that directory,
                      otherwise they will be relative to the directory being processed
                      (i.e. subdirectory's results go into the subdirectory).
    skipReferences = If any subdirectory has the name "references" then do not
                      descend into it. (Ignored when doSubdirectories is False.)
    details = Instruct the analytics to dump all of their details. This
                      will contain specific information about the scene such as
                      node names so only use if you wish to expose those details.
                      See the help() for each analytic to see exactly what they
                      will expose when the details are turned on.
    force   = If False then do not run analytics on files for which they
                      already exist. Only the default location (MayaAnalytics/FILE/)
                      is checked to determine if this is true.
    listOnly= If True then don't run the analytics, just list the files that
                      would be otherwise run.
    skipList= List of files to omit from the analytic run. Just the filename
                      is expected, not the full path.
    
    Returns a tuple (Done,Skipped,Failed,Results) with files analyzed, or that
    would have been analyzed if the listOnly flag was False, where:
            Done            = the list of files actually analyzed
            Skipped         = the list of files skipped because they were already analyzed
                                      or they appeared on the skipList
            Failed  = the list of files skipped due to earlier failed analysis
            Results = the list of file return results for each analytic. This return
                              value allows an analysis to produce results and also define
                              a return state for the success of the analytic's test. The
                              list is a tuple of (FILE, ANALYTIC, str(RESULT)). The result
                              is always converted to a string and it's up to the caller to
                              interpret it.
    """

    pass



_allAnalytics = {}


