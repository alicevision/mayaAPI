"""
Utility to read and analyze dependency graph or evaluation graph structure
information. Allows you to produce a visualization of a single graph, a
text comparision of two graphs, or a merged comparison of two graphs.

        import maya.debug.graphStructure.graphStructure as graphStructure

        # Store the current scene's graph structure in a file
        cmds.dbpeek( op='graph', all=True, argument=['nodes','connections'], outputFile='FileForGraph.dg' )
        # or
        #       g = graphStructure()
        #   g.write( 'FileForGraph.dg' )

        # Get a new scene and get its structure directly
        cmds.file( 'MyTestFile.ma', force=True, open=True )
        graph1 = graphStructure()

        # Retrieve the stored graph structure
        graph2 = graphStructure( fileName='FileForGraph.dg' )

        # Compare them to see if they are the same
        if len(graph1.compare(graph2)) > 0:
                print 'Oh noooes, the graph structure has changed!'
                # Now visualize the differences
                graph1.compareAsDot(graph2, fileName='GraphCompare.dot', showOnlyDifferences=True)
"""

import sys
import maya.cmds as cmds
import re

class dotFormatting:
    """
    Encapsulation of all of the .dot formatting decisions made for this
    type of graph output.
    """
    
    
    
    def __init__(self, longNames=False):
        """
        If longNames is True then don't attempt to shorten the node names by
        removing namespaces and DAG path elements.
        """
    
        pass
    
    
    def alteredConnection(self, src, dst, inOriginal):
        """
        Print out code for a connection that was in one graph but not the other.
        If inOriginal is True the connection was in the original graph but not
        the secondary one, otherwise vice versa.
        """
    
        pass
    
    
    def alteredNodeFormat(self, inOriginal):
        """
        Print out formatting instruction for nodes that were in one graph
        but not the other. If inOriginal is True the nodes were in the
        original graph but not the secondary one, otherwise vice versa.
        """
    
        pass
    
    
    def contextNodeFormat(self):
        """
        Print out the formatting instruction to make nodes visible in the
        comparison graph but faded to indicate that they are actually the
        same and only present for context.
        """
    
        pass
    
    
    def footer(self):
        """
        Print this only once, at the end of the .dot file
        """
    
        pass
    
    
    def header(self):
        """
        Print this only once, at the beginning of the .dot file
        """
    
        pass
    
    
    def legend(self, file):
        """
        Print out a legend node. In the case of a graph dump this is only
        the title, containing the name of the file analyzed.
        """
    
        pass
    
    
    def legendForCompare(self, file1, file2, showOnlyDifferences):
        """
        Print out a legend node showing the formatting information for a
        comparison of two graphs.
        """
    
        pass
    
    
    def node(self, node):
        """
        Print out a graph node with a simplified label.
        """
    
        pass
    
    
    def nodeLabel(self, node):
        """
        Provide a label for a node. Uses the basename if useLongNames is not
        turned on, otherwise the full name.
        
        e.g.  Original:   grandparent|parent:ns1|:child
                  Label:      child
        """
    
        pass
    
    
    def simpleConnection(self, src, dst):
        """
        Print out a simple connection
        """
    
        pass
    
    
    def simpleNodeFormat(self):
        """
        Print out the formatting instruction to make nodes the default format.
        """
    
        pass
    
    
    aAndbStyle = 'penwidth="1.0", style="solid", color="#000000", fontcolor="#000000"'
    
    
    aNotbStyle = 'penwidth="1.0", style="dotted", color="#CC0000", fontcolor="#CC0000"'
    
    
    bNotaStyle = 'penwidth="4.0", style="solid", color="#127F12", fontcolor="#127F12"'
    
    
    contextStyle = 'penwidth="1.0", style="solid", color="#CCCCCC", fontcolor="#CCCCCC"'
    
    
    legendCompare = '\t{\n\t\trank = sink ;\n\t\tnode [shape=box] ;\n\t\t__bothGraphs [label="In both graphs", penwidth="1.0", style="solid", color="#000000", fontcolor="#000000"] ;\n\t\t__aButNotb [label="In graph 1 but not graph 2", penwidth="1.0", style="dotted", color="#CC0000", fontcolor="#CC0000"] ;\n\t\t__bButNota [label="In graph 2 but not graph 1", penwidth="4.0", style="solid", color="#127F12", fontcolor="#127F12"] ;\n\t\t\n\t}'
    
    
    legendCompareOnlyDifferences = '\t{\n\t\trank = sink ;\n\t\tnode [shape=box] ;\n\t\t__bothGraphs [label="In both graphs", penwidth="1.0", style="solid", color="#000000", fontcolor="#000000"] ;\n\t\t__aButNotb [label="In graph 1 but not graph 2", penwidth="1.0", style="dotted", color="#CC0000", fontcolor="#CC0000"] ;\n\t\t__bButNota [label="In graph 2 but not graph 1", penwidth="4.0", style="solid", color="#127F12", fontcolor="#127F12"] ;\n\t\t__context [label="In both graphs, shown for context", penwidth="1.0", style="solid", color="#CCCCCC", fontcolor="#CCCCCC"] ;\n\t}'
    
    
    legendFmt = '\t{\n\t\trank = sink ;\n\t\tnode [shape=box] ;\n\t\t__bothGraphs [label="In both graphs", penwidth="1.0", style="solid", color="#000000", fontcolor="#000000"] ;\n\t\t__aButNotb [label="In graph 1 but not graph 2", penwidth="1.0", style="dotted", color="#CC0000", fontcolor="#CC0000"] ;\n\t\t__bButNota [label="In graph 2 but not graph 1", penwidth="4.0", style="solid", color="#127F12", fontcolor="#127F12"] ;\n\t\t%s\n\t}'


class graphStructure:
    """
    Provides access and manipulation on graph structure data that has been
    produced by the 'dbpeek -op graph' or 'dbpeek -op evaluationGraph' commands.
    """
    
    
    
    def __init__(self, structureFileName=None, longNames=False, evaluationGraph=False):
        """
        Create a graph structure object from a file or the current scene.
        If the structureFileName is None then the current scene will be used,
        otherwise the file will be read.
        
        The graph data is read in and stored internally in a format that makes
        formatting and comparison easy.
        
        If longNames is True then don't attempt to shorten the node names by
        removing namespaces and DAG path elements.
        
        If evaluationGraph is True then get the structure of the evaluation
        manager graph, not the DG. This requires that the graph has already
        been created of course, e.g. by playing back a frame or two in EM
        serial or EM parallel mode.
        """
    
        pass
    
    
    def compare(self, other):
        """
        Compare this graph structure against another one and generate a
        summary of how the two graphs differ. Differences will be returned
        as a string list consisting of difference descriptions. That way
        when testing, an empty return means the graphs are the same.
        
        The different argument formats are:
        
                NodeAdded: N                    Nodes in self but not in other
                NodeRemoved: N                  Nodes in other but not in self
                PlugOutAdded: P                 Output plugs in self but not in other
                PlugOutRemoved: P               Output plugs in other but not in self
                PlugWorldAdded: P               Worldspace output plugs in self but not in other
                PlugWorldRemoved: P             Worldspace output plugs in other but not in self
                PlugInAdded: P                  Input plugs in self but not in other
                PlugInRemoved: P                Input plugs in other but not in self
                ConnectionAdded: S D    Connections in self but not in other
                ConnectionRemoved: S D  Connections in other but not in self
        """
    
        pass
    
    
    def compareAsDot(self, other, fileName=None, showOnlyDifferences=False):
        """
        Compare this graph structure against another one and print out a
        .dot format for visualization in an application such as graphViz.
        
        The two graphs are overlayed so that the union of the graphs is
        present. Colors for nodes and connetions are:
        
                Black      : They are present in both graphs
                Red/Dotted : They are present in this graph but not the alternate graph
                Green/Bold : They are present in the alternate graph but not this graph
        
        If the fileName is specified then the output is sent
        to that file, otherwise it is printed to stdout.
        
        If showOnlyDifferences is set to True then the output will omit all of
        the nodes and connections the two graphs have in common. Some common
        nodes may be output anyway if there is a new connection between them.
        
        Plugs have no dot format as yet.
        """
    
        pass
    
    
    def name(self):
        """
        Return an identifying name for this graph structure.
        """
    
        pass
    
    
    def write(self, fileName=None):
        """
        Dump the graph in the .dg format it uses for reading. Useful for
        creating a dump file from the current scene, or just viewing the
        graph generated from the current scene. If the fileName is specified
        then the output is sent to that file, otherwise it goes to stdout.
        """
    
        pass
    
    
    def writeAsDot(self, fileName=None):
        """
        Dump the graph in .dot format for visualization in an application
        such as graphViz. If the fileName is specified then the output is
        sent to that file, otherwise it is printed to stdout.
        
        Plugs have no dot format as yet.
        """
    
        pass



def splitConnection(connection):
    """
    Extract the name of a node and its attribute specification from
    one side of a connection.
    """

    pass


def checkMaya():
    pass



PLUG_OUT_TAG = '->'

_mayaIsAvailable = True

PLUG_WORLD_TAG = '-W>'

PLUG_IN_TAG = '<-'


