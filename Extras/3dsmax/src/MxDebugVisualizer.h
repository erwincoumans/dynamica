#ifndef MX_DEBUGVISUALIZER_H
#define MX_DEBUGVISUALIZER_H

#include "PxPlugin.h"

/**
	This class is used for visualizing the DebugRenderable data from the
	PhysX SDK on top of the viewports in 3ds max.
*/
class MxDebugVisualizer : public ViewportDisplayCallback
{
public:
	MxDebugVisualizer();
	virtual ~MxDebugVisualizer();

	virtual void Display(TimeValue t, ViewExp *vpt, int flags);
	virtual void GetViewportRect(TimeValue t, ViewExp *vpt, Rect *rect);
	virtual BOOL Foreground() {return FALSE;}
	void Cleanup();
private:
	Point3* m_points;
	NxU32 m_numElements;
};

#endif //MX_DEBUGVISUALIZER_H
