import PySide.QtGui as _QtGui
import uuid
import maya.cmds as cmds

from PySide.QtCore import *
from PySide.QtGui import *

class UVToolTray(QDockWidget):
    def __init__(self):
        pass
    
    
    def addTrayButton(self, text, function, toolTip=None, objName=''):
        pass
    
    
    def dragEnterEvent(self, event):
        pass
    
    
    def dragMoveEvent(self, event):
        pass
    
    
    def dragWidgetHorizontalLayout(self, event):
        pass
    
    
    def dragWidgetVerticalLayout(self, event):
        pass
    
    
    def dropEvent(self, event):
        pass
    
    
    def onDockLocationChange(self, area):
        pass
    
    
    def onTrayButtonClicked(self):
        pass
    
    
    def onTrayButtonClicked2(self, objName):
        pass
    
    
    signalTrayButtonClicked = None
    
    
    staticMetaObject = None


class TrayButton(QPushButton):
    def __init__(self, parent):
        pass
    
    
    def mouseMoveEvent(self, event):
        pass
    
    
    staticMetaObject = None



BUTTON_SYTLE = '\n                QPushButton { \n                    border-radius: 20px;\n                    background-color: rgb(176, 189, 194);\n                    color: rgb(0, 0, 0); \n                    font: 10pt "MS Shell Dlg 2";\n                    min-width: 80px;\n                    min-height: 80px; \n                }\n                QPushButton:hover {  \n                    border-radius: 18px;\n                    border: 2px solid #8f8f91; \n                }\n\n                QPushButton:pressed {\n                    background-color: rgb(98, 105, 108);\n                }\n\n                QPushButton:flat {\n                    border: none; /* no border for a flat push button */\n                }\n\n                QPushButton:default {\n                    border-color: navy; /* make the default button prominent */\n                }\n                '

ICON_HEIGHT = 40

ICON_WIDTH = ICON_HEIGHT

