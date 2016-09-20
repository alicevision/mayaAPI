/* Select Closest Point Locator Plugin
 *
 * Demo plugin to demonstrate the MPxLocator custom selection point feature
 *
 * Custom selection allows the user to specify the selection point
 * for a custom locator
 *
 * For a given cursor ray, the user can specify where in the locator's local space
 * that cursor ray intersects the locator, so Maya can make better 
 * decisions about if a certain custom locator object is selected.  This is
 * particularly important if multiple custom locator objects are hit by
 * the cursor ray.
 *
 * This plugin implements the custom selection using the closestPoint and
 * useClosestPointForSelection members of the MPxLocator class.
 *
 */
#ifndef _SELECTCLOSESTPOINTLOCATOR_PLUGIN
#define _SELECTCLOSESTPOINTLOCATOR_PLUGIN

#include <maya/MStatus.h>
#include <maya/MPlug.h>
#include <maya/MPxLocatorNode.h>

class selectClosestPointLocator : public MPxLocatorNode
{

public:
        // Default constructor & destructor
        selectClosestPointLocator();
        virtual ~selectClosestPointLocator();

        // Creator and initializer
        static void* creator();

        static MStatus initialize();

        static MTypeId d_id;

		virtual bool useClosestPointForSelection() const { return true; };
		virtual MPoint closestPoint(MPoint cursorRayPoint, MVector cursorRayDir) const;

        // Class attributes
		static MObject aPlaneSizeAttr;
		static MObject aNumDivsAttr;

        virtual void  draw( M3dView & view,  const MDagPath & path,
                                M3dView::DisplayStyle style, M3dView::DisplayStatus );
};

#endif

