# Copyright 2015 Autodesk, Inc. All rights reserved.
# 
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.

'''
Framework to specify the default widget or a derived widget by a standard UI.
This allows studios the ability to override the standard CreaseSetEditor
and replace it with their own and still use the existing menu items
for calling it.
'''

from PySide.QtCore import * 
from PySide.QtGui import * 
from maya.app.general.mayaMixin import MayaQWidgetBaseMixin

# ===============================
# Classes
class MyBasePushButton(MayaQWidgetBaseMixin, QPushButton):
    def __init__(self, node=None, *args, **kwargs):
        super(MyBasePushButton, self).__init__(*args, **kwargs)
        self.setText('Default Button')


class MyDerivedPushButton(MyBasePushButton):
    def __init__(self, node=None, *args, **kwargs):
        super(MyDerivedPushButton, self).__init__(*args, **kwargs)
        self.setText('Derived Button')

# ===============================
# Global: The default widget to use when called through getDefaultWidget()
_DefaultWidget = MyBasePushButton

# Get/Set defaultWidget
def getDefaultWidget():
    '''Get the current widget class used by default.
    '''
    # If the _DefaultCreaseSetEditor variable has been defined and overridden, then use it.
    #   Otherwise use the standard CreaseSetEditor
    return _DefaultWidget


def setDefaultWidget(cls=MyBasePushButton):
    '''Set the class to use by default.
    '''
    # Make sure the class passed in is the the base or derived widget
    if not issubclass(cls, MyBasePushButton):
        raise TypeError, 'Invalid class'%cls
    
    # Set the new default widget
    global _DefaultWidget
    _DefaultWidget = cls

# ===============================
def main():
    # Show the default widget
    widgetClass = getDefaultWidget() 
    w1 = widgetClass()
    w1.show()

    # Show the override widget with same getDefaultWidget()
    setDefaultWidget(MyDerivedPushButton)
    widgetClass = getDefaultWidget() 
    w2 = widgetClass()
    w2.show()
    return (w1, w2)


