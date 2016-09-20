# Copyright 2015 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.

"""
Attribute Editor style widget

Notes:
  * Uses mayaMixin to handle details of using PySide in Maya

Limitations:
  * Assumption that the node name does not change.  For handling node names, look into using MObjectHandle
  * File->New and File->Load not handled
  * Deleting node not handled
"""

from maya import cmds
from maya import mel
from maya import OpenMaya as om
from maya import OpenMayaUI as omui 

from PySide.QtCore import * 
from PySide.QtGui import * 
from PySide.QtUiTools import *
from shiboken import wrapInstance 

from maya.app.general.mayaMixin import MayaQWidgetBaseMixin, MayaQWidgetDockableMixin
import functools


class MCallbackIdWrapper(object):
    '''Wrapper class to handle cleaning up of MCallbackIds from registered MMessage
    '''
    def __init__(self, callbackId):
        super(MCallbackIdWrapper, self).__init__()
        self.callbackId = callbackId

    def __del__(self):
        om.MMessage.removeCallback(self.callbackId)

    def __repr__(self):
        return 'MCallbackIdWrapper(%r)'%self.callbackId


def getDependNode(nodeName):
    """Get an MObject (depend node) for the associated node name

    :Parameters:
        nodeName
            String representing the node
    
    :Return: depend node (MObject)

    """
    dependNode = om.MObject()
    selList = om.MSelectionList()
    selList.add(nodeName)
    if selList.length() > 0: 
        selList.getDependNode(0, dependNode)
    return dependNode


class Example_connectAttr(MayaQWidgetDockableMixin, QScrollArea):
    def __init__(self, node=None, *args, **kwargs):
        super(Example_connectAttr, self).__init__(*args, **kwargs)

        # Member Variables
        self.nodeName = node               # Node name for the UI
        self.attrUI = None                 # Container widget for the attr UI widgets
        self.attrWidgets = {}              # Dict key=attrName, value=widget
        self.nodeCallbacks = []            # Node callbacks for handling attr value changes
        self._deferredUpdateRequest = {}   # Dict of widgets to update

        # Connect UI to the specified node
        self.attachToNode(node)


    def attachToNode(self, nodeName):
        '''Connect UI to the specified node
        '''
        self.nodeName = nodeName
        self.attrs = None
        self.nodeCallbacks = []
        self._deferredUpdateRequest.clear()
        self.attrWidgets.clear()

        # Get a sorted list of the attrs
        attrs = cmds.listAttr(self.nodeName)
        attrs.sort() # in-place sort the attrs

        # Create container for attr widgets
        self.setWindowTitle('ConnectAttrs: %s'%self.nodeName)
        self.attrUI = QWidget(parent=self)
        layout = QFormLayout()

        # Loop over the attrs and construct widgets
        acceptedAttrTypes = set(['doubleLinear', 'string', 'double', 'float', 'long', 'short', 'bool', 'time', 'doubleAngle', 'byte', 'enum'])
        for attr in attrs:
            # Get the attr value (and skip if invalid)
            try:
                attrType = cmds.getAttr('%s.%s'%(self.nodeName, attr), type=True)
                if attrType not in acceptedAttrTypes:
                    continue # skip attr
                v = cmds.getAttr('%s.%s'%(self.nodeName, attr))
            except Exception, e:
                continue  # skip attr

            # Create the widget and bind the function
            attrValueWidget = QLineEdit(parent=self.attrUI)
            attrValueWidget.setText(str(v))

            # Use functools.partial() to dynamically constructing a function
            # with additional parameters.  Perfect for menus too.
            onSetAttrFunc =  functools.partial(self.onSetAttr, widget=attrValueWidget, attrName=attr)
            attrValueWidget.editingFinished.connect( onSetAttrFunc )

            # Add to layout
            layout.addRow(attr, attrValueWidget)

            # Track the widget associated with a particular attr
            self.attrWidgets[attr] = attrValueWidget

        # Attach the QFormLayout to the root attrUI widget
        self.attrUI.setLayout(layout)

        # Assign the widget to this QScrollArea
        self.setWidget(self.attrUI)

        # Do a 'connectControl' style operation with MMessage callbacks
        if len(self.attrWidgets) > 0:
            # Note: addNodeDirtyPlugCallback better than addAttributeChangedCallback
            # for UI since the 'dirty' check will always refresh the value of the attr
            nodeObj = getDependNode(nodeName)
            cb = om.MNodeMessage.addNodeDirtyPlugCallback(nodeObj, self.onDirtyPlug, None)
            self.nodeCallbacks.append( MCallbackIdWrapper(cb) )


    def onSetAttr(self, widget, attrName, *args, **kwargs):
        '''Handle setting the attribute when the UI widget edits the value for it.
        If it fails to set the value, then restore the original value to the UI widget
        '''
        print "onSetAttr", attrName, widget, args, kwargs
        try:
            attrType = cmds.getAttr('%s.%s'%(self.nodeName, attrName), type=True)
            if attrType == 'string':
                cmds.setAttr('%s.%s'%(self.nodeName, attrName), widget.text(), type=attrType)
            else:
                cmds.setAttr('%s.%s'%(self.nodeName, attrName), eval(widget.text()))
        except Exception, e:
            print e
            curVal = cmds.getAttr('%s.%s'%(self.nodeName, attrName))
            widget.setText( str(curVal) )


    def onDirtyPlug(self, node, plug, *args, **kwargs):
        '''Add to the self._deferredUpdateRequest member variable that is then 
        deferred processed by self._processDeferredUpdateRequest(). 
        '''
        # get long name of the attr, to use as the dict key
        attrName = plug.partialName(False, False, False, False, False, True)

        # get widget associated with the attr
        widget = self.attrWidgets.get(attrName, None)
        if widget != None:
            # get node.attr string
            nodeAttrName = plug.partialName(True, False, False, False, False, True) 

            # Add to the dict of widgets to defer update
            self._deferredUpdateRequest[widget] = nodeAttrName

            # Trigger an evalDeferred action if not already done
            if len(self._deferredUpdateRequest) == 1:
                cmds.evalDeferred(self._processDeferredUpdateRequest, low=True)


    def _processDeferredUpdateRequest(self):
        '''Retrieve the attr value and set the widget value
        '''
        for widget,nodeAttrName in self._deferredUpdateRequest.items():
            v = cmds.getAttr(nodeAttrName)
            widget.setText(str(v))
            print "_processDeferredUpdateRequest ", widget, nodeAttrName, v
        self._deferredUpdateRequest.clear()


def main():
    obj = cmds.polyCube()[0]
    ui = Example_connectAttr(node=obj)
    ui.show(dockable=True, floating=True)
    return ui


if __name__ == '__main__':
    main()
