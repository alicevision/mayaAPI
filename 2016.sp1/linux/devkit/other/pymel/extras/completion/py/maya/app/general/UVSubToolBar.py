import re
import PySide.QtGui as _QtGui
import maya.cmds as cmds
import maya.mel as mel
import sys
import os

from PySide.QtCore import *
from PySide.QtGui import *

from random import randint

class UVSubToolBar(QWidget):
    def __init__(self):
        pass
    
    
    def collapseButtonGroup(self):
        pass
    
    
    def createPushButton(self, icon, function, text, toolTip=None):
        pass
    
    
    def createSeparator(self):
        pass
    
    
    def createToggleButton(self, icon, function, toolTip=None):
        pass
    
    
    def getUIIcon(self, img):
        pass
    
    
    def onFloat(self):
        pass
    
    
    def putHorButtonLayout(self):
        pass
    
    
    def putVerButtonLayout(self):
        pass
    
    
    def updateButtons(self):
        pass
    
    
    def updateOrientation(self, orientation):
        pass
    
    
    def updateSeparator(self):
        pass
    
    
    COLUMN_COUNT = 2
    
    
    staticMetaObject = None



SEPARATOR_SHORT = 7

SEPARATOR_LONG = 56

SYTLE = 'QToolButton  {\n                            border: 0px solid black;\n                            margin: 0px;\n                        };'

ICON_HEIGHT = 25

ICON_WIDTH = ICON_HEIGHT

