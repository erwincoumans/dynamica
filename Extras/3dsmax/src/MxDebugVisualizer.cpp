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


#include "MxUtils.h"

#include <max.h>

#include "MxPluginData.h"
#include "MxDebugVisualizer.h"

MxDebugVisualizer::MxDebugVisualizer()
{
	m_points = NULL;
	m_numElements = 0;
}

MxDebugVisualizer::~MxDebugVisualizer()
{
	Cleanup();
}

void MxDebugVisualizer::Cleanup()
{
	if(m_points)
		delete[] m_points;
	m_points = NULL;
	m_numElements = 0;
}


void DebugSetupColor(NxU32 color, float& red, float& green, float& blue)
{
	blue	= NxF32((color)&0xff)/255.0f;
	green	= NxF32((color>>8)&0xff)/255.0f;
	red		= NxF32((color>>16)&0xff)/255.0f;
}


void MxDebugVisualizer::Display(TimeValue t, ViewExp *vpt, int flags)
{
#if 0
	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return;
	const NxDebugRenderable* pDebugRenderable = scene->getDebugRenderable();
	if (!pDebugRenderable) return;
	GraphicsWindow* gw = vpt->getGW();
	if (!gw) return;


	NxDebugRenderable data = *pDebugRenderable;
	DrawLineProc lineproc(gw);

	// Render lines
	NxU32 NbLines = data.getNbLines();
	//	NxU32 NbTris = data.getNbTriangles();
	//	NxU32 NbPoints = data.getNbPoints();
	const NxDebugLine* Lines = data.getLines();

	Cleanup();
	m_points = new Point3[NbLines*2];

	while(NbLines--)
	{
		float red = 0.f, green = 0.f, blue = 0.f;
		DebugSetupColor(Lines->color, red, green, blue);
		lineproc.SetLineColor(red, green, blue);
		m_points[m_numElements*2].Set(Lines->p0.x, Lines->p0.y, Lines->p0.z);
		m_points[m_numElements*2+1].Set(Lines->p1.x, Lines->p1.y, Lines->p1.z);
		lineproc.proc(&(m_points[m_numElements*2]), 2);
		Lines++;
		m_numElements++;
	}
#endif

}

void MxDebugVisualizer::GetViewportRect(TimeValue t, ViewExp *vpt, Rect *rect)
{
	GraphicsWindow* gw = vpt->getGW();

	if(gw)
	{
		rect->left = 0;
		rect->top = 0;
		rect->right = gw->getWinSizeX();
		rect->bottom = gw->getWinSizeY();
	}
}
