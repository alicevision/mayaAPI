# Copyright 2015 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.

from maya import cmds
from maya import mel
from maya import OpenMayaUI as omui 
from PySide.QtCore import * 
from PySide.QtGui import * 
from shiboken import wrapInstance 

mayaMainWindowPtr = omui.MQtUtil.mainWindow() 
mayaMainWindow = wrapInstance(long(mayaMainWindowPtr), QWidget) 

class CreatePolygonUI(QWidget):
    def __init__(self, *args, **kwargs):
        super(CreatePolygonUI, self).__init__(*args, **kwargs)
        self.setParent(mayaMainWindow)
        self.setWindowFlags(Qt.Window)
        self.setObjectName('CreatePolygonUI_uniqueId')
        self.setWindowTitle('Create polygon')
        self.setGeometry(50, 50, 250, 150)
        self.initUI()
        self.cmd = 'polyCone'
        
    def initUI(self):
        self.combo = QComboBox(self)
        self.combo.addItem( 'Cone' )
        self.combo.addItem( 'Cube' )
        self.combo.addItem( 'Sphere' )
        self.combo.addItem( 'Torus' )
        self.combo.setCurrentIndex(0)
        self.combo.move(20, 20)
        self.combo.activated[str].connect(self.combo_onActivated)

        self.button = QPushButton('Create', self)
        self.button.move(20, 50)
        self.button.clicked.connect(self.button_onClicked)
        
        
    def combo_onActivated(self, text):
        self.cmd = 'poly' + text + '()'
        
    def button_onClicked(self):
        mel.eval( self.cmd )
            
def main():
    ui = CreatePolygonUI()
    ui.show()
    return ui
    
if __name__ == '__main__':
    main()