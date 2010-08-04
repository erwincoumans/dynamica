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