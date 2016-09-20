import re
import os

def MayaFileIterator(rootDirectory, skipMA=False, skipMB=False, skipFiles=(), skipDirectories=(), descend=True):
    """
    Iterator to walk all Maya files in or below a specific directory.
    
    Parameters:
            skipMA                  : Skip files with a .ma extension
            skipMB                  : Skip files with a .mb extension
            skipFile                : Do not process any files with the given name.
                                                    Name is given as a regular expression so multiple
                                                    files can be skipped using this. To match the
                                                    name exactly you must use escaped special characters.
    
                                                    '.*temp'        : Matches 'temp', 'xtemp', and 'xtempY'
                                                    '^temp$'        : Matches 'temp' only
                                                    '^t*p$'         : Matches 'temp' and 'trip' but not 'temps' or 'stamp'
            skipDirectories : Do not process any directories with the given name.
                                                    Name is given as a regular expression so multiple
                                                    directories can be skipped using this. To match
                                                    the name exactly you must use escaped special characters.
    
                                                    '.*temp'         : Matches 'temp', 'xtemp', and 'xtempY'
                                                    'temp\w'         : Matches 'temp.mb' or 'temp.file.mb' but not 'tempfile.mb'
                                                    't*p\w'  : Matches 'temp' and 'trip' but not 'temps' or 'stamp'
            descend                 : Recurse into directories
    
    Return:
            list of filepaths
    
    Note that only .ma and .mb extensions are recognized so if you turn on both
    skipMA and skipMB nothing will be found. The iterator gives the full path of
    each file found relative to the root directory.
    
    Usage:
            Find all Maya files under "root/projects" that aren't temporary files,
            defined as those named temp.ma, temp.mb, or that live in a temp/ subdirectory.
    
            import MayaFileIterator
            for path in MayaFileIterator.MayaFileIterator('Maya/projects',
                                                                                                      skipFiles=['^temp.m{a,b}$'],
                                                                                                      skipDirectories=['temp']):
                    print path
    """

    pass


def getMayaFilesByDirectory(generator):
    """
    Help function for the MayaFileIterator class that runs the generator
    and then packages up the results in a directory-centric format.
    
    Parameter
            generator : A MayaFileIterator function call, already constructed but not used
    
    Return value
            A list of ( DIRECTORY, [FILES] ) pairs consisting of
            all matching files from generation using the passed-in generator.
    
    theGen = MayaFileIterator("Maya/projects", skipFiles=['temp\w'])
    for (theDir,theFiles) in getMayaFilesByDirectory(theGen):
            print theDir
            for theFile in theFiles:
                    print ' -- ',theFile
    """

    pass



