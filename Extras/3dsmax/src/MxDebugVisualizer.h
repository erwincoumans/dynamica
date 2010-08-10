/* Copyright (c) 2008 NVIDIA CORPORATION

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//For feedback and latest version see http://dynamica.googlecode.com

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
