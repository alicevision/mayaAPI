import re
import PySide.QtGui as _QtGui
import maya.mel as mel
import sys
import maya.cmds as cmds
import os

from PySide.QtCore import *
from PySide.QtGui import *

from random import randint

class UVSubSideBar(QDockWidget):
    def __init__(self):
        pass
    
    
    def collapseMenu(self):
        pass
    
    
    def createLayout(self):
        pass
    
    
    def createSeparator(self):
        pass
    
    
    def getMockUIIcon(self, img):
        pass
    
    
    def getUIIcon(self, img):
        pass
    
    
    def mouseMoveEvent(self, event):
        pass
    
    
    def tearOff(self, newTarget):
        pass
    
    
    def updateDockPosition(self, area):
        pass
    
    
    MOCK_UI_PATH = 'C:/Users/t_cheus/Documents/maya/scripts/UIAssets/'
    
    
    reattachSignal = None
    
    
    staticMetaObject = None
    
    
    tearOffSignal = None



BUTTON_SYTLE = 'QToolButton{\n                        border: 0px solid black;\n                };'

SEPARATOR_STYLE = 'QPushButton{\n                                background-color: #4E4E4E;\n                                color: #C8C8C8;\n                                font-weight:bold;\n                                font-size:12px;\n                                text-align: left;\n                                padding-left: 35px;\n                        }'

UI_ICON_PATH = 'C:/t_cheus/clients/mainline/Maya/src/PolyUISlice/UI/bitmaps/'

ICON_HEIGHT = 25

ICON_WIDTH = ICON_HEIGHT

