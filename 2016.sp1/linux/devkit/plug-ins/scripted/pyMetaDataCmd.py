import sys
import os

import maya.api.OpenMaya as om

"""
To use, make sure that pyMetaDataCmd.py is in your MAYA_PLUG_IN_PATH
then do the following:

Select a node

import maya
maya.cmds.loadPlugin("pyMetaDataCmd.py")
maya.cmds.pyMetaData()

There should now be an extra attribute on your node called:
ownerName
with your user name as its string value
"""


def maya_useNewAPI():
    """
    The presence of this function tells Maya that the plugin produces, and
    expects to be passed, objects created using Maya Python API 2.0.
    """
    pass


class PyMetaDataCmd(om.MPxCommand):
    """
    Plug-in command named 'pyMetaData'.
    
    pyMetaData adds a dynamic string attribute to selected dependency nodes.
    Or, node names or a list of node names may be passed to the command.
    Dynamic attributes on nodes can be used as meta-data.

    The force/f flag is used to specify the command will overwrite existing
    dynamic string attributes of the same name.
    False will prevent overwriting pre-existing attributes.
    (default False)

    The value/v flag is used to specify a value to be set on the new attribute.
    (default user name from USERNAME or USER envar)

    The name/n flag is used to specify a name string used to name the attribute.
    (default 'userName')

    Undo/redo are supported.

    This command is written using Maya Python API 2.0.

    Example of usage, MEL:
    
    pyMetaData;
    pyMetaData -f; 
    pyMetaData -force -name priorityLevel -value High;
    pyMetaData -name health -v sick pSphere1 pSphere2;

    Example of usage, Python:
    
    maya.cmds.pyMetaData()
    maya.cmds.pyMetaData(f=True)
    maya.cmds.pyMetaData(force=True, name='priorityLevel', value='High')
    maya.cmds.pyMetaData('pSphere1','pSphere2',name='health', v='sick')
    maya.cmds.pyMetaData(['pSphere1','pSphere2'],name='health', v='sick')
    """

    # Variables to be used in Class 
    #
    cmdName = 'pyMetaData'

    nameFlag = '-n'
    nameFlagLong = '-name'

    forceFlag = '-f'
    forceFlagLong = '-force'

    valueFlag = '-v'
    valueFlagLong= '-value'


    def __init__(self):
        om.MPxCommand.__init__(self)

    @staticmethod
    def creator():
        return PyMetaDataCmd()

    @staticmethod
    def createSyntax():
        """
        Using kSelectionList in setObjectType
        and calling useSelectionAsDefault(True),
        allows us to use either a command line list of nodes,
        or use the active selection list if no node names are specified.
        
        The force/f flag is defined as requiring no arguments.
        By not specifying any arguments for the flag when we define it,
        the flag will not require any arguments when used in MEL,
        it will still require a value of 'True' or 'False' when used in Python.
        """
        syntax = om.MSyntax()
        syntax.setObjectType(om.MSyntax.kSelectionList)
        syntax.useSelectionAsDefault(True)
        syntax.addFlag(PyMetaDataCmd.forceFlag, PyMetaDataCmd.forceFlagLong)
        syntax.addFlag(PyMetaDataCmd.nameFlag, PyMetaDataCmd.nameFlagLong, om.MSyntax.kString)
        syntax.addFlag(PyMetaDataCmd.valueFlag, PyMetaDataCmd.valueFlagLong, om.MSyntax.kString)
        return syntax

    def doIt(self, args):
        # Use an MArgDatabase to Parse our command
        #
        try:
            argdb = om.MArgDatabase(self.syntax(), args)
        except RuntimeError:
            om.MGlobal.displayError('Error while parsing arguments:\n#\t# If passing in list of nodes, also check that node names exist in scene.')
            raise

        # SELECTION or LIST of Node Names
        #
        selList = argdb.getObjectList()

        # FORCE
        #
        force = argdb.isFlagSet(PyMetaDataCmd.forceFlag)

        # NAME
        #
        if argdb.isFlagSet(PyMetaDataCmd.nameFlag):
            name = argdb.flagArgumentString(PyMetaDataCmd.nameFlag, 0)
        else:
            name = 'ownerName'

        # VALUE
        #
        if argdb.isFlagSet(PyMetaDataCmd.valueFlag):
            value = argdb.flagArgumentString(PyMetaDataCmd.valueFlag, 0)
        else:
            if os.environ.has_key('USERNAME'):
                value = os.environ['USERNAME']
            elif os.environ.has_key('USER'):
                value = os.environ['USER']
            else:
                value = 'USERNAME'
                


        # Use an MDGModifier so we can undo this work
        #
        self.dgmod = om.MDGModifier()

        # Define a function to do most of the work
        #
        def addMetaData(dependNode, fnDN):
            # Check to see if the attribute already exists on the node
            #
            if fnDN.hasAttribute(name):
                if force:
                    plug = om.MPlug(dependNode, fnDN.attribute(name))
                    self.dgmod.newPlugValueString(plug, value)
                    self.dgmod.doIt()
                else:
                    print ('passing over "' + fnDN.name()+ '": force flag not set to True')
            else:
                # Create a new attribute 
                #
                fnAttr = om.MFnTypedAttribute()
                newAttr = fnAttr.create(name, name, om.MFnData.kString)

                # Now add the new attribute to the current dependency node,
                # using dgmod to facilitate undo
                #
                self.dgmod.addAttribute(dependNode, newAttr)
                self.dgmod.doIt()

                # Create a plug to set and retrieve value off the node.
                #
                plug = om.MPlug(dependNode, newAttr)
                
                # Set the value for the plug, using dgmod to facilitate undo
                #
                self.dgmod.newPlugValueString(plug, value)
                self.dgmod.doIt()


        # Iterate over all dependency nodes in list
        # add the meta-data attribute if it is not already there
        #
        for i in range(selList.length()):
            # Get the dependency node and create
            # a function set for it
            #
            try:
                dependNode = selList.getDependNode(i)
                fnDN = om.MFnDependencyNode(dependNode)
            except RuntimeError:
                om.MGlobal.displayWarning('item %d is not a depend node'%i)
                continue
            addMetaData(dependNode, fnDN)

    def redoIt(self):
        self.dgmod.doIt()

    def undoIt(self):
        self.dgmod.undoIt()

    def isUndoable(self):
        return True

def initializePlugin(plugin):
    pluginFn = om.MFnPlugin(plugin)
    pluginFn.registerCommand(
        PyMetaDataCmd.cmdName, PyMetaDataCmd.creator, PyMetaDataCmd.createSyntax
    )

def uninitializePlugin(plugin):
    pluginFn = om.MFnPlugin(plugin)
    pluginFn.deregisterCommand(PyMetaDataCmd.cmdName)

#-
# ==========================================================================
# Copyright (C) 2011 Autodesk, Inc. and/or its licensors.  All
# rights reserved.
#
# The coded instructions, statements, computer programs, and/or related
# material (collectively the "Data") in these files contain unpublished
# information proprietary to Autodesk, Inc. ("Autodesk") and/or its
# licensors, which is protected by U.S. and Canadian federal copyright
# law and by international treaties.
#
# The Data is provided for use exclusively by You. You have the right
# to use, modify, and incorporate this Data into other products for
# purposes authorized by the Autodesk software license agreement,
# without fee.
#
# The copyright notices in the Software and this entire statement,
# including the above license grant, this restriction and the
# following disclaimer, must be included in all copies of the
# Software, in whole or in part, and all derivative works of
# the Software, unless such copies or derivative works are solely
# in the form of machine-executable object code generated by a
# source language processor.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
# AUTODESK DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED
# WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF
# NON-INFRINGEMENT, MERCHANTABILITY OR FITNESS FOR A PARTICULAR
# PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE, OR
# TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS LICENSORS
# BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL,
# DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK
# AND/OR ITS LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY
# OR PROBABILITY OF SUCH DAMAGES.
#
# ==========================================================================
#+
