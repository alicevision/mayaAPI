//-
//**************************************************************************/
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk 
// license agreement provided at the time of installation or download, 
// or which otherwise accompanies this software in either electronic 
// or hard copy form.
//**************************************************************************/
//+

#include "gpuCacheUtil.h"


namespace GPUCache {

/*==============================================================================
 * CLASS InstanceMaterialLookup
 *============================================================================*/

InstanceMaterialLookup::InstanceMaterialLookup(const MDagPath& dagPath)
    : fInstObjGroupsPlug(findInstObjGroupsPlug(dagPath))
{}

InstanceMaterialLookup::~InstanceMaterialLookup()
{}

bool InstanceMaterialLookup::hasWholeObjectMaterial()
{
    // Default Viewport behavior:
    //   If instObjGroups[x] is connected, it's whole object material assignment.
    return fInstObjGroupsPlug.isSource();
}

MObject InstanceMaterialLookup::findWholeObjectShadingGroup()
{
    // Not a whole object material assignment.
    if (!hasWholeObjectMaterial()) {
        return MObject::kNullObj;
    }

    // Find the destination shading group
    return findShadingGroupByPlug(fInstObjGroupsPlug);
}

MObject InstanceMaterialLookup::findWholeObjectSurfaceMaterial()
{
    // Not a whole object material assignment.
    if (!hasWholeObjectMaterial()) {
        return MObject::kNullObj;
    }

    // Find the shading group node.
    MObject shadingGroup = findWholeObjectShadingGroup();
    if (shadingGroup.isNull()) return MObject::kNullObj;

    // Find the source surface material.
    return findSurfaceMaterialByShadingGroup(shadingGroup);
}

bool InstanceMaterialLookup::hasComponentMaterials()
{
    // Find instObjGroups[instanceNumber].objectGroup[X] plugs
    std::vector<MPlug> ogPlugs;
    findObjectGroupsPlug(fInstObjGroupsPlug, ogPlugs);

    // If any of the objectGroup[X] plug is connected, this is a 
    // per-component material assignment.
    BOOST_FOREACH (const MPlug& ogPlug, ogPlugs) {
        if (ogPlug.isSource()) {
            return true;
        }
    }
    return false;
}

bool InstanceMaterialLookup::findShadingGroups(std::vector<MObject>& shadingGroups)
{
    // Not a per-component material assignment.
    if (!hasComponentMaterials()) {
        return false;
    }

    // Find the objectGroup[X] plugs.
    std::vector<MPlug> ogPlugs;
    findObjectGroupsPlug(fInstObjGroupsPlug, ogPlugs);

    // Find the destination shading groups for each objectGroup[X]
    BOOST_FOREACH (const MPlug& ogPlug, ogPlugs) {
        shadingGroups.push_back(findShadingGroupByPlug(ogPlug));
    }

    return true;
}

bool InstanceMaterialLookup::findSurfaceMaterials(std::vector<MObject>& surfaceMaterials)
{
    // Not a per-component material assignment.
    if (!hasComponentMaterials()) {
        return false;
    }

    // Find the connected shading groups.
    std::vector<MObject> shadingGroups;
    if (!findShadingGroups(shadingGroups)) {
        return false;
    }

    // Find the source surface materials for each shading group.
    BOOST_FOREACH (const MObject& shadingGroup, shadingGroups) {
        surfaceMaterials.push_back(findSurfaceMaterialByShadingGroup(shadingGroup));
    }

    return true;
}

/*
// We will eventually support per-face material assignment. 
// This will require a change to the usage of MGeometryExtract to generate
// index groups first.
// Comment out the code for now.
std::vector<MObject> InstanceMaterialLookup::findComponents()
{
    // Find objectGroup plugs
    std::vector<MPlug> ogPlugs;
    findObjectGroupsPlug(fInstObjGroupsPlug);

    std::vector<MObject> mergedComponents;

    BOOST_FOREACH (MPlug& ogPlug, ogPlugs) {
        // Find objectGrpCompList plug
        MPlug gclPlug = ogPlug.child(0);
        MFnComponentListData compList(gclPlug.asMObject());

        if (compList.length() == 0) {
            mergedComponents.push_back(MObject::kNullObj);
            continue;
        }

        if (compList[0].hasFn(MFn::kMeshPolygonComponent)) {
            // Merge mesh face component
            MFnSingleIndexedComponent mergedComp;
            mergedComp.create(MFn::kMeshPolygonComponent);

            for (unsigned int i = 0; i < compList.length(); i++) {
                MFnSingleIndexedComponent comp(compList[i]);
                for (int j = 0; j < comp.elementCount(); j++) {
                    mergedComp.addElement(j);
                }
            }

            mergedComponents.push_back(mergedComp.object());
        }
        else {
            assert(0); // per-patch not supported now
            mergedComponents.push_back(MObject::kNullObj);
        }
    }

    return mergedComponents;
}
*/

MPlug InstanceMaterialLookup::findInstObjGroupsPlug(const MDagPath& dagPath)
{
    // The path must be derived from a shape.
    assert(dagPath.node().hasFn(MFn::kShape));

    MStatus status;
    MFnDagNode dagNode(dagPath, &status);
    assert(status == MS::kSuccess);

    // Find the instObjGroups array plug (instanced attribute).
    MPlug plug = dagNode.findPlug("instObjGroups", false);
    assert(!plug.isNull());

    // Select the instance number.
    plug.selectAncestorLogicalIndex(dagPath.instanceNumber());
    return plug;
}

MObject InstanceMaterialLookup::findShadingGroupByPlug(const MPlug& srcPlug)
{
    if (srcPlug.isNull()) return MObject::kNullObj;

    // shape.srcPlug -> shadingGroup.dagSetMember
    if (srcPlug.isSource()) {
        // List the destination plugs.
        MPlugArray plugArray;
        srcPlug.connectedTo(plugArray, false, true);
        assert(plugArray.length() == 1);

        // The destination node is the shading group.
        if (plugArray.length() > 0) {
            MObject shadingGroup = plugArray[0].node();
            assert(shadingGroup.hasFn(MFn::kShadingEngine));

            if (shadingGroup.hasFn(MFn::kShadingEngine)) {
                return shadingGroup;
            }
        }
    }
    return MObject::kNullObj;
}

MObject InstanceMaterialLookup::findSurfaceMaterialByShadingGroup(const MObject& shadingGroup)
{
    if (shadingGroup.isNull()) return MObject::kNullObj;
    assert(shadingGroup.hasFn(MFn::kShadingEngine));

    // Find the surfaceShader plug.
    MFnDependencyNode dgNode(shadingGroup);
    MPlug ssPlug = dgNode.findPlug("surfaceShader", false);
    assert(!ssPlug.isNull());

    // material.outColor -> shadingGroup.surfaceShader
    if (ssPlug.isDestination()) {
        MPlugArray plugArray;
        ssPlug.connectedTo(plugArray, true, false);
        assert(plugArray.length() == 1);

        // The source node is the surface material.
        if (plugArray.length() > 0) {
            MObject shader = plugArray[0].node();
            assert(!shader.isNull());

            return shader;
        }
    }
    return MObject::kNullObj;
}

void InstanceMaterialLookup::findObjectGroupsPlug(const MPlug& iogPlug, std::vector<MPlug>& ogPlugs)
{
    assert(!iogPlug.isNull());

    // 0th child is objectGroups
    MPlug ogPlug = iogPlug.child(0);
    unsigned int ogPlugCount = ogPlug.numElements();
    ogPlugs.reserve(ogPlugCount);

    for (unsigned int i = 0; i < ogPlugCount; i++) {
        // plug = instObjGroups[which].objectGroups[i]
        ogPlugs.push_back(ogPlug[i]);
    }
}


/*==============================================================================
 * CLASS ShadedModeColor
 *============================================================================*/

bool ShadedModeColor::evaluateBool(
    const MaterialProperty::Ptr& prop,
    double                       timeInSeconds
)
{
    assert(prop && prop->type() == MaterialProperty::kBool);
    if (!prop || prop->type() != MaterialProperty::kBool) {
        return false;
    }

    const MaterialNode::Ptr     srcNode = prop->srcNode();
    const MaterialProperty::Ptr srcProp = prop->srcProp();
    if (srcNode && srcProp) {
        // If there is a connection, we use the default value.
        return prop->getDefaultAsBool();
    }
    else {
        // Otherwise, we use the value in the property.
        return prop->asBool(timeInSeconds);
    }
}

float ShadedModeColor::evaluateFloat(
    const MaterialProperty::Ptr& prop,
    double                       timeInSeconds
)
{
    assert(prop && prop->type() == MaterialProperty::kFloat);
    if (!prop || prop->type() != MaterialProperty::kFloat) {
        return 0.0f;
    }

    const MaterialNode::Ptr     srcNode = prop->srcNode();
    const MaterialProperty::Ptr srcProp = prop->srcProp();
    if (srcNode && srcProp) {
        // If there is a connection, we use the default value.
        return prop->getDefaultAsFloat();
    }
    else {
        // Otherwise, we use the value in the property.
        return prop->asFloat(timeInSeconds);
    }
}

MColor ShadedModeColor::evaluateDefaultColor(
    const MaterialProperty::Ptr& prop,
    double                       timeInSeconds
)
{
    assert(prop && prop->type() == MaterialProperty::kRGB);
    if (!prop || prop->type() != MaterialProperty::kRGB) {
        return MColor::kOpaqueBlack;
    }

    // Check source connections.
    const MaterialNode::Ptr     srcNode = prop->srcNode();
    const MaterialProperty::Ptr srcProp = prop->srcProp();
    if (srcNode && srcProp) {
        // There is a source connection. Let's check if it's a texture2d node.
        const boost::shared_ptr<const Texture2d> srcTex = 
            boost::dynamic_pointer_cast<const Texture2d>(srcNode);
        if (srcTex && srcTex->OutColor == srcProp) {
            // This property has a source texture2d node and the output
            // of the texture2d node is outColor.
            // We use the Default Color as the outColor.
            return srcTex->DefaultColor->asColor(timeInSeconds);
        }
        else {
            // The source is not texture2d.outColor.
            // We use the default value instead.
            return prop->getDefaultAsColor();
        }
    }

    // No source connection. We use the value in the property directly.
    return prop->asColor(timeInSeconds);
}

MColor ShadedModeColor::evaluateColor(
    const MaterialProperty::Ptr& prop,
    double                       timeInSeconds
)
{
    assert(prop && prop->type() == MaterialProperty::kRGB);
    if (!prop || prop->type() != MaterialProperty::kRGB) {
        return MColor::kOpaqueBlack;
    }

    // Check source connections.
    const MaterialNode::Ptr     srcNode = prop->srcNode();
    const MaterialProperty::Ptr srcProp = prop->srcProp();
    if (srcNode && srcProp) {
		// If there is a connection, we use the default value.
		return prop->getDefaultAsColor();
    }
    else {
		// Otherwise, we use the value in the property.
		return prop->asColor(timeInSeconds);
    }
}


} // namespace GPUCache

