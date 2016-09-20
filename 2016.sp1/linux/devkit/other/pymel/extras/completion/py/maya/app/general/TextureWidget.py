import re
import maya.mel as mel
import sys
import maya.cmds as cmds
import os

from PySide.QtCore import *
from PySide.QtGui import *

from random import randint
from maya.app.general.QUVEditor import QUVEditor

texWidget = None

gMainPane = []

removeMyTextureWindow = '\n                            global proc removeMyTextureWindow(string $whichPanel)\n                            {\n                                print(" remove "+$whichPanel);\n                                textureWindow -e -unParent $whichPanel;\n                                print(" end remove ");\n                            }\n                            '

createMyTextureWindow = '\n                            global proc createMyTextureWindow(string $whichPanel)\n                            {\n                                print(" create "+$whichPanel);\n                                textureWindow -unParent $whichPanel;\n                                print(" end create ");\n                            }\n                            '

texWinName = []

updateMyTextureWindow = '\n                            global proc txtWndUpdateEditor(string $editor, string $editorCmd, string $updatFunc, int $reason)\n                            {\n                            }\n                            '

addMyTextureWindow = '\n                        global proc addMyTextureWindow(string $whichPanel)\n                        {\n                                print(" add "+$whichPanel);\n                                string $formName = `formLayout`;\n                                // Attach the editor to the UI\n                                textureWindow -e -parent $formName $whichPanel;\n                                \n                                //string $textureEditorControl = `textureWindow -query -control $whichPanel`;\n\n                                //\n                                // If any changes goto update\n                                //\n                                //textureWindow -e \n                                //    -cc "txtWndUpdateEditor" $whichPanel\n                                //    "textureWindow" "null"\n                                //    $whichPanel;\n\n                                //txtWndUpdateEditor($whichPanel,"textureWindow","null",101);\n                                \n                                if (`exists performTextureViewGridOptions`)\n                                        performTextureViewGridOptions 0;\n                                \n                                // Change image range\n                                //if (`exists performTextureViewImageRangeOptions`)\n                                //        performTextureViewImageRangeOptions 0;\n\n                                print(" end add ");\n                        }\n                        '

columnLayout = None


