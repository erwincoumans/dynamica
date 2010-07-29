#include <NxPhysics.h>
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
