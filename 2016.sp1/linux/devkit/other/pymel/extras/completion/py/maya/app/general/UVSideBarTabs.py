import inspect
import re
import PySide.QtGui as _QtGui
import maya.mel as mel
import sys
import maya.cmds as cmds
import os

from PySide.QtCore import *
from PySide.QtGui import *

from random import randint

class UVSideBarTabs(QDockWidget):
    def __init__(self):
        pass
    
    
    def getUIIcon(self, img):
        pass
    
    
    def onDockLocationChange(self, area):
        pass
    
    
    staticMetaObject = None



ICON_HEIGHT = 15

SEPARATOR_STYLE = '   QLabel {\n                            background: #000000;\n                        }'

TAB_STYLE = ' QTabWidget {\n                    background: #454545;\n                }'

UI_ICON_PATH = 'C:/t_cheus/clients/mainline/Maya/src/PolyUISlice/UI/bitmaps/'


