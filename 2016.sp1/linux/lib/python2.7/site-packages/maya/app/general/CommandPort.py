"""
Implementation of the commandPort command.

Input to a commandPort is handled by a subclass of TcommandHandler.
Customization of commandPort behaviour could be achieved by implementing
a custom subclass, and adding it to commandHandlersMap, with a string key
which will be passed to openCommandPort as the 'lang' parameter.
"""
import maya
maya.utils.loadStringResourcesForModule(__name__)

from errno import EINTR
import operator
import os
import select 
import socket
import SocketServer
import StringIO
import threading
import Queue
import time
import cPickle as PICKLE
import types
import sys
from maya import utils, cmds, mel, OpenMaya

if cmds.about(windows=True):
    import ServerRegistryMMap

# map of command ports portName->TcommandServer
__commandPorts = {}
# map of language -> command handler class
commandHandlersMap = {}
# Transfer string encoding
transformEncoding = 'utf8'
# Query Maya encoding
mayaEncoding = cmds.about(codeset=True)

class TcommandHandler(SocketServer.StreamRequestHandler):
    """
    StreamRequestHandler base class for command ports, 
    handle() is called when client connects.  Subclasses must implement _languageExecute.
    """
    # the response terminator
    resp_term = '\n\x00'

    def postSecurityWarning(self):
        # Posts the security warning dialog, sets the dialog 
        # result accordingly
        message = maya.stringTable['y_CommandPort.kSecurityWarningMessage' ]
        try:
            if len(self.data) > 300:
                data = self.data[0:300] + '...'
            else:
                data = self.data
            msg = message % (self.server.portName, data)
        except UnicodeDecodeError:
            msg = message % (self.server.portName, repr(self.data))
        allowButton = maya.stringTable['y_CommandPort.kAllow' ]
        denyButton = maya.stringTable['y_CommandPort.kDeny' ]
        allowAllButton = maya.stringTable['y_CommandPort.kAllowAll' ]
        title = maya.stringTable['y_CommandPort.kSecurityWarningTitle' ]
        res = cmds.confirmDialog(title = title,
                                message = msg,
                                button = (allowButton, denyButton, allowAllButton),
                                defaultButton = allowButton,
                                cancelButton = denyButton )
        if res == denyButton:
            self.dialog_result = False
        elif res == allowAllButton:
            self.dialog_result = True
        else:
            self.dialog_result = None

    def recieveData(self):
        # Called by handle() to accumulate one incoming message.
        # Returns the recieved message, None if the request should be terminated
        # Raises socket.timeout if no data is read

        nextdata = self.request.recv(self.server.bufferSize)
        if nextdata is None:
            return None
        data = nextdata.rstrip('\x00')
        if len(data) == 0:
            return None
        oldtimeout = self.request.gettimeout()
        self.request.settimeout(1.5)
        while len(nextdata) >= self.server.bufferSize:
            try:
                nextdata = self.request.recv(self.server.bufferSize)
                data += nextdata
            except socket.timeout:
                # no more data, go with what we have
                break
        self.request.settimeout(oldtimeout)

        # First, decode received utf8 string
        try :
            data = data.strip().decode(transformEncoding)
        except :
            warnMsg = maya.stringTable['y_CommandPort.kInvalidUTF8' ]
            sys.stderr.write(warnMsg + "\n")
            return None
        # Second, encode it to the local encoding
        try :
            data = data.encode(mayaEncoding)
        except :
            warnMsg = maya.stringTable['y_CommandPort.kEncodeToNativeFailed' ]
            warnMsg = warnMsg % mayaEncoding
            sys.stderr.write(warnMsg + "\n")
            return None

        return data

    def handle(self):
        # Called to handle each connection request

        try:
            # if we are echoing output, we hold on to the first request
            # and poll the socket and for pending command messages.
            # Otherwise, we just block on the socket.
            if self.server.echoOutput:
                self.request.settimeout(1.5)
            while not self.server.die:
                # check for pending command messages
                if self.server.echoOutput:
                    while not self.server.commandMessageQueue.empty():
                        self.wfile.write(self.server.commandMessageQueue.get() + self.resp_term)
                # set self.data to be the incoming message
                try:
                    self.data = self.recieveData()
                except socket.timeout:
                    continue
                if self.data is None: 
                    break
                # check if we need to display the security warning
                # posting the dialog also has to be done in the gui thread
                if self.server.securityWarning:
                    utils.executeInMainThreadWithResult(self.postSecurityWarning)
                    if self.dialog_result is False:
                        self.wfile.write(maya.stringTable['y_CommandPort.kExecutionDeniedByMaya' ] + self.resp_term)
                        return
                    elif self.dialog_result is True:
                        self.server.securityWarning = False

                # execute the message
                response = utils.executeInMainThreadWithResult(self._languageExecute)

                # write the command responses to the client
                self.wfile.write(response)
        except socket.error:
            # a socket exception is a normal way to terminate the connection
            pass


class TMELCommandHandler(TcommandHandler):
    """
    The StreamRequestHandler for deferred MEL execution
    """
    def _languageExecute(self):
        writer = StringIO.StringIO()
        # if there is a prefix, add it to the start of each line
        #
        if len(self.server.prefix) > 0:
            newLines = []
            for line in self.data.split('\n'):
                newLines.append(self.server.prefix + ' "' + cmds.encodeString(line) + '";')
            self.data = '\n'.join(newLines)
        if not self.server.sendResults:
            try:
                mel.eval(self.data)
                writer.write( self.resp_term )
            except RuntimeError,ex:
                writer.write(unicode(ex) + self.resp_term)
        elif self.server.returnNbCommands:
            # send back number of commands eval'd instead of results

            try:
                mel.eval(self.data)
                writer.write( unicode(self.data.count('\n') + 1) + self.resp_term )
            except RuntimeError,ex:
                writer.write( unicode(ex) + self.resp_term )
        else:
            # eval each line and send back the results
            for line in self.data.split('\n'):
                outp = None
                try:
                    outp = mel.eval(line)
                except RuntimeError,ex:
                    outp = unicode(ex)
                else:
                    # if the result is a list, intersperse with tabs
                    if outp is not None and not isinstance(outp, types.StringTypes):
                        if operator.isSequenceType(outp):
                            tempList = []
                            for item in outp :
                                # If this item is not string type, like "int",
                                # We only need to use "unicode()"
                                if not isinstance(item, types.StringTypes):
                                    tempList.append(unicode(item))
                                # If this item is string type, when using "unicode()"
                                # we need to specify the source encoding or it will fail
                                elif isinstance(item, str):
                                    tempList.append(unicode(item, mayaEncoding))
                                else :
                                    # if it is already unicode
                                    tempList.append( item )
                            outp = '\t'.join( tempList )
                        else :
                            # like "int", "float".. type
                            try :
                                outp = unicode(outp)
                            except :
                                outp = None
                    # if the result is a string
                    elif isinstance(outp, str):
                        outp = unicode(outp, mayaEncoding)
                if outp is None:
                    outp = ""
                writer.write(outp.strip() + self.resp_term)
        try:
            return writer.getvalue().encode(transformEncoding)
        except:
            return self.resp_term

class TpythonCommandHandler(TcommandHandler):
    """
    The StreamRequestHandler instance for deferred python execution
    """
    def _languageExecute(self):
        lines = self.data.splitlines()
        # if there is a prefix, add it to the start of each line
        #
        if len(self.server.prefix) > 0 :
            newLines = []
            for _line in lines :
                newLines.append( self.server.prefix + ' "' + cmds.encodeString(_line) + ' "' )
            lines = newLines
            self.data = '\n'.join( newLines )

        outp = None
        if len(lines) == 1:
            try:
                # first try evaluating it as an expression so we can get the result
                outp = eval(self.data)
            except Exception:
                pass
        if outp is None:
            # more than one line, or failed to eval
            # exec everything instead
            try:
                exec self.data
            except Exception,ex:
                outp = ex

        if self.server.returnNbCommands:
            outp = len(lines)
        if not self.server.sendResults:
            outp = ""

        # If it is a local string, we need to decode it to unicode first
        if isinstance(outp, str) :
            outp = unicode(outp, mayaEncoding)
        # If it is a list, try to deocode all local strings inside it to unicode
        if type(outp) == type([]):
            listSize = len(outp)
            for i in range(0, listSize):
                if isinstance(outp[i], str):
                    outp[i] = unicode(outp[i], mayaEncoding)

        if self.server.pickleOutput and self.server.sendResults:
            try :
                # Pickle form, type of "outp" is "str"
                outp = unicode(PICKLE.dumps(outp))
            except PICKLE.PickleError,ex:
                outp = unicode(ex)
        elif not isinstance(outp, unicode):
            outp = unicode(outp)

        # Convert Maya encoding to utf8
        outp = outp.encode(transformEncoding)
        return (outp + self.resp_term)

def commandOutputCallback(message, msgType, qu):
    # Command callback function
    # When new output comes in, we add the output 
    # to the message queue, which the request handler will drain
    #    
    # message - the message string
    # msgType - message type enum
    # qu      - user data, which is the message queue
    qu.put(message)

class TcommandServer(threading.Thread):
    """
    Server for one command port
    
    socketServer - StreamServer instance to serve on endpoint
    """
    def __init__(self, socketServer):
        threading.Thread.__init__(self,None)
        self.daemon = True
        self.__servObj = socketServer
        self.__die = False

    def run(self):
        r""" called by Thread.start """
        self.__servObj.die = False
        if self.__servObj.echoOutput:
            self.__servObj.commandMessageQueue = Queue.Queue()
            self.callbackId = OpenMaya.MCommandMessage.addCommandOutputCallback(commandOutputCallback, self.__servObj.commandMessageQueue)

        while not self.__die:
            try:
                self.__servObj.handle_request()
            except select.error,ex:
                # EINTR error can be ignored
                if ex[0] == EINTR:
                    continue
                raise

    def __del__(self):
        if self.__servObj.echoOutput:
            OpenMaya.MCommandMessage.removeCallback(self.callbackId)
        self.__servObj.cleanup()

    def shutdown(self):
        """ tell the server to shutdown """
        self.__die = True
        try:
            # create a socket, connect to our listening socket and poke it so that it dies
            self.__servObj.die = True
            s = socket.socket(self.__servObj.socket.family, self.__servObj.socket.type)
            s.connect(self.__servObj.endpoint)
            s.send('\x00')
            s.close()
        except (socket.gaierror,socket.error):
            # several erros could happen here, 
            # most are ok as we are closing the scoket anyway
            pass

# Add default command handlers
commandHandlersMap['mel'] = TMELCommandHandler
commandHandlersMap['python'] = TpythonCommandHandler

def openCommandPort(portName, lang, prefix, sendResults, returnNbCommands, echoOutput, bufferSize, securityWarning, pickleOutput = False):
    """
    Open a command port with the given name
    Can be INET (host:port) or Unix local format (/tmp/endpoint)
    
    On Windows, the Unix-style format will create a named binary file-mapping
    object which contains a mapping of paths to ports.
    On Mac and Linux a Unix-domain socket will be created.
    
    Environment variable MAYA_IP_TYPE can be used to override the default 
    address family by setting it to either 'ipv4' or 'ipv6'.  The default is 
    ipv4.

    portname         - name of the port, used to refer to the server, follows 
                       above format
    lang             - must be a key in commandHandlersMap eg 'mel' or 'python'
    prefix           - string prefix to prepend to every command
    sendResults      - True means send results of commands back to client
    returnNbCommands - True means return the number of commands executed
    echoOutput       - True means echo command output to the port
    bufferSize       - byte size of the socket buffer
    securityWarning  - True means issue a security warning
    pickleOutput     - True means the return string for python command would be
                       pickled, default is false
    Returns error string on failure, None on success
    """
    # make sure port name is encodable as str
    try:
        portName = str(portName)
    except UnicodeEncodeError:
        msg = maya.stringTable['y_CommandPort.kCoundNotCreateCommandPortNotAscii' ]
        return(msg % portName)
        
    nameLen = len(portName)
    
    # parse out an ipv6 network resource identifier format '[2001:db8::1428:57ab]:443' -> '2001:db8::1428:57ab:443'
    if nameLen > 0 and portName[0] == '[' and portName.rfind(']') > 0:
        portNameNormal = portName[1:].replace(']', '')
    else:
        portNameNormal = portName

    portInvalidNameMsg = maya.stringTable['y_CommandPort.kCouldNotCreateCommandPortInvalidName' ]
    colonIx = portNameNormal.rfind(':')
    if nameLen == 0 or colonIx == nameLen - 1:
        msg = portInvalidNameMsg
        return(msg % portName)

    # look up command handler for this language
    try:
        HdlrClass = commandHandlersMap[lang]
    except KeyError:
        msg = maya.stringTable['y_CommandPort.kLanguageNotSupported' ]
        return(msg % lang)

    # check if the portName is already active
    if portName in __commandPorts:
        msg = maya.stringTable['y_CommandPort.kCommandPortAlreadyActive' ]
        return(msg % portName)

    # select default address family, defaulting to ipv4 unless we set the environment
    # variable to use IPv6
    # ultimatly, the address family is determined by the portName string, which may be
    # one or the other.
    hasIPV6 = False
    addrFamily = socket.AF_INET
    host = '127.0.0.1'
    try:
        if 'ipv6' == os.environ['MAYA_IP_TYPE'].lower():
            try:
                # this call will raise if the OS doesn't support ipv6
                socket.getaddrinfo('::1',None)
                hasIPV6 = True
                host = '::1'
                addrFamily = socket.AF_INET6
            except socket.gaierror:
                # ipv6 isn't going work
                pass
    except KeyError: pass
	
    port = 0
    # parse out the endpoint
    if colonIx >= 0:
        defaulthost = host
        # INET port, don't know if it is ipv4 or ipv6 yet
        try:
            if colonIx == 0:
                # eg: ':65000'
                port = int(portNameNormal[1:])
            else:
                # eg: 'localhost:65000'
                host = portNameNormal[0:colonIx]
                port = int(portNameNormal[colonIx+1:])

            if port > 65535 or port < 1:
                msg = maya.stringTable['y_CommandPort.kCoundNotCreateCommandPortInvalidPort' ]
                return(msg % portName)

            try:
                try:
                    # check the endpoint with the current addrFamily and host
                    endpoints = socket.getaddrinfo(host,port,addrFamily)
                except socket.gaierror:
                    if defaulthost != host:
                        # maybe the hostname is wrong, try the default hostname
                        endpoints = socket.getaddrinfo(defaulthost,port,addrFamily)
                    else:
                        raise
            except socket.gaierror:
                # try the other address family, unless there is no other
                if not hasIPV6:
                    raise
                if addrFamily is socket.AF_INET6:
                    addrFamily = socket.AF_INET
                else:
                    addrFamily = socket.AF_INET6
                endpoints = socket.getaddrinfo(host,port,addrFamily)
            if len(endpoints) == 0:
                raise ValueError
            endpoint = endpoints[0][-1]
        except (ValueError, socket.gaierror):
            msg = portInvalidNameMsg
            return(msg % portName)
        ServerClass = SocketServer.TCPServer
        ServerClass.address_family = addrFamily
        def cleanup(): pass
    else:
        # filename (no ':' in name)
        if cmds.about(windows=True):
            try:
                # We have a named mmap file wherein we keep the mappings of paths to INET ports
                endpoint = ServerRegistryMMap.registerServer(portName, addrFamily)
            except RuntimeError,ex:
                return ex.args[0]
            ServerClass = SocketServer.TCPServer
            ServerClass.address_family = addrFamily
            def cleanup(): pass
        else:
            # normal Unix domain socket
            if portNameNormal[0] == '/':
                endpoint = portNameNormal
            else:
                endpoint = '/tmp/' + portNameNormal
            if os.path.exists(endpoint):
                # file is already there, delete it now so that we can create it again
                try:
                    os.remove(endpoint)
                except OSError:
                    # probably won't be able to create the socket
                    # but try anyway
                    pass
            ServerClass = SocketServer.UnixStreamServer
            def cleanup():
                try:
                    os.remove(endpoint)
                except OSError: pass
    
    try:
        # create the server and attach preferences variables
        socketServer =                  ServerClass(endpoint, HdlrClass)
        socketServer.prefix =           prefix
        socketServer.sendResults =      sendResults
        socketServer.returnNbCommands = returnNbCommands and sendResults
        socketServer.echoOutput =       echoOutput and sendResults
        socketServer.bufferSize =       max(16, int(bufferSize))
        socketServer.securityWarning =  securityWarning
        socketServer.portName =         portNameNormal
        socketServer.endpoint =         endpoint              
        socketServer.cleanup =          cleanup
        socketServer.pickleOutput =     pickleOutput

        # create the command server and start it
        cmdServer = TcommandServer(socketServer)
        __commandPorts[portName] = cmdServer
        cmdServer.start()
        time.sleep(0)
    except socket.error,ex:
        msg = maya.stringTable['y_CommandPort.kSocketErrorCreatingCommandPort' ]
        return(msg % (portName, ex.args[0]))

def closeCommandPort(portName):
    """ 
    Close the specified command port  
    Returns error string on failure, None on success
    """
    try:
        portName = str(portName)
        server = __commandPorts.pop(portName)
        server.shutdown()
        if cmds.about(windows=True):
            activeServers = ServerRegistryMMap.getInstance()
            activeServers.removeServer(portName)
    except (KeyError, UnicodeEncodeError):
        return(maya.stringTable['y_CommandPort.kNoSuchCommandPort' ] % repr(portName))

def listCommandPorts():
    """
    Returns the list of command port names, in the format used by openCommandPort
    """
    return __commandPorts.keys()
# Copyright (C) 1997-2014 Autodesk, Inc., and/or its licensors.
# All rights reserved.
#
# The coded instructions, statements, computer programs, and/or related
# material (collectively the "Data") in these files contain unpublished
# information proprietary to Autodesk, Inc. ("Autodesk") and/or its licensors,
# which is protected by U.S. and Canadian federal copyright law and by
# international treaties.
#
# The Data is provided for use exclusively by You. You have the right to use,
# modify, and incorporate this Data into other products for purposes authorized 
# by the Autodesk software license agreement, without fee.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND. AUTODESK
# DOES NOT MAKE AND HEREBY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES
# INCLUDING, BUT NOT LIMITED TO, THE WARRANTIES OF NON-INFRINGEMENT,
# MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, OR ARISING FROM A COURSE 
# OF DEALING, USAGE, OR TRADE PRACTICE. IN NO EVENT WILL AUTODESK AND/OR ITS
# LICENSORS BE LIABLE FOR ANY LOST REVENUES, DATA, OR PROFITS, OR SPECIAL,
# DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES, EVEN IF AUTODESK AND/OR ITS
# LICENSORS HAS BEEN ADVISED OF THE POSSIBILITY OR PROBABILITY OF SUCH DAMAGES.

