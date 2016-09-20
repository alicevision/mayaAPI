#-
# ==========================================================================
# Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
#
# Use of this software is subject to the terms of the Autodesk
# license agreement provided at the time of installation or download,
# or which otherwise accompanies this software in either electronic
# or hard copy form.
# ==========================================================================
#+

import os
import os.path
import getopt
import sys
import xml.dom.minidom
import string
import re
import array

"""

This example shows how to parse data from a Maya cache file, up to version 2.0.
It produces an ascii dump of the first few elements of data for every channel 
at every time.  It parses the XML file in addition to the cache data files and 
handles caches that are one file per frame as well as one file. It reads 32 bit  
chunks (mcc) as well as 64 bit chunks (mcx).
To use: 

python cacheFileExample.py -f mayaCacheFile.xml


Overview of Maya Caches:
========================

Conceptually, a Maya cache consists of 1 or more channels of data.  
Each channel has a number of properties, such as:

- start/end time 
- data type of the channel (eg. "DoubleVectorArray" to represents a point array)
- interpretation (eg. "positions" the vector array represents position data, as opposed to per vertex normals, for example)
- sampling type (eg. "regular" or "irregular")
- sampling rate (meaningful only if sampling type is "regular")

Each channel has a number of data points in time, not necessarily regularly spaced, 
and not necessarily co-incident in time with data in other channels.  
At the highest level, a Maya cache is simply made up of channels and their data in time.

On disk, the Maya cache is made up of a XML description file, and 1 or more data files.  
The description file provides a high level overview of what the cache contains, 
such as the cache type (one file, or one file per frame), channel names, interpretation, etc.  
The data files contain the actual data for the channels.  
In the case of one file per frame, a naming convention is used so the cache can check its 
available data at runtime.

Here is a visualization of the data format of the OneFile case:

//  |---CACH (Group)	// Header
//  |     |---VRSN		// Version Number (char*)
//  |     |---STIM		// Start Time of the Cache File (int)
//  |     |---ETIM		// End Time of the Cache File (int)
//  |
//  |---MYCH (Group)	// 1st Time 
//  |     |---TIME		// Time (int)
//  |     |---CHNM		// 1st Channel Name (char*)
//  |     |---SIZE		// 1st Channel Size
//  |     |---DVCA		// 1st Channel Data (Double Vector Array)
//  |     |---CHNM		// n-th Channel Name
//  |     |---SIZE		// n-th Channel Size
//  |     |---DVCA		// n-th Channel Data (Double Vector Array)
//  |     |..
//  |
//  |---MYCH (Group)	// 2nd Time 
//  |     |---TIME		// Time
//  |     |---CHNM		// 1st Channel Name
//  |     |---SIZE		// 1st Channel Size
//  |     |---DVCA		// 1st Channel Data (Double Vector Array)
//  |     |---CHNM		// n-th Channel Name
//  |     |---SIZE		// n-th Channel Size
//  |     |---DVCA		// n-th Channel Data (Double Vector Array)
//  |     |..
//  |
//  |---..
//	|
//

In a multiple file caches, the only difference is that after the 
header "CACH" group, there is only one MYCH group and there is no 
TIME chunk.	In the case of one file per frame, the time is part of 
the file name - allowing Maya to scan at run time to see what data 
is actually available, and it allows users to move data in time by 
manipulating the file name.  

!Note that it's not necessary to have data for every channel at every time.  

"""
def fileFormatError():
    print "Error: unable to read cache format\n";
    sys.exit(2)

def readTag(fd,tagFOR,blockTag):
    count = 4
    blockTag.append(fd.read(4))	
    # Padding
    if tagFOR == "FOR8":
        fd.read(4)
        count = 8
        
    return count
   
def readInt(fd,needSwap,tagFOR):
    intArray = array.array('l') 
    size = 1
    if tagFOR == "FOR8":
        size = 2
    intArray.fromfile(fd,size)
    if needSwap:    
        intArray.byteswap()
        
    return intArray[size - 1]        

class CacheChannel:
    m_channelName = ""
    m_channelType = ""                
    m_channelInterp = ""
    m_sampleType = ""
    m_sampleRate = 0
    m_startTime = 0
    m_endTime = 0      
    def __init__(self,channelName,channelType,interpretation,samplingType,samplingRate,startTime,endTime):
        self.m_channelName = channelName
        self.m_channelType = channelType                
        self.m_channelInterp = interpretation
        self.m_sampleType = samplingType
        self.m_sampleRate = samplingRate
        self.m_startTime = startTime
        self.m_endTime = endTime     
        print "Channel Name =%s,type=%s,interp=%s,sampleType=%s,rate=%d,start=%d,end=%d\n"%(channelName,channelType,interpretation,samplingType,samplingRate,startTime,endTime)
        
class CacheFile:
    m_baseFileName = ""
    m_directory = ""    
    m_cacheType = ""
    m_cacheStartTime = 0
    m_cacheEndTime = 0
    m_timePerFrame = 0
    m_version = 0.0
    m_channels = []
    m_printChunkInfo = False
    m_tagSize = 4
    m_blockTypeSize = 4
    m_glCount = 0
    m_numFramesToPrint = 2
    
    ########################################################################
    #   Description:
    #       Class constructor - tries to figure out full path to cache
    #       xml description file before calling parseDescriptionFile()
    #
    def __init__(self,fileName):
        # fileName can be the full path to the .xml description file,
        # or just the filename of the .xml file, with or without extension
        # if it is in the current directory
        dir = os.path.dirname(fileName)
        fullPath = ""
        if dir == "":
            currDir = os.getcwd() 
            fullPath = os.path.join(currDir,fileName)
            if not os.path.exists(fullPath):
                fileName = fileName + '.xml';
                fullPath = os.path.join(currDir,fileName)
                if not os.path.exists(fullPath):
                    print "Sorry, can't find the file %s to be opened\n" % fullPath
                    sys.exit(2)                    
                
        else:
            fullPath = fileName                
                
        self.m_baseFileName = os.path.basename(fileName).split('.')[0]        
        self.m_directory = os.path.dirname(fullPath)
        self.parseDescriptionFile(fullPath)

    ########################################################################
    # Description:
    #   Given the full path to the xml cache description file, this 
    #   method parses its contents and sets the relevant member variables
    #
    def parseDescriptionFile(self,fullPath):          
        dom = xml.dom.minidom.parse(fullPath)
        root = dom.getElementsByTagName("Autodesk_Cache_File")
        allNodes = root[0].childNodes
        for node in allNodes:
            if node.nodeName == "cacheType":
                self.m_cacheType = node.attributes.item(0).nodeValue                
            if node.nodeName == "time":
                timeRange = node.attributes.item(0).nodeValue.split('-')
                self.m_cacheStartTime = int(timeRange[0])
                self.m_cacheEndTime = int(timeRange[1])
            if node.nodeName == "cacheTimePerFrame":
                self.m_timePerFrame = int(node.attributes.item(0).nodeValue)
            if node.nodeName == "cacheVersion":
                self.m_version = float(node.attributes.item(0).nodeValue)                
            if node.nodeName == "Channels":
                self.parseChannels(node.childNodes)
                
    ########################################################################
    # Description:
    #   helper method to extract channel information
    #            
    def parseChannels(self,channels):                         
        for channel in channels:
            if re.compile("channel").match(channel.nodeName) != None :
                channelName = ""
                channelType = ""                
                channelInterp = ""
                sampleType = ""
                sampleRate = 0
                startTime = 0
                endTime = 0                                               
                
                for index in range(0,channel.attributes.length):
                    attrName = channel.attributes.item(index).nodeName                                                            
                    if attrName == "ChannelName":                        
                        channelName = channel.attributes.item(index).nodeValue                        
                    if attrName == "ChannelInterpretation":
                        channelInterp = channel.attributes.item(index).nodeValue
                    if attrName == "EndTime":
                        endTime = int(channel.attributes.item(index).nodeValue)
                    if attrName == "StartTime":
                        startTime = int(channel.attributes.item(index).nodeValue)
                    if attrName == "SamplingRate":
                        sampleRate = int(channel.attributes.item(index).nodeValue)
                    if attrName == "SamplingType":
                        sampleType = channel.attributes.item(index).nodeValue
                    if attrName == "ChannelType":
                        channelType = channel.attributes.item(index).nodeValue
                    
                channelObj = CacheChannel(channelName,channelType,channelInterp,sampleType,sampleRate,startTime,endTime)
                self.m_channels.append(channelObj)
                    
    def printIntData( self, count, data, desc ):
        if self.m_printChunkInfo: 
            print "%0.2d  %d %s" % (count, data, desc )
   
    def printBlockSize( self, count, blockSize ):
        if self.m_printChunkInfo: 
            print "%0.2d  %d Bytes" % (count, blockSize)

    def printTag( self, count, tag ):
        if self.m_printChunkInfo: 
            print "%0.2d  %s" % (count, tag)

    def printString( self, text ):
        if self.m_printChunkInfo: 
            print text

    def printTime( self, count, time ):
        if self.m_printChunkInfo: 
            print "%0.2d  %d sec" % (count, time)
                    
    def readHeader( self, fd,needSwap,tagFOR ):
        self.printString( "\nHEADER" ) 
        #CACH
        blockTag = fd.read(self.m_tagSize)
        self.m_glCount += self.m_tagSize
        self.printTag(self.m_glCount, blockTag)
        
        #VRSN (version)
        blockTagList = []
        self.m_glCount += readTag(fd, tagFOR, blockTagList)
        self.printTag( self.m_glCount, blockTagList[0] )

        blockSize = readInt(fd,needSwap,tagFOR)
        self.m_glCount += self.m_blockTypeSize
        self.printBlockSize(self.m_glCount, blockSize)

        version = fd.read(self.m_blockTypeSize)
        self.m_glCount += self.m_blockTypeSize
        self.printTag(self.m_glCount, version)
       
        #STIM (start time)
        blockTagList = []
        self.m_glCount += readTag(fd, tagFOR, blockTagList)
        self.printTag(self.m_glCount, blockTagList[0])

        blockSize = readInt(fd,needSwap,tagFOR)
        self.m_glCount += self.m_blockTypeSize
        self.printBlockSize(self.m_glCount, blockSize)
       
        startTime = readInt(fd,needSwap,tagFOR)
        self.m_glCount += self.m_blockTypeSize
        self.printTime( self.m_glCount, startTime )
       
        #ETIM (end time)
        blockTagList = []
        self.m_tagSize = readTag(fd, tagFOR, blockTagList)
        self.m_glCount += self.m_tagSize
        self.printTag(self.m_glCount, blockTagList[0])

        blockSize = readInt(fd,needSwap,tagFOR)
        self.m_glCount += self.m_blockTypeSize
        self.printBlockSize(self.m_glCount, blockSize)
       
        endtime = readInt(fd,needSwap,tagFOR)
        self.m_glCount += self.m_blockTypeSize
        self.printTime( self.m_glCount, endtime )

    def readData(self, fd, bytesRead, dataBlockSize, needSwap, tagFOR ):
        # print "Data found at time %f seconds:\n"%(time/6000.0)            
        while bytesRead < dataBlockSize:
                        
            self.printString( "\nDATA" ) 
                        
            #channel name is next.
            #the tag for this must be CHNM
            blockTagList = []
            bytesRead += readTag(fd, tagFOR, blockTagList)
            self.m_glCount += bytesRead			
            self.printTag(self.m_glCount, blockTagList[0])
         
            chnmTag = blockTagList[0]
            if chnmTag != "CHNM":
                fileFormatError()
                  
            #Next comes a 32/64 bit that tells us how long the 
            #channel name is
            chnmSize = readInt(fd,needSwap,tagFOR)
            bytesRead += self.m_blockTypeSize
            self.m_glCount += bytesRead			
            self.printBlockSize(self.m_glCount, chnmSize)
                           
            #The string is padded out to 32 bit boundaries,
            #so we may need to read more than chnmSize
            mask = 3
            if tagFOR == "FOR8":
                mask = 7
            chnmSizeToRead = (chnmSize + mask) & (~mask)            
            channelName = fd.read(chnmSize)
            paddingSize = chnmSizeToRead-chnmSize
            if paddingSize > 0:
                fd.read(paddingSize)
            bytesRead += chnmSizeToRead
            self.m_glCount += bytesRead			
            self.printTag(self.m_glCount, channelName)
         
            #Next is the SIZE field, which tells us the length 
            #of the data array
            blockTagList = []
            bytesRead += readTag(fd, tagFOR, blockTagList)
            self.m_glCount += bytesRead			
            self.printTag(self.m_glCount, blockTagList[0])

            sizeTag = blockTagList[0]
            if sizeTag != "SIZE":
                fileFormatError()
                  
            blockSize = readInt(fd,needSwap,tagFOR)
            bytesRead += self.m_blockTypeSize
            self.m_glCount += bytesRead
            self.printBlockSize(self.m_glCount, blockSize)
         
            #finally the actual size of the array:
            arrayLength = readInt(fd,needSwap,"")
            # Padding for FOR8
            if tagFOR == "FOR8":
                readInt(fd,needSwap,"")
            bytesRead += self.m_blockTypeSize
            self.m_glCount += bytesRead			
            self.printIntData( self.m_glCount, arrayLength, "arrayLength" )
         
            #data format tag:
            blockTagList = []
            bytesRead += readTag(fd, tagFOR, blockTagList)
            self.m_glCount += bytesRead			
            self.printTag(self.m_glCount, blockTagList[0])

            dataFormatTag = blockTagList[0]
         
            #buffer length - how many bytes is the actual data
            bufferLength = readInt(fd,needSwap,tagFOR)
            bytesRead += self.m_blockTypeSize
            self.m_glCount += bytesRead
            self.printIntData( self.m_glCount, bufferLength, "bufferLength" )
                  
            numPointsToPrint = 5
            if dataFormatTag == "FVCA":
                #FVCA == Float Vector Array
                if bufferLength != arrayLength*3*4:
                    fileFormatError()
                floatArray = array.array('f')    
                floatArray.fromfile(fd,arrayLength*3)
                bytesRead += arrayLength*3*4
                self.m_glCount += bytesRead				
                if needSwap:    
                    floatArray.byteswap()
                if numPointsToPrint > arrayLength:
                    numPointsToPrint = arrayLength                
                print "Channelname = %s,Data type float vector array,length = %d elements, First %d points:" % (channelName,arrayLength,numPointsToPrint)
                print floatArray[0:numPointsToPrint*3]
            elif dataFormatTag == "DVCA":                    
                #DVCA == Double Vector Array
                if bufferLength != arrayLength*3*8:
                    fileFormatError()
                doubleArray = array.array('d')    
                doubleArray.fromfile(fd,arrayLength*3)
                bytesRead += arrayLength*3*8
                self.m_glCount += bytesRead
                if needSwap:    
                    doubleArray.byteswap()
                if numPointsToPrint > arrayLength:
                    numPointsToPrint = arrayLength                
                print "Channelname = %s,Data type double vector array,length = %d elements, First %d points:" % (channelName,arrayLength,numPointsToPrint)
                print doubleArray[0:numPointsToPrint*3]
            elif dataFormatTag == "DBLA":
                #DBLA == Double Array
                print ""
                if bufferLength != arrayLength*8:
                    fileFormatError()
                doubleArray = array.array('d')    
                doubleArray.fromfile(fd,arrayLength)
                bytesRead += arrayLength*8
                self.m_glCount += bytesRead				
                if needSwap:    
                    doubleArray.byteswap()
                if numPointsToPrint > arrayLength:
                    numPointsToPrint = arrayLength                
                print "Channelname = %s,Data type double array,length = %d elements, First %d points:" % (channelName,arrayLength,numPointsToPrint)
                print doubleArray[0:numPointsToPrint]
            elif dataFormatTag == "FBCA":
                #FBCA == Float Array
                print ""
                if bufferLength != arrayLength*4:
                    fileFormatError()
                doubleArray = array.array('f')    
                doubleArray.fromfile(fd,arrayLength)
                bytesRead += arrayLength*4
                self.m_glCount += bytesRead				
                if needSwap:    
                    doubleArray.byteswap()
                if numPointsToPrint > arrayLength:
                    numPointsToPrint = arrayLength                
                print "Channelname = %s,Data type float array,length = %d elements, First %d points:" % (channelName,arrayLength,numPointsToPrint)
                print doubleArray[0:numPointsToPrint]
            else:
                fileFormatError()   
            
            #Padding                
            sizeToRead = (bufferLength + mask) & (~mask)            
            paddingSize = sizeToRead-bufferLength
            if paddingSize > 0:
                fd.read(paddingSize)
            bytesRead += paddingSize
            self.m_glCount += bytesRead		
            print "\n"


    ########################################################################
    # Description:
    #   method to parse and display the contents of the data file, for the
    #   One large file case ("OneFile")             
    def parseDataOneFile(self):
        dataFilePath = os.path.join(cacheFile.m_directory,cacheFile.m_baseFileName)
        dataFilePath = dataFilePath + ".mc"
        self.m_glCount = 0
        self.m_tagSize = 4
        self.m_blockTypeSize = 4
       
        if not os.path.exists(dataFilePath):
            print "Error: unable to open cache data file at %s\n" % dataFilePath
            sys.exit(2)	
        fd = open(dataFilePath,"rb")
        tagFOR = fd.read(4)
        self.m_glCount += 4 
       
        # Padding
        if tagFOR == "FOR8":
            fd.read(4)
            self.m_glCount += 4 
            self.m_blockTypeSize = 8
            
        self.printTag( self.m_glCount, tagFOR)
        
        #blockTag must be FOR4/FOR8
        if tagFOR != "FOR4" and tagFOR != "FOR8":
            fileFormatError()

        platform = sys.platform
        needSwap = False
        if re.compile("win").match(platform) != None :
            needSwap = True
          
        if re.compile("linux").match(platform) != None :
            needSwap = True
          
        blockSize = readInt(fd,needSwap,tagFOR)    
        self.m_glCount += self.m_blockTypeSize
        self.printBlockSize( self.m_glCount ,blockSize )
       
        self.readHeader(fd,needSwap,tagFOR)
        self.m_glCount += blockSize
                
        frameCount = 0
        while frameCount < self.m_numFramesToPrint:
            frameCount+=1;
          
            print "\n\nREAD FRAME %d" % (frameCount)
                      
            #From now on the file is organized in blocks of time
            #Each block holds the data for all the channels at that
            #time
            tagFOR = fd.read(4)
            # Padding
            if tagFOR == "FOR8":
                fd.read(4)
            
            if tagFOR == "":
                #EOF condition...we are done
                return
                 
            if tagFOR != "FOR4" and tagFOR != "FOR8":
                fileFormatError()
                 
            self.m_glCount += self.m_blockTypeSize
            self.printTag( self.m_glCount, tagFOR)	
            
            dataBlockSize = readInt(fd,needSwap,tagFOR)
            self.m_glCount += self.m_blockTypeSize
            self.printBlockSize(self.m_glCount, dataBlockSize)

            self.printString( "\nFRAMEINFO" ) 
              
            bytesRead = 0
            blockTagList = []
            bytesRead += readTag(fd, "", blockTagList)
            self.m_glCount += bytesRead
            self.printTag(self.m_glCount, blockTagList[0])
              
            mychTag = blockTagList[0]
            if mychTag != "MYCH":
                fileFormatError()
              
            blockTagList = []
            bytesRead += readTag(fd, tagFOR, blockTagList)
            self.m_glCount += bytesRead
            self.printTag(self.m_glCount, blockTagList[0])
              
            if blockTagList[0] != "TIME":
                fileFormatError()
              
            blockSize = readInt(fd,needSwap,tagFOR)
            bytesRead += self.m_blockTypeSize
            self.m_glCount += bytesRead
            self.printBlockSize(self.m_glCount, blockSize)
          
            #Next 32/64 bit int is the time itself, in ticks
            #1 tick = 1/6000 of a second
            time = readInt(fd,needSwap,tagFOR)
            bytesRead += self.m_blockTypeSize
            self.m_glCount += bytesRead		
            self.printTime( self.m_glCount, time )

            print "--------------------------------------------------------------\n"      
            print "Data found at time %f seconds:\n"%(time)                    
          
            self.readData( fd, bytesRead, dataBlockSize, needSwap, tagFOR )
            
    ########################################################################
    # Description:
    #   method to parse and display the contents of the data file, for the
    #   file per frame case ("OneFilePerFrame")             
    def parseDataFilePerFrame(self):    
        
        allFilesInDir = os.listdir(self.m_directory) 
        matcher = re.compile(self.m_baseFileName)
        dataFiles = []
        for afile in allFilesInDir:
            if os.path.splitext(afile)[1] == ".mc" and matcher.match(afile) != None:            
                dataFiles.append(afile)

        for dataFile in dataFiles:
            self.m_glCount = 0
            self.m_tagSize = 4
            self.m_blockTypeSize = 4
            fileName = os.path.split(dataFile)[1]
            baseName = os.path.splitext(fileName)[0]
            
            frameAndTickNumberStr = baseName.split("Frame")[1]
            frameAndTickNumber = frameAndTickNumberStr.split("Tick")
            frameNumber = int(frameAndTickNumber[0])
            tickNumber = 0
            if len(frameAndTickNumber) > 1:
                tickNumber = int(frameAndTickNumber[1])
                            
            timeInTicks = frameNumber*cacheFile.m_timePerFrame + tickNumber
            print "--------------------------------------------------------------\n"      
            print "Data found at time %f seconds:\n"%(timeInTicks/6000.0)        
            
            dataFile = self.m_directory + "/" + dataFile
            print self.m_directory
            print dataFile
            fd = open(dataFile,"rb")
            tagFOR = fd.read(4)
            self.m_glCount += 4 
           
            # Padding
            if tagFOR == "FOR8":
                fd.read(4)
                self.m_glCount += 4 
                self.m_blockTypeSize = 8
            
            #blockTag must be FOR4/FOR8
            if tagFOR != "FOR4" and tagFOR != "FOR8":
                fileFormatError()
                        
            platform = sys.platform
            needSwap = False
            if re.compile("win").match(platform) != None :
                needSwap = True
                
            if re.compile("linux").match(platform) != None :
                needSwap = True
                
            #offset = readInt(fd,needSwap)
            blockSize = readInt(fd,needSwap,tagFOR)    
            self.m_glCount += self.m_blockTypeSize
            self.printBlockSize( self.m_glCount ,blockSize )
            
            self.readHeader(fd,needSwap,tagFOR)
            self.m_glCount += blockSize
                        
            tagFOR = fd.read(4)
            # Padding
            if tagFOR == "FOR8":
                fd.read(4)
                             
            if tagFOR != "FOR4" and tagFOR != "FOR8":
                fileFormatError()

            self.m_glCount += self.m_blockTypeSize
            self.printTag( self.m_glCount, tagFOR)	
            
            dataBlockSize = readInt(fd,needSwap,tagFOR)
            self.m_glCount += self.m_blockTypeSize
            self.printBlockSize(self.m_glCount, dataBlockSize)
            
            bytesRead = 0
            
            blockTagList = []
            bytesRead += readTag(fd, "", blockTagList)
            self.m_glCount += bytesRead
            self.printTag(self.m_glCount, blockTagList[0])
              
            mychTag = blockTagList[0]
            if mychTag != "MYCH":
                fileFormatError()
                
            self.readData( fd, bytesRead, dataBlockSize, needSwap, tagFOR )

def usage():
    print "Use -f to indicate the cache description file (.xml) you wish to parse\n"

try:
    (opts, args) = getopt.getopt(sys.argv[1:], "f:")
except getopt.error:
    # print help information and exit:
    usage()
    sys.exit(2)

if len(opts) == 0:
    usage()
    sys.exit(2)
        
fileName = ""
for o,a in opts:
    if o == "-f":
        fileName = a    

cacheFile = CacheFile(fileName)

if cacheFile.m_version > 2.0:
    print "Error: this script can only parse cache files of version 2 or lower\n"
    sys.exit(2)

print "*******************************************************************************\n"        
print "Maya Cache version %f, Format = %s\n"%(cacheFile.m_version,cacheFile.m_cacheType)
print "The cache was originally created at %d FPS\n"%(6000.0/cacheFile.m_timePerFrame)
print "Cache has %d channels, starting at time %f seconds and ending at %f seconds\n"%(len(cacheFile.m_channels),cacheFile.m_cacheStartTime/6000.0,cacheFile.m_cacheEndTime/6000.0)
for channel in cacheFile.m_channels:        
    print   "Channelname =%s, type=%s, interpretation =%s, sampling Type = %s\n"% (channel.m_channelName,channel.m_channelType,channel.m_channelInterp,channel.m_sampleType)
    print   "sample rate (for regular sample type only) = %f FPS\n"%(6000.0/channel.m_sampleRate)
    print   "startTime=%f seconds, endTime=%f seconds\n"%(channel.m_startTime/6000.0,channel.m_endTime/6000.0)
print "*******************************************************************************\n"

if cacheFile.m_cacheType == "OneFilePerFrame":
    cacheFile.parseDataFilePerFrame()
elif cacheFile.m_cacheType == "OneFile":
    cacheFile.parseDataOneFile()
else:
    print "unknown cache type!\n"
    