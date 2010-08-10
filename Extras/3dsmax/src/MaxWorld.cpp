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

#include "MaxWorld.h"
#include <SimpObj.h>
#include "MxUtils.h"
#include "MaxNode.h"
#include <decomp.h> 

ccMaxNodeMapContainer   ccMaxWorld::Nodes;


//Matrix3              ccMaxWorld::SystemTM(true);
//Matrix3              ccMaxWorld::SystemTMInv(true);
float                ccMaxWorld::UnitChange = 1.0f;

TimeValue            ccMaxWorld::MaxTime()
{
	return GetCOREInterface()->GetTime();
}


void ccMaxWorld::RefreshTime()
{
	//MaxTime = GetCOREInterface()->GetTime();
}

void ccMaxWorld::SetUnitChange(float unitChange)
{
	UnitChange = unitChange;
	//SystemTM.SetScale(Point3(UnitChange, UnitChange, UnitChange));
	//SystemTMInv = Inverse(SystemTM);
}

Matrix3 ccMaxWorld::ChangeToMaxUnit(Matrix3& tm)
{
	Matrix3 tmp(tm);
	Point3 p = ChangeToMaxUnit(tmp.GetRow(3));
	tmp.SetRow(3, p);
	return (tmp);
}

Matrix3 ccMaxWorld::ChangeToPhysXUnit(Matrix3& tm)
{
	Matrix3 tmp(tm);
	Point3 p = ChangeToPhysXUnit(tmp.GetRow(3));
	tmp.SetRow(3, p);
	return (tmp);
}
//
//Matrix3  ccMaxWorld::MaxTMToPhysX(Matrix3& nodeTM)
//{
//	nodeTM = ccMaxWorld::ChangeToPhysXUnit(nodeTM);
//	// get scale TM
//	Matrix3 scaleTM, poseTM;
//	ParseScale(nodeTM, scaleTM, poseTM);
//	return poseTM;
//}
//
//Matrix3  ccMaxWorld::GetCurrentTM(INode* node)
//{
//	assert(node);
//	const TimeValue t  = ccMaxWorld::MaxTime();  //GetCOREInterface()->GetTime();
//
//	Matrix3 nodeTM      = node->GetNodeTM(t);
//	return MaxTMToPhysX(nodeTM);
//}

Point3 ccMaxWorld::ChangeToMaxUnit(const Point3& pos)
{
	return (pos / GetUnitChange());
}

Point3 ccMaxWorld::ChangeToPhysXUnit(const Point3& pos)
{
	return (pos * GetUnitChange());
}

/*
  make tm = scaleTM x poseTM
*/
Point3 ccMaxWorld::ParseScale(const Matrix3& tm, Matrix3 &scaleTM, Matrix3 &poseTM)
{
	/* --- it is the old solution. it works.
	scaleTM.IdentityMatrix();
	poseTM.IdentityMatrix();
	//
	Point3 X = tm.GetRow(0);
	Point3 Y = tm.GetRow(1);
	Point3 Z = tm.GetRow(2);
	Point3 T = tm.GetRow(3);

	Point3 scale;

	scale.x = X.Length();
	X.Normalize();

	Y = Y - (X * DotProd(Y, X));
	Z = Z - (X * DotProd(Z, X));

	scale.y = Y.Length();
	Y.Normalize();

	Z = Z - (Y * DotProd(Z, Y));

	scale.z = Z.Length();
	Z.Normalize();

	Point3 t1(scale.x, 0.0f, 0.0f), t2(0.0f, scale.y, 0.0f), t3(0.0f, 0.0f, scale.z);
	scaleTM.SetRow(0, t1);
	scaleTM.SetRow(1, t2);
	scaleTM.SetRow(2, t3);
	poseTM = Inverse(scaleTM);
	poseTM *= tm;
	*/
	scaleTM.IdentityMatrix();
	poseTM.IdentityMatrix();

	AffineParts ap;
	decomp_affine(tm, &ap);
	ApplyScaling(scaleTM, ScaleValue(ap.k * ap.f, ap.u)); 
	poseTM = Inverse(scaleTM) * tm;
	Point3 x = scaleTM.GetRow(0), y = scaleTM.GetRow(1), z = scaleTM.GetRow(2);
	Point3 scale(x.x, y.y, z.z);
	return scale;
}



/*
  Find the created ccMaxNode from node. If not find it, generate a new one.
*/
ccMaxNode* ccMaxWorld::FindNode(INode* node)
{
	if(node == NULL)
		return NULL;
	ccMaxNodeMapContainerIter iter = Nodes.find(node);
	if(iter != Nodes.end() && (iter->second != NULL))
		return iter->second;
	ccMaxNode* pn = new ccMaxNode(node);
	Nodes[node] = pn;
	return pn;
}

void ccMaxWorld::FreeNodes()
{
	for(ccMaxNodeMapContainerIter iter = Nodes.begin(); iter != Nodes.end(); ++iter)
	{
		ccMaxNode* pn = iter->second;
		delete pn;
		iter->second = NULL;
	}
	Nodes.clear();
}


void ccMaxWorld::FreeNode(INode* node)
{
	if(node == NULL)
		return;
	ccMaxNodeMapContainerIter iter = Nodes.find(node);
	if(iter != Nodes.end() && (iter->second != NULL))
	{
		ccMaxNode* pn = iter->second;
		delete pn;
		iter->second = NULL;
		Nodes.erase(iter);
	}
}