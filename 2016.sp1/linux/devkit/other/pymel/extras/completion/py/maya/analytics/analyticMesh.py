import re
import math
import maya

from maya.analytics.BaseAnalytic import BaseAnalytic
from maya.analytics.decorators import addHelp
from maya.analytics.decorators import addMethodDocs
from maya.analytics.decorators import makeAnalytic

class analyticMesh(BaseAnalytic):
    """
    Analyze the volume and distribution of mesh data.
    """
    
    
    
    def run(self, showDetails=False):
        """
        Scan all of the Mesh shapes in the scene and provide a column for
        each node with the following statistics in it:
                - Number of vertices in the mesh
                - Number of faces in the mesh
                - Number of edges in the mesh
                - Number of triangles in the mesh
                - Number of UV coordinates in the mesh
                - Number of vertices "tweaked"
                - Is it using user normals?
                - What is the source node for the mesh? For meshes with no
                  construction history it will be the mesh itself. For others
                  it could be a polySphere or other creation operation, or some
                  other mesh at the beginning of a deformation chain. When
                  'showDetails' is off the nodeType is shown for non-meshes and
                  the generated name proxy is show for meshes.
        
        If you set the 'showDetails' argument to True then the names of the
        actual nodes in the scene will be used in the output. Otherwise the
        node names will be replaced by a generic name 'NODETYPEXXX' where
        'NODETYPE' is the type of node being reported (e.g. 'mesh') and 'XXX'
        is a unique index per node type.  (An appropriate number of leading
        '0's will be added so that all names are of the same length
        for easier reading.)
        """
    
        pass
    
    
    def help():
        """
        Call this method to print the class documentation, including all methods.
        """
    
        pass
    
    
    ANALYTIC_NAME = 'Mesh'
    
    
    __fulldocs__ = 'Analyze the volume and distribution of mesh data.\nBase class for output for analytics.\n\nThe default location for the anlaytic output is in a subdirectory\ncalled \'MayaAnalytics\' in your temp directory. You can change that\nat any time by calling setOutputDirectory().\n\nClass static member:\n\t ANALYTIC_NAME\t: Name of the analytic\n\nClass members:\n\t director\t\t: Directory the output will go to\n\t fileName\t\t: Name of the file for this analytic\n\t fileState\t\t: Is the output stream currently opened?\n\t\t\t\t\t  If FILE_DEFAULT then it isn\'t openable\n\t outputFile\t\t: Output File object for writing the results\n\n\tMethods\n\t-------\n\thelp : Call this method to print the class documentation, including all methods.\n\n\toutputExists : Checks to see if this analytic already has existing output at\n\t               the current directory location. If stdout/stderr this is always\n\t               false, otherwise it checks for the existence of a non-zero sized\n\t               output file. (All legal output will at least have a header line,\n\t               and we may have touched the file to create it in the call to\n\t               setOutputDirectory().\n\n\trun : Scan all of the Mesh shapes in the scene and provide a column for\n\t      each node with the following statistics in it:\n\t      \t- Number of vertices in the mesh\n\t      \t- Number of faces in the mesh\n\t      \t- Number of edges in the mesh\n\t      \t- Number of triangles in the mesh\n\t      \t- Number of UV coordinates in the mesh\n\t      \t- Number of vertices "tweaked"\n\t      \t- Is it using user normals?\n\t      \t- What is the source node for the mesh? For meshes with no\n\t      \t  construction history it will be the mesh itself. For others\n\t      \t  it could be a polySphere or other creation operation, or some\n\t      \t  other mesh at the beginning of a deformation chain. When\n\t      \t  \'showDetails\' is off the nodeType is shown for non-meshes and\n\t      \t  the generated name proxy is show for meshes.\n\t      \n\t      If you set the \'showDetails\' argument to True then the names of the\n\t      actual nodes in the scene will be used in the output. Otherwise the\n\t      node names will be replaced by a generic name \'NODETYPEXXX\' where\n\t      \'NODETYPE\' is the type of node being reported (e.g. \'mesh\') and \'XXX\'\n\t      is a unique index per node type.  (An appropriate number of leading\n\t      \'0\'s will be added so that all names are of the same length\n\t      for easier reading.)\n\n\tsetOutputDirectory : Call this method to set a specific directory as the output location.\n\t                     The special names \'stdout\' and \'stderr\' are recognized as the\n\t                     output and error streams respectively rather than a directory.\n'
    
    
    isAnalytic = True



reMultiGeometryOutput = None


