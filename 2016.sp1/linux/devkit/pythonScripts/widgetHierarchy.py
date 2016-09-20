# Copyright 2015 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.

"""
Widget Hierarchy
"""

from PySide.QtCore import * 
from PySide.QtGui import * 
from shiboken import wrapInstance 
from maya import OpenMayaUI as omui 
from maya.app.general.mayaMixin import MayaQWidgetBaseMixin
from pprint import pprint


class WidgetHierarchyTree(MayaQWidgetBaseMixin, QTreeView):
    def __init__(self, rootWidget=None, *args, **kwargs):
        super(WidgetHierarchyTree, self).__init__(*args, **kwargs)

        # Destroy this widget when closed.  Otherwise it will stay around
        self.setAttribute(Qt.WA_DeleteOnClose, True)

        # Determine root widget to scan
        if rootWidget != None:
            self.rootWidget = rootWidget
        else:
            mayaMainWindowPtr = omui.MQtUtil.mainWindow() 
            self.rootWidget = wrapInstance(long(mayaMainWindowPtr), QWidget)

        self.populateModel()


    def populateModel(self):
        # Create the headers
        self.columnHeaders = ['Class', 'ObjectName', 'Children']
        myModel = QStandardItemModel(0,len(self.columnHeaders))
        for col,colName in enumerate(self.columnHeaders):
            myModel.setHeaderData(col, Qt.Horizontal, colName)

        # Recurse through child widgets
        parentItem = myModel.invisibleRootItem();
        self.populateModel_recurseChildren(parentItem, self.rootWidget)
        self.setModel( myModel )


    def populateModel_recurseChildren(self, parentItem, widget):
        # Construct the item data and append the row
        classNameStr = str(widget.__class__).split("'")[1]
        classNameStr = classNameStr.replace('PySide.','').replace('QtGui.', '').replace('QtCore.', '')
        items = [QStandardItem(classNameStr), 
                 QStandardItem(widget.objectName()),
                 QStandardItem(str(len(widget.children()))),
                 ]
        parentItem.appendRow(items)

        # Recurse children and perform the same action
        for childWidget in widget.children():
            self.populateModel_recurseChildren(items[0], childWidget)


def main():
    ui = WidgetHierarchyTree()
    ui.show()
    return ui


if __name__ == '__main__':
    main()
