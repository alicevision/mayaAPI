import re
import PySide.QtGui as _QtGui
import maya.mel as mel
import sys
import maya.cmds as cmds
import os

from PySide.QtCore import *
from PySide.QtGui import *

from random import randint

class UVGenericSection(QGroupBox):
    def __init__(self, parent=None):
        pass
    
    
    def createIconButton(self, icon, function):
        pass
    
    
    def createSideButton(self, icon, text, function):
        pass
    
    
    def createSideInput(self, text, validator=None):
        pass
    
    
    def createTextButton(self, text, function):
        pass
    
    
    def getUIIcon(self, img):
        pass
    
    
    def mouseMoveEvent(self, event):
        pass
    
    
    staticMetaObject = None



SECTION_STYLE = '\n                    UVGenericSection{\n                        border: 1px solid black;\n                        /*font-size: 18px;*/\n                        font-weight: bold;\n                        background-clip: padding;\n                        margin-top: 7px;\n                        /*margin-bottom: 7px;*/\n                    }\n\n                    QGroupBox::title{\n                        subcontrol-origin: margin;\n                        left: 7px;\n                        top:   -10px;\n                        margin-top: 10px;\n                    }\n                '

ICON_HEIGHT = 19

TITLE_FONT_STYLE = '\n                    QGroupBox::title{\n                        color: yellow;\n                    }'

BUTTON_SYTLE = 'QToolButton{border: 0px solid black;};'


