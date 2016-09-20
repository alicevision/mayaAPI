//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+


// Example Plugin: colorTransformData.cpp
//
// This plugin is an example of a file translator that extracts a scene's color management information.
//

#include <maya/MFileObject.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnPlugin.h>
#include <maya/MItDependencyNodes.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MColorManagementUtilities.h>
#include <maya/MString.h>

#include <maya/MFStream.h>

class colorTransformDataTranslator : public MPxFileTranslator
{
public:
	bool		haveReadMethod() const;
	bool		haveWriteMethod() const;
	MString		defaultExtension() const;

	MStatus		writer(
					const MFileObject& file,
					const MString& options,
					FileAccessMode mode
				);

	static void*	creator();
	static void		setPluginName(const MString& name);
	static MString	translatorName();

protected:
    const MString getInputColorSpace(const MObject& object);
    void writeColorSpaceForNodes(fstream& f);
    void writeColorTransformData(fstream& f);
    void writeOutputTransformId(fstream& f);

	static MString	fExtension;
	static MString	fFileVersion;
	static MString	fPluginName;
	static MString	fTranslatorName;
};

MString colorTransformDataTranslator::fFileVersion = "1.0";
MString colorTransformDataTranslator::fExtension = "ctd";
MString colorTransformDataTranslator::fPluginName = "";
MString colorTransformDataTranslator::fTranslatorName = "Maya Color Management Data";

//*************************************************************************************************

inline MString colorTransformDataTranslator::defaultExtension() const
{	return fExtension;		}


inline bool colorTransformDataTranslator::haveReadMethod() const
{	return false;			}


inline bool colorTransformDataTranslator::haveWriteMethod() const
{	return true;			}


inline void colorTransformDataTranslator::setPluginName(const MString& name)
{	fPluginName = name;		}


inline MString colorTransformDataTranslator::translatorName()
{	return fTranslatorName;	}


void* colorTransformDataTranslator::creator()
{
	return new colorTransformDataTranslator();
}

//
// Maya calls this method to have the translator write out a file.
//
MStatus colorTransformDataTranslator::writer(
		const MFileObject& file,
		const MString& /* options */,
		MPxFileTranslator::FileAccessMode mode
)
{
	//
	// For simplicity, we only do full saves/exports.
	//
	if ((mode != kSaveAccessMode) && (mode != kExportAccessMode))
	   	return MS::kNotImplemented;

	//
	// Let's see if we can open the output file.
	//
	fstream	output(file.fullName().asChar(), ios::out | ios::trunc);

	if (!output.good()) return MS::kNotFound;

    writeColorSpaceForNodes(output);
    writeOutputTransformId(output);
    writeColorTransformData(output);

	output.close();

	return MS::kSuccess;
}

//
// Utility method to retrieve from a node its color space attribute.
//
const MString colorTransformDataTranslator::getInputColorSpace(const MObject& object)
{
    MString inputColorSpace;

    if(!object.isNull() && 
       ( (object.apiType()==MFn::kFileTexture) || (object.apiType()==MFn::kImagePlane) ) )
    {
        MStatus	status;
        MFnDependencyNode texNode(object, &status);
        if(status)
        {
            static const char* const colorSpaceStr = "colorSpace";

            MPlug plug = texNode.findPlug(colorSpaceStr, &status);
            if (status && !plug.isNull())
            {
                plug.getValue(inputColorSpace);
            }
        }
    }
    return inputColorSpace;
}

void colorTransformDataTranslator::writeColorSpaceForNodes(fstream& f)
{
	MItDependencyNodes	nodeIter;

    f << "====================== Nodes with color space attribute =======================\n";

	for (; !nodeIter.isDone(); nodeIter.next())
	{
		MObject				node = nodeIter.item();

        if(!node.isNull() && 
           ( (node.apiType()==MFn::kFileTexture) || (node.apiType()==MFn::kImagePlane) ) )
        {
          MString inputColorSpace = getInputColorSpace(node);

          MString transformId;
          
          f << "Found node with colorspace " << inputColorSpace.asUTF8();
          if(MColorManagementUtilities::getColorTransformCacheIdForInputSpace(
                 inputColorSpace, transformId))
          {
              f << ", its corresponding transform id: " << transformId.asUTF8() << std::endl;
          }
          else
          {
              f << ", no corresponding transform id found.\n";
          }
        }

    }
}

void colorTransformDataTranslator::writeColorTransformData(fstream& f)
{
    f << "============================ Color Transform Data =============================\n";

    if(MColorManagementUtilities::isColorManagementAvailable())
    {
      MColorManagementUtilities::MColorTransformData colorTransformData;

      f << "Data block size: " << colorTransformData.getSize() << std::endl;
      f.write(static_cast<const char*>(colorTransformData.getData()), colorTransformData.getSize());
    }
    else 
    {
      f << "Color management functionality is not available\n";
    }
    f << "\n===============================================================================\n";
}  

void colorTransformDataTranslator::writeOutputTransformId(fstream& f)
{
    f << "============================ Output Transform Id =============================\n";

    MString transformId;
    if(MColorManagementUtilities::getColorTransformCacheIdForOutputTransform(transformId))
    {
        f << "Output transform id: " << transformId.asUTF8() << std::endl;
    }
    else
    {
        f << "No output transform id found.\n";
    }
}

//*************************************************************************************************

MStatus initializePlugin(MObject obj)
{
	MFnPlugin plugin(obj, PLUGIN_COMPANY, "1.0", "Any");

	colorTransformDataTranslator::setPluginName(plugin.name());

	plugin.registerFileTranslator(
		colorTransformDataTranslator::translatorName(),
		NULL,
		colorTransformDataTranslator::creator,
		NULL,
		NULL,
		false
	);

	return MS::kSuccess;
}


MStatus uninitializePlugin(MObject obj)
{
	MFnPlugin plugin( obj );

	plugin.deregisterFileTranslator(colorTransformDataTranslator::translatorName());

	return MS::kSuccess;
}
