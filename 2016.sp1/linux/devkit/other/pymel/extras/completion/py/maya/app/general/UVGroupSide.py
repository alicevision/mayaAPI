import re
from . import UVSubSideBar as _UVSubSideBar
import maya.mel as mel
import sys
import maya.cmds as cmds
import os

from PySide.QtGui import *
from PySide.QtCore import *

from random import randint
from maya.app.general.UVSubSideBar import UVSubSideBar
from maya.app.general.UVGroupSection import UVGroupSection

class UVGroupSide(UVSubSideBar):
    def __init__(self):
        pass
    
    
    staticMetaObject = None



