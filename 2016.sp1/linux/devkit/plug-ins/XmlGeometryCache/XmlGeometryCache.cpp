//-
// ==========================================================================
// Copyright 2009 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

//
// Description:
//	This plug-in provides an example of the use of MPxCacheFormat.
//
//	In this example, the cache files are written in xml format.
//

#include <string>
#include <stack>
#include <fstream>
#include <sstream>

#include <assert.h>
#include <stdlib.h>
#include <maya/MFnPlugin.h>
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MPxCacheFormat.h>
#include <maya/MTime.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MDoubleArray.h>
#include <maya/MFloatArray.h>
#include <maya/MIntArray.h>
#include <maya/MVectorArray.h>

using namespace std;

class XmlCacheFormat : public MPxCacheFormat
{
public:
	XmlCacheFormat();
	~XmlCacheFormat();

	static void*	creator();
	static void		setPluginName( const MString& name );
	static MString	translatorName();

	MStatus			isValid();

    MStatus			open( const MString& fileName, FileAccessMode mode );
    void			close();

    MStatus			readHeader();
    MStatus			writeHeader( const MString& version, MTime& startTime, MTime& endTime );

    void			beginWriteChunk();
    void			endWriteChunk();
    MStatus			beginReadChunk();
    void			endReadChunk();

    MStatus			writeTime( MTime& time );
    MStatus			readTime( MTime& time );
    MStatus			findTime( MTime& time, MTime& foundTime );
    MStatus			readNextTime( MTime& foundTime );

    unsigned		readArraySize();

    MStatus			writeDoubleArray( const MDoubleArray& );
    MStatus			readDoubleArray( MDoubleArray&, unsigned size );
    MStatus			writeFloatArray( const MFloatArray& );
    MStatus			readFloatArray( MFloatArray&, unsigned size );
    MStatus			writeDoubleVectorArray( const MVectorArray& array );
    MStatus			readDoubleVectorArray( MVectorArray&, unsigned arraySize );
    MStatus			writeFloatVectorArray( const MFloatVectorArray& array );
    MStatus			readFloatVectorArray( MFloatVectorArray& array, unsigned arraySize );

    MStatus			writeChannelName( const MString& name );
    MStatus			findChannelName( const MString& name );
    MStatus			readChannelName( MString& name );

    MStatus			writeInt32( int );
    int				readInt32();
    MStatus			rewind();
 
	MString			extension();

protected:

	static MString	comment( const MString& text );
	static MString	quote( const MString& text );

	static MString	fExtension;
	static MString	fCacheFormatName;

private:
	void			startXmlBlock( string& t );
	void			endXmlBlock();
	void			writeXmlTagValue( string& tag, string value );
	void			writeXmlTagValue( string& tag, int value );
	bool			readXmlTagValue( string tag, MStringArray& value );
	bool			readXmlTagValueInChunk( string tag, MStringArray& values );
	void			readXmlTag( string& value );
	bool			findXmlStartTag( string& tag );
	bool			findXmlStartTagInChunk( string& tag );
	bool			findXmlEndTag( string& tag );
	void			writeXmlValue( string& value );
	void			writeXmlValue( double value );
	void			writeXmlValue( float value );
	void			writeXmlValue( int value );
		
	MString			fFileName;
	fstream			fFile;
	stack<string>	fXmlStack;
	FileAccessMode	fMode;

};

MString XmlCacheFormat::fExtension = "mc";			// For files on disk
MString XmlCacheFormat::fCacheFormatName = "xml";	// For presentation in GUI

inline MString XmlCacheFormat::translatorName()
{
	return fCacheFormatName;
}

void* XmlCacheFormat::creator()
{
	return new XmlCacheFormat();
}


// ****************************************
//
//  Helper functions

string XMLSTARTTAG( string x ) 
{
	return ( "<" + x + ">" );
}

string XMLENDTAG( string x ) 
{
	return ( "</" + x + ">" );
}

string cacheTag("awGeoCache");
string startTimeTag("startTime");
string endTimeTag("endTime");
string versionTag("version");
string timeTag("time");	
string sizeTag("size");
string intTag("integer32");
string doubleArrayTag("doubleArray");
string floatArrayTag("floatArray");
string doubleVectorArrayTag("doubleVectorArray");
string floatVectorArrayTag("floatVectorArray");
string channelTag("channel");	
string chunkTag("chunk");			

XmlCacheFormat::XmlCacheFormat()
{
}

XmlCacheFormat::~XmlCacheFormat()
{
    close();	
}

MStatus
XmlCacheFormat::open( const MString& fileName, FileAccessMode mode )
{
	bool rtn = true;

	assert((fileName.length() > 0));

	fFileName = fileName;
	
	if( mode == kWrite ) {
		fFile.open(fFileName.asChar(), ios::out);
	}
	else if (mode == kReadWrite) {
		fFile.open(fFileName.asChar(), ios::app);
	} else {
		fFile.open(fFileName.asChar(), ios::in);
	}

	if (!fFile.is_open()) {
		rtn = false;
	} else {
		if (mode == kRead) {
			rtn = readHeader();
		}
	}

	return rtn ? MS::kSuccess : MS::kFailure ;
}


MStatus
XmlCacheFormat::isValid()
{
	bool rtn = fFile.is_open();
	return rtn ? MS::kSuccess : MS::kFailure ;
}

MStatus
XmlCacheFormat::readHeader() 
{
	bool rtn = false;
	if (kWrite != fMode) {
		if( fFile.is_open() ) {
			string tag;
			readXmlTag( tag );

			if( tag == XMLSTARTTAG(cacheTag) ) 
			{
				MStringArray value;
				readXmlTagValue( versionTag, value );

				readXmlTagValue( startTimeTag, value );

				readXmlTagValue( endTimeTag, value );

				readXmlTag( tag ); 	// Should be header close tag, check
				if( tag != XMLENDTAG(cacheTag) )
				{
					// TODO - handle this error case
				}

				rtn = true;
			}
		}
	}

	return rtn ? MS::kSuccess : MS::kFailure ;
}

MStatus
XmlCacheFormat::rewind()
{
   	if( fFile.is_open() ) 
   	{
		close();
        	if(!open(fFileName, kRead ))
		{
			return MS::kFailure;
		}
		return MS::kSuccess;
	}
	else
	{
		return MS::kFailure;
	}
}

void XmlCacheFormat::close() 
{
    if( fFile.is_open() ) {
	fFile.close();
    }
}

MStatus XmlCacheFormat::writeInt32( int i ) 
{
        writeXmlTagValue(intTag, i);
	return MS::kSuccess;
}

int
XmlCacheFormat::readInt32()
{
	MStringArray value;
	readXmlTagValue(intTag, value);
	return atoi( value[0].asChar() );
}


MStatus
XmlCacheFormat::writeHeader( const MString& version, MTime& startTime, MTime& endTime )
{
	stringstream oss;

	startXmlBlock(cacheTag);
	string v(version.asChar());
	writeXmlTagValue(versionTag, v);

	oss << startTime;
	writeXmlTagValue(startTimeTag, oss.str() );

	oss.str("");
	oss.clear();
	oss << endTime;
	writeXmlTagValue(endTimeTag, oss.str() );

	endXmlBlock();
	return MS::kSuccess;
}


MStatus
XmlCacheFormat::readTime( MTime& time )
{
	MStringArray timeValue;
	readXmlTagValue(timeTag, timeValue);
	time.setValue( strtod( timeValue[0].asChar(), NULL ) );

	return MS::kSuccess;
}

MStatus
XmlCacheFormat::writeTime( MTime& time )
{
	stringstream oss;
	oss << time;
	writeXmlTagValue(timeTag, oss.str() );
	return MS::kSuccess;
}


MStatus
XmlCacheFormat::findChannelName( const MString& name )
//	
//  Given that the right time has already been found, find the name
//  of the channel we're trying to read.
//
{
	MStringArray value;
	while (readXmlTagValueInChunk(channelTag, value)) 
	{
		if( value.length() == 1 && value[0] == name )
		{
			return MS::kSuccess;
		}
	}

	return MS::kFailure;
}

MStatus
XmlCacheFormat::readChannelName( MString& name )
//	
//  Given that the right time has already been found, find the name
//  of the channel we're trying to read.
//
//  If no more channels exist, return false. Some callers rely on this false return 
//  value to terminate scanning for channels, thus it's not an error condition.
//
//  TODO: the current implementation assumes there are numChannels of data stored
//  for every time.  This assumption needs to be removed.
//
{
	MStringArray value;
	readXmlTagValueInChunk(channelTag, value);

	name = value[0];

	return name.length() == 0 ? MS::kFailure : MS::kSuccess;
}


MStatus
XmlCacheFormat::readNextTime( MTime& foundTime )
//	
// Read the next time based on the current read position.
//
{	
	MTime readAwTime(0.0, MTime::k6000FPS);
	bool ret = readTime(readAwTime);
	foundTime = readAwTime;

	return ret ? MS::kSuccess : MS::kFailure ;
}

MStatus
XmlCacheFormat::findTime( MTime& time, MTime& foundTime )
//	
// Find the biggest cached time, which is smaller or equal to 
// seekTime and return foundTime
//
{
	MTime timeTolerance(0.0, MTime::k6000FPS);
	MTime seekTime(time);
	MTime preTime( seekTime - timeTolerance );
	MTime postTime( seekTime + timeTolerance );

	bool fileRewound = false;
	while (1)
	{
		bool timeTagFound = beginReadChunk();
		if ( ! timeTagFound && !fileRewound )
		{
			if(!rewind())
			{
				return MS::kFailure;
			}
			fileRewound = true;
			timeTagFound = beginReadChunk();
		}
		if ( timeTagFound ) 
		{
			MTime rTime(0.0, MTime::k6000FPS);
			readTime(rTime);

			if(rTime >= preTime && rTime <= postTime)
			{
				foundTime = rTime;
				return MS::kSuccess;
			}
			if(rTime > postTime ) 
			{
				if (!fileRewound) 
				{
					if(!rewind())
					{
						return MS::kFailure;
					}
					fileRewound = true;
				} 
				else 
				{
					// Time could not be found
					//
					return MS::kFailure;
				}
			} 
			else 
			{
				fileRewound = true;
			}
		    endReadChunk();
		} 
		else
		{
		    // Not a valid disk cache file.
			break;
		}
	}

	return MS::kFailure;
}

MStatus
XmlCacheFormat::writeChannelName( const MString& name )
{
	string chan = name.asChar();
	writeXmlTagValue( channelTag, chan );
	return MS::kSuccess;
}


void XmlCacheFormat::beginWriteChunk()
{
	startXmlBlock(chunkTag);
}


void XmlCacheFormat::endWriteChunk()
{
	endXmlBlock();
}

MStatus XmlCacheFormat::beginReadChunk()
{
	return findXmlStartTag(chunkTag) ? MS::kSuccess : MS::kFailure;
}

void XmlCacheFormat::endReadChunk()
{
	findXmlEndTag(chunkTag);
}

MStatus
XmlCacheFormat::writeDoubleArray( const MDoubleArray& array )
{
	int size = array.length();
	assert(size != 0);

	writeXmlTagValue(sizeTag,size);

	startXmlBlock( doubleArrayTag );
	for(int i = 0; i < size; i++)
	{
	    writeXmlValue(array[i]);
	}
	endXmlBlock();
	return MS::kSuccess;
}

MStatus
XmlCacheFormat::writeFloatArray( const MFloatArray& array )
{
	int size = array.length();
	assert(size != 0);

	writeXmlTagValue(sizeTag,size);

	startXmlBlock( floatArrayTag );
	for(int i = 0; i < size; i++)
	{
	    writeXmlValue(array[i]);
	}
	endXmlBlock();
	return MS::kSuccess;
}

MStatus
XmlCacheFormat::writeDoubleVectorArray( const MVectorArray& array )
{
	int size = array.length();
	assert(size != 0);

	writeXmlTagValue(sizeTag,size);

	startXmlBlock( doubleVectorArrayTag );
	for(int i = 0; i < size; i++)
	{
	    writeXmlValue(array[i][0]);
	    writeXmlValue(array[i][1]);
	    writeXmlValue(array[i][2]);
	    string endl("\n");
	    writeXmlValue(endl);
	}
	endXmlBlock();
	return MS::kSuccess;
}


MStatus 
XmlCacheFormat::writeFloatVectorArray( const MFloatVectorArray& array )
{
	int size = array.length();
	assert(size != 0);

	writeXmlTagValue(sizeTag,size);

	startXmlBlock( floatVectorArrayTag );
	for(int i = 0; i < size; i++)
	{
	    writeXmlValue(array[i][0]);
	    writeXmlValue(array[i][1]);
	    writeXmlValue(array[i][2]);
	    string endl("\n");
	    writeXmlValue(endl);
	}
	endXmlBlock();
	return MS::kSuccess;
}


unsigned
XmlCacheFormat::readArraySize()
{
	MStringArray value;

	if (readXmlTagValue(sizeTag, value))
	{
		unsigned readSize = (unsigned) atoi( value[0].asChar() );
		return readSize;
	}

	return 0;
}

MStatus
XmlCacheFormat::readDoubleArray( MDoubleArray& array, unsigned arraySize )
{
	MStringArray value;
	readXmlTagValue(doubleArrayTag, value);

	assert( value.length() == arraySize );
	array.setLength( arraySize );
	for ( unsigned int i = 0; i < value.length(); i++ )
	{
		array[i] = strtod( value[i].asChar(), NULL );
	}
	
	return MS::kSuccess;
}

MStatus
XmlCacheFormat::readFloatArray( MFloatArray& array, unsigned arraySize )
{
	MStringArray value;
	readXmlTagValue(floatArrayTag, value);

	assert( value.length() == arraySize );
	array.setLength( arraySize );
	for ( unsigned int i = 0; i < value.length(); i++ )
	{
		array[i] = (float)strtod( value[i].asChar(), NULL );
	}
	
	return MS::kSuccess;
}

MStatus
XmlCacheFormat::readDoubleVectorArray( MVectorArray& array, unsigned arraySize )
{
	MStringArray value;
	if( !readXmlTagValue(doubleVectorArrayTag, value) )
	{
		return MS::kFailure;
	}

	assert( value.length() == arraySize * 3 );
	array.setLength( arraySize );
	for (unsigned i = 0; i < arraySize; i++ )
	{
		double v[3];
		v[0] = strtod( value[i*3].asChar(), NULL );
		v[1] = strtod( value[i*3+1].asChar(), NULL );
		v[2] = strtod( value[i*3+2].asChar(), NULL );

		array.set( v, i );
	}
	
	return MS::kSuccess;
}

MStatus 
XmlCacheFormat::readFloatVectorArray( MFloatVectorArray& array, unsigned arraySize )
{
	MStringArray value;
	readXmlTagValue(floatVectorArrayTag, value);

	assert( value.length() == arraySize * 3 );

	array.clear();
	array.setLength( arraySize );
	for (unsigned i = 0; i < arraySize; i++ )
	{
		float v[3];
		v[0] = (float)strtod( value[i*3].asChar(), NULL );
		v[1] = (float)strtod( value[i*3+1].asChar(), NULL );
		v[2] = (float)strtod( value[i*3+2].asChar(), NULL );

		array.set(v,i);
	}

	return MS::kSuccess;
}

MString
XmlCacheFormat::extension()
{
	return fExtension;
}

// ****************************************
//
//  Helper functions
//

void XmlCacheFormat::startXmlBlock( string& t )
{
	fXmlStack.push(t);
	fFile << "<" << t << ">\n";
}


void XmlCacheFormat::endXmlBlock()
{
	string block = fXmlStack.top();
	fFile << "</" << block << ">\n";
	fXmlStack.pop();
}

void XmlCacheFormat::writeXmlTagValue( string& tag, string value )
{
	for (unsigned int i = 0; i < fXmlStack.size(); i++ )
		fFile << "\t";

	fFile << "<" << tag << "> ";	// Extra space is important for reading
	fFile << value;
	fFile << " </" << tag << ">\n";	// Extra space is important for reading
}

void XmlCacheFormat::writeXmlTagValue( string& tag, int value )
{
	for (unsigned int i = 0; i < fXmlStack.size(); i++ )
		fFile << "\t";

	fFile << "<" << tag << "> ";	// Extra space is important for reading
	ostringstream oss;
	oss << value;
	fFile << oss.str();
	fFile << " </" << tag << ">\n";	// Extra space is important for reading
}

bool XmlCacheFormat::readXmlTagValue( string tag, MStringArray& values )
{
	string endTag = XMLENDTAG(tag);
	bool status = true;

	values.clear();

	// Yes this could be much much smarter
	if( findXmlStartTag(tag) )
	{
		string token;
		fFile >> token;
		while ( !fFile.eof() && token != endTag )
		{
			values.append( token.data() );
			fFile >> token;
		} 
	}
	else
	{
		status = false;
	}

	return status;
}


bool XmlCacheFormat::readXmlTagValueInChunk( string tag, MStringArray& values )
{
	string endTag = XMLENDTAG(tag);
	bool status = true;

	values.clear();

    // Find the tag in the currently read chunk.
	if( findXmlStartTagInChunk(tag) )
	{
		string token;
		fFile >> token;
        // Look for the values within the bounds of the tag.
		while ( !fFile.eof() && token != endTag )
		{
			values.append( token.data() );
			fFile >> token;
		} 
	}
	else
	{
		status = false;
	}

	return status;
}

void XmlCacheFormat::readXmlTag( string& value )
{
	value.clear();
        fFile >> value;
}

bool XmlCacheFormat::findXmlStartTag( string& tag )
{
	string tagRead;
	string tagExpected = XMLSTARTTAG(tag);

	fFile >> tagRead;
	// Keep looking all the way to EOF
	while(!fFile.eof() && tagRead != tagExpected) 
	{
		fFile >> tagRead;
	}
	if( tagRead != tagExpected )
	{
	}

	return ( tagRead == tagExpected );
}

bool XmlCacheFormat::findXmlStartTagInChunk( string& tag )
//
// Look for the given tag within the currently read chunk.
//
{
	string tagRead;
	string tagExpected = XMLSTARTTAG(tag);
	string tagEndChunk("</"+chunkTag+">");

    fFile >> tagRead;

    // Keep looking all the way to EOF
    while((!fFile.eof()) && (tagRead != tagExpected) && (tagRead != tagEndChunk)) 
    {
		fFile >> tagRead;
    }
	if( (tagRead != tagExpected) && (tagRead != tagEndChunk) )
	{
	}

	return ( tagRead == tagExpected );
}

bool XmlCacheFormat::findXmlEndTag(string& tag)
{
	string tagRead;
	string tagExpected("</"+tag+">");

	fFile >> tagRead;
	if( tagRead != tagExpected )
	{
	}

	return ( tagRead == tagExpected );
}

void XmlCacheFormat::writeXmlValue( string& value )
{
	fFile << value << " ";
}

void XmlCacheFormat::writeXmlValue( double value )
{
	fFile << value << " ";
}

void XmlCacheFormat::writeXmlValue( float value )
{
	fFile << value << " ";
}

void XmlCacheFormat::writeXmlValue( int value )
{
	fFile << value << " ";
}



// ****************************************

MStatus initializePlugin( MObject obj )
{
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "1.0", "Any" );

	plugin.registerCacheFormat(
		XmlCacheFormat::translatorName(),
		XmlCacheFormat::creator
	);

	return MS::kSuccess;
}


MStatus uninitializePlugin( MObject obj )
{
	MFnPlugin plugin( obj );

	plugin.deregisterCacheFormat( XmlCacheFormat::translatorName() );

	return MS::kSuccess;
}
