"""
Functions which are not listed in the maya documentation, such as commands created by plugins,
as well as the name parsing classes `DependNodeName`, `DagNodeName`, and `AttributeName`.
"""

import pymel.internal.factories as _factories
import inspect
import re
import pymel.internal.pmcmds as cmds

from pymel.internal.pmcmds import dR_testCmd
from pymel.internal.pmcmds import nexOpt
from pymel.internal.pmcmds import dR_contextChanged
from pymel.internal.pmcmds import mtkShrinkWrap
from pymel.internal.pmcmds import mtkQuadDrawPoint
from pymel.internal.pmcmds import dR_multiCutSlicePointCmd
from pymel.internal.pmcmds import dR_nexCmd
from pymel.internal.pmcmds import dR_multiCutPointCmd
from pymel.internal.pmcmds import dR_DoCmd

class NameParser(unicode):
    def __getattr__(self, attr):
        """
        >>> NameParser('foo:bar').spangle
        AttributeName(u'foo:bar.spangle')
        """
    
        pass
    
    
    def __repr__(self):
        pass
    
    
    def addPrefix(self, prefix):
        """
        addPrefixToName
        """
    
        pass
    
    
    def attr(self, attr):
        """
        access to AttributeName of a node. returns an instance of the AttributeName class for the
        given AttributeName.
        
            >>> NameParser('foo:bar').attr('spangle')
            AttributeName(u'foo:bar.spangle')
        """
    
        pass
    
    
    def namespace(self):
        """
        Returns the namespace of the object with trailing colon included
        """
    
        pass
    
    
    def namespaceList(self):
        """
        Useful for cascading references.  Returns all of the namespaces of the calling object as a list
        """
    
        pass
    
    
    def stripGivenNamespace(self, namespace, partial=True):
        """
        Returns a new instance of the object with any occurrences of the given namespace removed.  The calling instance is unaffected.
        The given namespace may end with a ':', or not.
        If partial is True (the default), and the given namespace has parent namespaces (ie, 'one:two:three'),
        then any occurrences of any parent namespaces are also stripped - ie, 'one' and 'one:two' would
        also be stripped.  If it is false, only namespaces
        
            >>> NameParser('foo:bar:top|foo:middle|foo:bar:extra:guy.spangle').stripGivenNamespace('foo:bar')
            AttributeName(u'top|middle|extra:guy.spangle')
        
            >>> NameParser('foo:bar:top|foo:middle|foo:bar:extra:guy.spangle').stripGivenNamespace('foo:bar', partial=False)
            AttributeName(u'top|foo:middle|extra:guy.spangle')
        """
    
        pass
    
    
    def stripNamespace(self, levels=0):
        """
        Returns a new instance of the object with its namespace removed.  The calling instance is unaffected.
        The optional levels keyword specifies how many levels of cascading namespaces to strip, starting with the topmost (leftmost).
        The default is 0 which will remove all namespaces.
        
            >>> NameParser('foo:bar.spangle').stripNamespace()
            AttributeName(u'bar.spangle')
        """
    
        pass
    
    
    def swapNamespace(self, prefix):
        """
        Returns a new instance of the object with its current namespace replaced with the provided one.
        The calling instance is unaffected.
        """
    
        pass
    
    
    def __new__(cls, strObj):
        """
        Casts a string to a pymel class. Use this function if you are unsure which class is the right one to use
        for your object.
        """
    
        pass
    
    
    __dict__ = None
    
    __weakref__ = None
    
    PARENT_SEP = '|'


class AttributeName(NameParser):
    def __call__(self, *args, **kwargs):
        """
        # Added the __call__ so to generate a more appropriate exception when a class method is not found
        """
    
        pass
    
    
    def __getitem__(self, item):
        pass
    
    
    def __init__(self, attrName):
        pass
    
    
    def add(self, **kwargs):
        pass
    
    
    def addAttr(self, **kwargs):
        pass
    
    
    def array(self):
        """
        Returns the array (multi) AttributeName of the current element
            >>> n = AttributeName('lambert1.groupNodes[0]')
            >>> n.array()
            AttributeName(u'lambert1.groupNodes')
        """
    
        pass
    
    
    def exists(self):
        pass
    
    
    def getParent(self, generations=1):
        """
        Returns the parent attribute
        
        Modifications:
            - added optional generations flag, which gives the number of levels up that you wish to go for the parent;
              ie:
                  >>> AttributeName("Cube1.multiComp[3].child.otherchild").getParent(2)
                  AttributeName(u'Cube1.multiComp[3]')
        
              Negative values will traverse from the top, not counting the initial node name:
        
                  >>> AttributeName("Cube1.multiComp[3].child.otherchild").getParent(-2)
                  AttributeName(u'Cube1.multiComp[3].child')
        
              A value of 0 will return the same node.
              The default value is 1.
        
              Since the original command returned None if there is no parent, to sync with this behavior, None will
              be returned if generations is out of bounds (no IndexError will be thrown).
        """
    
        pass
    
    
    def item(self, asSlice=False, asString=False):
        pass
    
    
    def lastPlugAttr(self):
        """
        >>> NameParser('foo:bar.spangle.banner').lastPlugAttr()
        u'banner'
        """
    
        pass
    
    
    def node(self):
        """
        plugNode
        
        >>> NameParser('foo:bar.spangle.banner').plugNode()
        DependNodeName(u'foo:bar')
        """
    
        pass
    
    
    def nodeName(self):
        """
        basename
        """
    
        pass
    
    
    def plugAttr(self):
        """
        plugAttr
        
        >>> NameParser('foo:bar.spangle.banner').plugAttr()
        u'spangle.banner'
        """
    
        pass
    
    
    def plugNode(self):
        """
        plugNode
        
        >>> NameParser('foo:bar.spangle.banner').plugNode()
        DependNodeName(u'foo:bar')
        """
    
        pass
    
    
    def set(self, *args, **kwargs):
        pass
    
    
    def setAttr(self, *args, **kwargs):
        pass
    
    
    attrItemReg = None


class DependNodeName(NameParser):
    def exists(self, **kwargs):
        """
        objExists
        """
    
        pass
    
    
    def extractNum(self):
        """
        Return the trailing numbers of the node name. If no trailing numbers are found
        an error will be raised.
        """
    
        pass
    
    
    def nextName(self):
        """
        Increment the trailing number of the object by 1
        """
    
        pass
    
    
    def nextUniqueName(self):
        """
        Increment the trailing number of the object until a unique name is found
        
        If there is no trailing number, appends '1' to the name.
        Will always return a different name than the current name, even if the
            current name already does not exist.
        """
    
        pass
    
    
    def node(self):
        """
        for compatibility with AttributeName class
        """
    
        pass
    
    
    def nodeName(self):
        """
        for compatibility with DagNodeName class
        """
    
        pass
    
    
    def prevName(self):
        """
        Decrement the trailing number of the object by 1
        """
    
        pass
    
    
    def stripNum(self):
        """
        Return the name of the node with trailing numbers stripped off. If no trailing numbers are found
        the name will be returned unchanged.
        """
    
        pass


class DagNodeName(DependNodeName):
    def firstParent(self):
        """
        firstParentOf
        """
    
        pass
    
    
    def getParent(self, generations=1):
        """
        Returns the parent node
        
        Modifications:
            - added optional generations flag, which gives the number of levels up that you wish to go for the parent;
              ie:
                  >>> DagNodeName("NS1:TopLevel|Next|ns2:Third|Fourth").getParent(2)
                  DagNodeName(u'NS1:TopLevel|Next')
        
              Negative values will traverse from the top, not counting the initial node name:
        
                  >>> DagNodeName("NS1:TopLevel|Next|ns2:Third|Fourth").getParent(-3)
                  DagNodeName(u'NS1:TopLevel|Next|ns2:Third')
        
              A value of 0 will return the same node.
              The default value is 1.
        
              Since the original command returned None if there is no parent, to sync with this behavior, None will
              be returned if generations is out of bounds (no IndexError will be thrown).
        """
    
        pass
    
    
    def getRoot(self):
        """
        unlike the root command which determines the parent via string formatting, this
        command uses the listRelatives command
        """
    
        pass
    
    
    def nodeName(self):
        """
        basename
        """
    
        pass
    
    
    def root(self):
        """
        rootOf
        """
    
        pass



def iterOnNurbs(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.iterOnNurbs`
    """

    pass


def adskAsset(*args, **kwargs):
    """
    Flags:
      - assetID : a                    (unicode)       []
    
      - library : l                    (unicode)       []
    
      - resolved : r                   (bool)          []
    
    
    Derived from mel command `maya.cmds.adskAsset`
    """

    pass


def greasePencil(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.greasePencil`
    """

    pass


def polySelectEditCtxDataCmd(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.polySelectEditCtxDataCmd`
    """

    pass


def cteRoster(*args, **kwargs):
    """
    Dynamic library stub function
    
    Flags:
      - absoluteTime : abs             (bool)          []
    
      - addAttribute : ada             (unicode)       []
    
      - addGroup : adg                 (unicode)       []
    
      - addObject : ado                (PyNode)        []
    
      - addRelatedKG : akg             (bool)          []
    
      - addSelectedObjects : aso       (bool)          []
    
      - animLayer : al                 (unicode)       []
    
      - append : ap                    (bool)          []
    
      - changeParent : cpr             (int)           []
    
      - clipEnd : ce                   (bool)          []
    
      - clipId : cid                   (int)           []
    
      - clipName : cn                  (bool)          []
    
      - clipStart : cs                 (bool)          []
    
      - copyAnimation : ca             (bool)          []
    
      - createDefaultClip : dcp        (bool)          []
    
      - exclusive : exc                (bool)          []
    
      - exists : exi                   (bool)          []
    
      - exportFbx : ex                 (unicode)       []
    
      - ghost : gh                     (bool)          []
    
      - hierarchy : h                  (bool)          []
    
      - importFbx : fbx                (unicode)       []
    
      - importMayaFile : mf            (unicode)       []
    
      - importOption : io              (unicode)       []
    
      - importSelection : importSelection (bool)          []
    
      - numClips : nc                  (int)           []
    
      - parent : p                     (int)           []
    
      - recursively : rec              (bool)          []
    
      - removeAll : rm                 (bool)          []
    
      - removeAttribute : rma          (int)           []
    
      - removeGroup : rmg              (int)           []
    
      - removeObject : rmo             (int)           []
    
      - renameGroup : rgp              (int, unicode)  []
    
      - rosterItemType : rit           (int)           []
    
      - selected : sel                 (bool)          []
    
      - startTime : mcl                (time)          []
    
      - type : typ                     (unicode)       []
    
    
    Derived from mel command `maya.cmds.cteRoster`
    """

    pass


def agFormatOut(*args, **kwargs):
    """
    Flags:
      - file : f                       (unicode)       []
    
    
    Derived from mel command `maya.cmds.agFormatOut`
    """

    pass


def adskAssetLibrary(*args, **kwargs):
    """
    Flags:
      - unload : ul                    (bool)          []
    
      - unloadAll : ua                 (bool)          []
    
    
    Derived from mel command `maya.cmds.adskAssetLibrary`
    """

    pass


def cteEditor(*args, **kwargs):
    """
    Dynamic library stub function
    
    Flags:
      - autoFit : af                   (unicode)       []
    
      - contextMenu : cm               (bool)          []
    
      - control : ctl                  (bool)          []
    
      - defineTemplate : dt            (unicode)       []
    
      - displayActiveKeyTangents : dat (unicode)       []
    
      - displayActiveKeys : dak        (unicode)       []
    
      - displayInfinities : di         (unicode)       []
    
      - displayKeys : dk               (unicode)       []
    
      - displayTangents : dtn          (unicode)       []
    
      - docTag : dtg                   (unicode)       []
    
      - exists : ex                    (bool)          []
    
      - filter : f                     (unicode)       []
    
      - forceMainConnection : fmc      (unicode)       []
    
      - highlightConnection : hlc      (unicode)       []
    
      - lockMainConnection : lck       (bool)          []
    
      - lookAt : la                    (unicode)       []
    
      - mainListConnection : mlc       (unicode)       []
    
      - menu : m                       (callable)      []
    
      - panel : pnl                    (unicode)       []
    
      - parent : p                     (unicode)       []
    
      - selectionConnection : slc      (unicode)       []
    
      - snapTime : st                  (unicode)       []
    
      - snapValue : sv                 (unicode)       []
    
      - stateString : sts              (bool)          []
    
      - togglekeyview : tkv            (bool)          []
    
      - unParent : up                  (bool)          []
    
      - unlockMainConnection : ulk     (bool)          []
    
      - updateMainConnection : upd     (bool)          []
    
      - useTemplate : ut               (unicode)       []
    
      - viewLeft : vl                  (bool)          []
    
      - viewRight : vr                 (bool)          []
    
    
    Derived from mel command `maya.cmds.cteEditor`
    """

    pass


def polyPrimitiveMisc(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.polyPrimitiveMisc`
    """

    pass


def dispatchGenericCommand(*args, **kwargs):
    """
    generic command dispatch function used for API commands
    
    
    Derived from mel command `maya.cmds.dispatchGenericCommand`
    """

    pass


def ogsdebug(*args, **kwargs):
    """
    Flags:
      - count : c                      (int)           []
    
      - debug : d                      (unicode)       []
    
      - timing : t                     (unicode)       []
    
      - verbose : v                    (bool)          []
    
    
    Derived from mel command `maya.cmds.ogsdebug`
    """

    pass


def flushIdleQueue(*args, **kwargs):
    """
    Flags:
      - resume : r                     (bool)          []
    
    
    Derived from mel command `maya.cmds.flushIdleQueue`
    """

    pass


def memoryDiag(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.memoryDiag`
    """

    pass


def dgControl(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.dgControl`
    """

    pass


def psdConvSolidTxOptions(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.psdConvSolidTxOptions`
    """

    pass


def TanimLayer(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.TanimLayer`
    """

    pass


def artSelect(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.artSelect`
    """

    pass


def dbPeek(*args, **kwargs):
    """
    The dbpeekcommand is used to analyze the Maya data for information of interest. See a description of the flags for
    details on what types of things can be analyzed.
    
    Dynamic library stub function
    
    Flags:
      - allObjects : all               (bool)          [create,query]
          Ignore any specified or selected objects and peek into all applicable objects. The definition of allObjectswill vary
          based on the peek operation being performed - see the flag documentation for details on what it means for a given
          operation. By default if no objects are selected or specified then it will behave as though this flag were set.
    
      - argument : a                   (unicode)       [create,query]
          Specify one or more arguments to be passed to the operation. The acceptable values for the argument string are
          documented in the flag to which they will be applied. If the argument itself takes a value then the value will be of the
          form argname=argvalue.
    
      - count : c                      (int)           [create,query]
          Specify a count to be used by the test. Different tests make different use of the count, query the operation to find out
          how it interprets the value. For example a performance test might use it as the number of iterations to run in the test,
          an output operation might use it to limit the amount of output it produces.
    
      - operation : op                 (unicode)       [create,query]
          Specify the peeking operation to perform. The various operations are registered at run time and can be listed by
          querying this flag without a value. If you query it with a value then you get the detail values that peek operation
          accepts and a description of what it does. In query mode, this flag can accept a value.Flag can have multiple arguments,
          passed either as a tuple or a list.
    
      - outputFile : of                (unicode)       [create,query]
          Specify the location of a file to which the information is to be dumped. Default will return the value from the command.
          Use the special names stdoutand stderrto redirect to your command window. The special name msdevis available when
          debugging on Windows to direct your output to the debug tab in the output window of Visual Studio.
    
    
    Derived from mel command `maya.cmds.dbPeek`
    """

    pass


def testPa(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.testPa`
    """

    pass


def dagCommandWrapper(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.dagCommandWrapper`
    """

    pass


def nop(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.nop`
    """

    pass


def clearShear(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.clearShear`
    """

    pass


def evalContinue(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.evalContinue`
    """

    pass


def copyNode(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.copyNode`
    """

    pass


def polySpinEdge(*args, **kwargs):
    """
    Flags:
      - caching : cch                  (bool)          []
    
      - constructionHistory : ch       (bool)          []
    
      - frozen : fzn                   (bool)          []
    
      - name : n                       (unicode)       []
    
      - nodeState : nds                (int)           []
    
      - offset : off                   (int)           []
    
      - reverse : rev                  (bool)          []
    
    
    Derived from mel command `maya.cmds.polySpinEdge`
    """

    pass


def polyTestPop(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.polyTestPop`
    """

    pass


def retimeHelper(*args, **kwargs):
    """
    Flags:
      - deleteFrame : df               (int)           []
    
      - frame : f                      (float)         []
    
      - lockBar : lb                   (int, int)      []
    
      - locks : lk                     (int)           []
    
      - mouseOver : mo                 (bool)          []
    
      - moveFrame : mf                 (int, float)    []
    
    
    Derived from mel command `maya.cmds.retimeHelper`
    """

    pass


def extendFluid(*args, **kwargs):
    """
    Flags:
      - endD : ed                      (int)           []
    
      - endH : eh                      (int)           []
    
      - endW : ew                      (int)           []
    
      - startD : sd                    (int)           []
    
      - startH : sh                    (int)           []
    
      - startW : sw                    (int)           []
    
    
    Derived from mel command `maya.cmds.extendFluid`
    """

    pass


def adskAssetListUI(*args, **kwargs):
    """
    Flags:
      - commandSuffix : cms            (unicode)       []
    
      - materialLoaded : mld           (bool)          []
    
      - uiCommand : uiC                (unicode)       []
    
    
    Derived from mel command `maya.cmds.adskAssetListUI`
    """

    pass


def mouldMesh(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.mouldMesh`
    """

    pass


def artAttrSkinPaint(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.artAttrSkinPaint`
    """

    pass


def rampWidget(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.rampWidget`
    """

    pass


def polyToCurve(*args, **kwargs):
    """
    Flags:
      - addUnderTransform : aut        (bool)          []
    
      - caching : cch                  (bool)          []
    
      - constructionHistory : ch       (bool)          []
    
      - degree : dg                    (int)           []
    
      - form : f                       (int)           []
    
      - frozen : fzn                   (bool)          []
    
      - name : n                       (unicode)       []
    
      - nodeState : nds                (int)           []
    
      - object : o                     (bool)          []
    
    
    Derived from mel command `maya.cmds.polyToCurve`
    """

    pass


def mouldSrf(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.mouldSrf`
    """

    pass


def greaseRenderPlane(*args, **kwargs):
    """
    Flags:
      - axis : ax                      (float, float, float) []
    
      - caching : cch                  (bool)          []
    
      - constructionHistory : ch       (bool)          []
    
      - createUVs : cuv                (int)           []
    
      - frozen : fzn                   (bool)          []
    
      - height : h                     (float)         []
    
      - name : n                       (unicode)       []
    
      - nodeState : nds                (int)           []
    
      - object : o                     (bool)          []
    
      - subdivisionsHeight : sh        (int)           []
    
      - subdivisionsWidth : sw         (int)           []
    
      - subdivisionsX : sx             (int)           []
    
      - subdivisionsY : sy             (int)           []
    
      - texture : tx                   (int)           []
    
      - width : w                      (float)         []
    
    
    Derived from mel command `maya.cmds.greaseRenderPlane`
    """

    pass


def subgraph(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.subgraph`
    """

    pass


def subdToNurbs(*args, **kwargs):
    """
    Flags:
      - addUnderTransform : aut        (bool)          []
    
      - applyMatrixToResult : amr      (bool)          []
    
      - caching : cch                  (bool)          []
    
      - constructionHistory : ch       (bool)          []
    
      - frozen : fzn                   (bool)          []
    
      - name : n                       (unicode)       []
    
      - nodeState : nds                (int)           []
    
      - object : o                     (bool)          []
    
      - outputType : ot                (int)           []
    
    
    Derived from mel command `maya.cmds.subdToNurbs`
    """

    pass


def flushThumbnailCache(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.flushThumbnailCache`
    """

    pass


def mouldSubdiv(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.mouldSubdiv`
    """

    pass


def dgPerformance(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.dgPerformance`
    """

    pass


def hotkeyMapSet(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.hotkeyMapSet`
    """

    pass


def cteComposition(*args, **kwargs):
    """
    Dynamic library stub function
    
    Flags:
      - active : act                   (bool)          []
    
      - rosterNode : rtn               (bool)          []
    
    
    Derived from mel command `maya.cmds.cteComposition`
    """

    pass


def nodeGrapher(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.nodeGrapher`
    """

    pass


def webViewCmd(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.webViewCmd`
    """

    pass


def rampWidgetAttrless(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.rampWidgetAttrless`
    """

    pass


def syncSculptCache(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.syncSculptCache`
    """

    pass


def repeatLast(*args, **kwargs):
    """
    Flags:
      - addCommand : ac                (unicode)       []
    
      - addCommandLabel : acl          (unicode)       []
    
      - commandList : cl               (int)           []
    
      - commandNameList : cnl          (int)           []
    
      - historyLimit : hl              (int)           []
    
      - item : i                       (int)           []
    
      - numberOfHistoryItems : nhi     (bool)          []
    
    
    Derived from mel command `maya.cmds.repeatLast`
    """

    pass


def interactionStyle(*args, **kwargs):
    """
    Flags:
      - style : s                      (unicode)       []
    
    
    Derived from mel command `maya.cmds.interactionStyle`
    """

    pass


def agFormatIn(*args, **kwargs):
    """
    Flags:
      - file : f                       (unicode)       []
    
      - name : n                       (unicode)       []
    
    
    Derived from mel command `maya.cmds.agFormatIn`
    """

    pass


def journal(*args, **kwargs):
    """
    Flags:
      - comment : c                    (unicode)       []
    
      - flush : fl                     (bool)          []
    
      - highPrecision : hp             (bool)          []
    
      - state : st                     (bool)          []
    
    
    Derived from mel command `maya.cmds.journal`
    """

    pass


def readPDC(*args, **kwargs):
    """
    Flags:
      - file : f                       (unicode)       []
    
      - test : t                       (bool)          []
    
    
    Derived from mel command `maya.cmds.readPDC`
    """

    pass


def flagTest(*args, **kwargs):
    """
    Flags:
      - floatRange : fr                (floatRange)    []
    
      - indexRange : ir                (indexRange)    []
    
      - multiUse : mu                  (float, int, unicode) []
    
      - noReport : nr                  (bool)          []
    
      - optionalQueryArgsFlag : oqa    (float, int, unicode) []
    
      - pythonOptionalQueryArgsFlag : poq (float, int, unicode) []
    
      - pythonQueryArgsFlag : pq       (float, int, unicode) []
    
      - queryArgsFlag : qa             (float, int, unicode) []
    
      - simpleFlag : s                 (bool)          []
    
      - stringArrayFlag : saf          (string[...])   []
    
      - stringFlag : sf                (unicode)       []
    
      - timeRange : tr                 (timeRange)     []
    
      - tripleFloat : tf               (float, float, float) []
    
    
    Derived from mel command `maya.cmds.flagTest`
    """

    pass


def debug(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.debug`
    """

    pass


def myTestCmd(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.myTestCmd`
    """

    pass


def _getParserClass(strObj):
    pass


def polySetVertices(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.polySetVertices`
    """

    pass


def adskAssetList(*args, **kwargs):
    """
    Flags:
      - infoType : it                  (unicode)       []
    
    
    Derived from mel command `maya.cmds.adskAssetList`
    """

    pass


def selectKeyframe(*args, **kwargs):
    """
    Flags:
      - animation : an                 (unicode)       []
    
      - attribute : at                 (unicode)       []
    
      - controlPoints : cp             (bool)          []
    
      - float : f                      (floatRange)    []
    
      - hierarchy : hi                 (unicode)       []
    
      - includeUpperBound : iub        (bool)          []
    
      - index : index                  (indexRange)    []
    
      - selectionWindow : sel          (float, float, float, float) []
    
      - shape : s                      (bool)          []
    
      - time : t                       (timeRange)     []
    
    
    Derived from mel command `maya.cmds.selectKeyframe`
    """

    pass


def testPassContribution(*args, **kwargs):
    """
    Flags:
      - renderLayer : rl               (unicode)       []
    
      - renderPass : rp                (unicode)       []
    
    
    Derived from mel command `maya.cmds.testPassContribution`
    """

    pass


def caddyManip(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.caddyManip`
    """

    pass


def paint3d(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.paint3d`
    """

    pass


def artAttr(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.artAttr`
    """

    pass


def manipComponentPivot(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.manipComponentPivot`
    """

    pass


def debugNamespace(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.debugNamespace`
    """

    pass


def licenseCheck(*args, **kwargs):
    """
    Flags:
      - mode : m                       (unicode)       []
    
      - type : typ                     (unicode)       []
    
    
    Derived from mel command `maya.cmds.licenseCheck`
    """

    pass


def meshIntersectTest(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.meshIntersectTest`
    """

    pass


def subdDisplayMode(*args, **kwargs):
    """
    Flags:
      - hideFaceGadgets : hfg          (bool)          []
    
      - showComponentsAsNumerals : scn (bool)          []
    
      - showFaceRegions : sfr          (bool)          []
    
      - showVisualEdgeVertices : svv   (bool)          []
    
      - showVisualEdges : sve          (bool)          []
    
      - subdivEdgeMask : sem           (int)           []
    
    
    Derived from mel command `maya.cmds.subdDisplayMode`
    """

    pass


def artFluidAttr(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.artFluidAttr`
    """

    pass


def groupParts(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.groupParts`
    """

    pass


def texSculptCacheSync(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.texSculptCacheSync`
    """

    pass


def polyIterOnPoly(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.polyIterOnPoly`
    """

    pass


def objstats(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.objstats`
    """

    pass


def polySelectSp(*args, **kwargs):
    """
    Flags:
      - loop : l                       (bool)          []
    
      - ring : r                       (bool)          []
    
    
    Derived from mel command `maya.cmds.polySelectSp`
    """

    pass


def polyColorSetCmdWrapper(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.polyColorSetCmdWrapper`
    """

    pass


def dgcontrol(*args, **kwargs):
    """
    Flags:
      - iomode : iom                   (bool)          []
    
    
    Derived from mel command `maya.cmds.dgcontrol`
    """

    pass


def fontAttributes(*args, **kwargs):
    """
    Flags:
      - faceName : fc                  (unicode)       []
    
      - font : fn                      (unicode)       []
    
      - pitch : p                      (unicode)       []
    
      - size : sz                      (int)           []
    
      - style : st                     (unicode)       []
    
      - weight : wt                    (unicode)       []
    
    
    Derived from mel command `maya.cmds.fontAttributes`
    """

    pass


def dgfootprint(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.dgfootprint`
    """

    pass


def dgstats(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.dgstats`
    """

    pass


def polyWarpImage(*args, **kwargs):
    """
    Flags:
      - background : bg                (int, int, int) []
    
      - bilinear : b                   (bool)          []
    
      - fileFormat : ff                (unicode)       []
    
      - inputName : inputName          (unicode)       []
    
      - inputUvSetName : iuv           (unicode)       []
    
      - noAlpha : na                   (bool)          []
    
      - outputName : on                (unicode)       []
    
      - outputUvSetName : ouv          (unicode)       []
    
      - overwrite : o                  (bool)          []
    
      - tiled : t                      (bool)          []
    
      - xResolution : xr               (int)           []
    
      - yResolution : yr               (int)           []
    
    
    Derived from mel command `maya.cmds.polyWarpImage`
    """

    pass


def cte(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.cte`
    """

    pass


def artAttrSkinPaintCmd(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.artAttrSkinPaintCmd`
    """

    pass


def artSetPaint(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.artSetPaint`
    """

    pass


def movieCompressor(*args, **kwargs):
    """
    Flags:
      - hardwareOptions : ho           (bool)          []
    
      - softwareOptions : so           (bool)          []
    
    
    Derived from mel command `maya.cmds.movieCompressor`
    """

    pass


def cteBakeClips(*args, **kwargs):
    """
    Dynamic library stub function
    
    Flags:
      - allClips : ac                  (bool)          []
    
      - bakeToClip : btc               (unicode)       []
    
      - clipIds : ids                  (unicode)       []
    
      - removeRosterItem : rri         (bool)          []
    
      - roster : ros                   (unicode)       []
    
      - rosterItem : ri                (int)           []
    
      - sampleBy : sb                  (time)          []
    
    
    Derived from mel command `maya.cmds.cteBakeClips`
    """

    pass


def dgdebug(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.dgdebug`
    """

    pass


def debugVar(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.debugVar`
    """

    pass


def nurbsCurveRebuildPref(*args, **kwargs):
    """
    Flags:
      - degree : d                     (int)           []
    
      - endKnots : end                 (int)           []
    
      - fitRebuild : fr                (int)           []
    
      - keepControlPoints : kcp        (bool)          []
    
      - keepEndPoints : kep            (bool)          []
    
      - keepRange : kr                 (int)           []
    
      - keepTangents : kt              (bool)          []
    
      - rebuildType : rt               (int)           []
    
      - smartSurfaceCurve : scr        (bool)          []
    
      - spans : s                      (int)           []
    
      - tolerance : tol                (float)         []
    
    
    Derived from mel command `maya.cmds.nurbsCurveRebuildPref`
    """

    pass


def hotkeyEditor(*args, **kwargs):
    """
    Dynamic library stub function
    
    
    Derived from mel command `maya.cmds.hotkeyEditor`
    """

    pass


def greasePencilHelper(*args, **kwargs):
    """
    Flags:
      - brushType : bt                 (int)           []
    
      - contextName : cn               (unicode)       []
    
      - setColor : sc                  (float, float, float) []
    
      - updateVar : uv                 (unicode, float) []
    
    
    Derived from mel command `maya.cmds.greasePencilHelper`
    """

    pass


def blend(*args, **kwargs):
    """
    Flags:
      - autoDirection : ad             (bool)          []
    
      - caching : cch                  (bool)          []
    
      - constructionHistory : ch       (bool)          []
    
      - crvsInFirstRail : cfr          (int)           []
    
      - flipLeft : fl                  (bool)          []
    
      - flipRight : fr                 (bool)          []
    
      - frozen : fzn                   (bool)          []
    
      - leftParameter : lp             (float)         []
    
      - multipleKnots : mk             (bool)          []
    
      - name : n                       (unicode)       []
    
      - nodeState : nds                (int)           []
    
      - object : o                     (bool)          []
    
      - polygon : po                   (int)           []
    
      - positionTolerance : pt         (float)         []
    
      - rightParameter : rp            (float)         []
    
      - tangentTolerance : tt          (float)         []
    
    
    Derived from mel command `maya.cmds.blend`
    """

    pass


def dagObjectHit(*args, **kwargs):
    """
    Flags:
      - cache : ch                     (bool)          []
    
      - menu : mn                      (unicode)       []
    
      - multiple : m                   (bool)          []
    
      - targetSize : ts                (int)           []
    
    
    Derived from mel command `maya.cmds.dagObjectHit`
    """

    pass


def imageWindowEditor(*args, **kwargs):
    """
    Flags:
      - autoResize : ar                (bool)          []
    
      - changeCommand : cc             (unicode, unicode, unicode, unicode) []
    
      - clear : cl                     (int, int, float, float, float) []
    
      - control : ctl                  (bool)          []
    
      - defineTemplate : dt            (unicode)       []
    
      - displayImage : di              (int)           []
    
      - displayStyle : dst             (unicode)       []
    
      - docTag : dtg                   (unicode)       []
    
      - doubleBuffer : dbf             (bool)          []
    
      - drawAxis : da                  (bool)          []
    
      - exists : ex                    (bool)          []
    
      - filter : f                     (unicode)       []
    
      - forceMainConnection : fmc      (unicode)       []
    
      - frameImage : fi                (bool)          []
    
      - frameRegion : fr               (bool)          []
    
      - highlightConnection : hlc      (unicode)       []
    
      - loadImage : li                 (unicode)       []
    
      - lockMainConnection : lck       (bool)          []
    
      - mainListConnection : mlc       (unicode)       []
    
      - marquee : mq                   (float, float, float, float) []
    
      - nbImages : nim                 (bool)          []
    
      - panel : pnl                    (unicode)       []
    
      - parent : p                     (unicode)       []
    
      - realSize : rs                  (bool)          []
    
      - refresh : ref                  (bool)          []
    
      - removeAllImages : ra           (bool)          []
    
      - removeImage : ri               (bool)          []
    
      - saveImage : si                 (bool)          []
    
      - scaleBlue : sb                 (float)         []
    
      - scaleGreen : sg                (float)         []
    
      - scaleRed : sr                  (float)         []
    
      - selectionConnection : slc      (unicode)       []
    
      - showRegion : srg               (int, int)      []
    
      - singleBuffer : sbf             (bool)          []
    
      - stateString : sts              (bool)          []
    
      - toggle : tgl                   (bool)          []
    
      - unParent : up                  (bool)          []
    
      - unlockMainConnection : ulk     (bool)          []
    
      - updateMainConnection : upd     (bool)          []
    
      - useTemplate : ut               (unicode)       []
    
      - writeImage : wi                (unicode)       []
    
    
    Derived from mel command `maya.cmds.imageWindowEditor`
    """

    pass


def customerInvolvementProgram(*args, **kwargs):
    """
    Flags:
      - desktopAnalytics : da          (bool)          []
    
    
    Derived from mel command `maya.cmds.customerInvolvementProgram`
    """

    pass


def directConnectPath(*args, **kwargs):
    """
    Derived from mel command `maya.cmds.directConnectPath`
    """

    pass


def cteClipEdit(*args, **kwargs):
    """
    Dynamic library stub function
    
    Flags:
      - absoluteDuration : ad          (int)           []
    
      - absoluteEnd : ae               (int)           []
    
      - absoluteLoopedEnd : ale        (int)           []
    
      - absoluteLoopedStart : als      (int)           []
    
      - absoluteStart : absoluteStart  (int)           []
    
      - clipId : id                    (int)           []
    
      - copyClip : ccl                 (bool)          []
    
      - crossfadeMode : cfm            (int)           []
    
      - crossfadeValue : cfv           (float)         []
    
      - duplicateClip : dcl            (bool)          []
    
      - extend : ex                    (bool)          []
    
      - extendParent : exp             (bool)          []
    
      - extendParents : xcp            (bool)          []
    
      - ghost : gh                     (bool)          []
    
      - loopEnd : le                   (float)         []
    
      - loopStart : ls                 (float)         []
    
      - moveClip : mcl                 (time)          []
    
      - parentClipId : pid             (int)           []
    
      - pasteClip : pcl                (time)          []
    
      - razorClip : rcl                (bool)          []
    
      - removeClip : rmc               (bool)          []
    
      - ripple : rpl                   (bool)          []
    
      - rosterId : rid                 (int)           []
    
      - scaleEnd : sce                 (time)          []
    
      - scalePivot : scp               (time)          []
    
      - scaleStart : scs               (time)          []
    
      - speedRamping : src             (int)           []
    
      - trimEnd : tre                  (time)          []
    
      - trimStart : trs                (time)          []
    
      - truncatedDuration : td         (int)           []
    
      - truncatedEnd : te              (int)           []
    
      - truncatedLoopedEnd : tle       (int)           []
    
      - truncatedLoopedStart : tls     (int)           []
    
      - truncatedStart : ts            (int)           []
    
    
    Derived from mel command `maya.cmds.cteClipEdit`
    """

    pass


def vnnCompound(*args, **kwargs):
    """
    Flags:
      - addNode : an                   (unicode)       []
    
      - addStatePortUI : spu           (bool)          []
    
      - create : c                     (unicode)       []
    
      - deletePort : dp                (unicode)       []
    
      - moveNodeIn : mi                (unicode)       []
    
      - renameNode : rn                (unicode, unicode) []
    
      - renamePort : rp                (unicode, unicode) []
    
      - saveAs : sa                    (unicode)       []
    
      - specializedTypeName : stn      (bool)          []
    
    
    Derived from mel command `maya.cmds.vnnCompound`
    """

    pass


def dynTestData(*args, **kwargs):
    """
    Flags:
      - arrayAttrs : aa                (bool)          []
    
      - verbose : v                    (bool)          []
    
    
    Derived from mel command `maya.cmds.dynTestData`
    """

    pass



