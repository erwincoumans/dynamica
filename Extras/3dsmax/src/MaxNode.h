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

#ifndef MAX_NODE_H
#define MAX_NODE_H



#include <max.h>
#include <list>
#include <map>
#include <iterator>
//#include "SimpleMesh.h"
#include "MxUtils.h"

class ccMaxNode;


struct ccPrimaryShapePara
{
	NxReal Radius;           // for NxSphereShape and NxCapsuleShape
	NxReal Height;           // for NxCapsuleShape
	NxVec3 BoxDimension;     // for NxBoxShape
};

/*
  ccMaxNode holds the information of the Max node. It converts mesh to PxSimpleMesh for further unified usage.
*/

class ccMaxNode {
	friend class ccMaxWorld;
private:
	ccMaxNode(INode* node);
	virtual ~ccMaxNode();
public:
	void                  SetNode(INode* node);
	Matrix3               GetCurrentTM();                      // get current pose TM, unit changed

	void                  SyncFromMaxMesh();

	inline INode*         GetMaxNode()    { return MaxINode; }
	inline ULONG          GetMaxHandle()  { if(MaxINode) return MaxINode->GetHandle(); return NULL; }

	void                  SetSimulatedPose(Matrix3& pose, bool useScaleTM = true);		// use simulation result to update node pose; for RB, useScaleRM = true, for Cloth, false
	void                  RestorePose();                                                // restore original pose

	PxSimpleMesh          SimpleMesh;
	NxShapeType           ShapeType;
	ccPrimaryShapePara    PrimaryShapePara;

	Matrix3               PhysicsNodePoseTM;          // pose TM
	Matrix3               PhysicsNodePoseTMInv;       // pose TM's inverse
	Point3                NodePosInPhysics;           // object's position in physics world, != poseTM.GetRow(3)

protected:
	INode*                MaxINode;                 // store the Max Node pointer

	bool                  ScaledIsUnified;          // true, if the mesh is not scaled or the mesh is scaled equally at x/y/z

private:
	Matrix3               PhysicsNodeScaleTM;         // scale TM

	Matrix3               MaxNodeObjectTM;				// Max Node's object TM
	Matrix3               MaxPivotTM;         // node object TM
	Matrix3               MaxNodeTM;          // pose TM
};


#endif