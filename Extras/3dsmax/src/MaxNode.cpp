#include "MaxNode.h"
#include <SimpObj.h>
#include "MxUtils.h"
#include "MaxWorld.h"
#include "PxPlugin.h"
#include <max.h>
#include "MAXScrpt\MAXScrpt.h"

extern CharStream* gCurrentstream;
const NxReal gTolerenceEpsilon = 0.001f;

ccMaxNode::ccMaxNode(INode* node)  : MaxINode(node), MaxNodeObjectTM(true), ScaledIsUnified(true), ShapeType(NX_SHAPE_MESH)
{
	SetNode(node);
}

ccMaxNode::~ccMaxNode() 
{
}

void ccMaxNode::SetNode(INode* node)
{
	MaxMsgBox(NULL, _T("SetNode"), _T("Error"), MB_OK);
	MaxINode = node;
	SimpleMesh.release();
	ScaledIsUnified = true;
	MaxNodeObjectTM.IdentityMatrix();
	if(MaxINode) {
		SyncFromMaxMesh();
	}
}

void RemovePivotScale(INode* node)
{
	const TimeValue  t  = ccMaxWorld::MaxTime();
	Matrix3 nodeTM = node->GetNodeTM(t);

	Point3 pos = nodeTM.GetRow(3);
	Matrix3 objectTM = node->GetObjectTM(t);
	nodeTM = objectTM;
	nodeTM.SetRow(3, pos);
	Matrix3 pv = objectTM * Inverse(nodeTM);
	node->SetNodeTM(t, nodeTM);

	node->SetObjOffsetPos(pv.GetTrans()); 
	node->SetObjOffsetRot(IdentQuat()); 
	node->SetObjOffsetScale(ScaleValue(Point3(1,1,1))); 
}

/*void ParseMatrix(Matrix3& tmMax, Matrix3& tmPhysX)
{
	NxMat34 m = MxMathUtils::MaxMatrixToNx(tmMax);
	NxQuat  q;
	m.M.toQuat(q);
	NxMat34 p(q, m.t);
	tmPhysX = MxMathUtils::NxMatrixToMax(p);
}
*/


bool PivotScaled(ScaleValue& sv)
{
	ScaleValue noscale(Point3(1,1,1));
	Point3 p = sv.s - noscale.s;
	Point3 q(sv.q.x - noscale.q.x, sv.q.y - noscale.q.y, sv.q.z - noscale.q.z);
	float w = sv.q.w - noscale.q.w;
	float  v = p.LengthSquared() + q.LengthSquared() + w * w;
	return v > gTolerenceEpsilon;
}

bool IsScaled(Matrix3& tm)
{
	Matrix3 t(1);
	t = tm - t;
	Point3 x = t.GetRow(0), y = t.GetRow(1), z = t.GetRow(2), u = t.GetRow(3);
	float v = x.LengthSquared() + y.LengthSquared() + z.LengthSquared() + u.LengthSquared();
	if(v > gTolerenceEpsilon)
		return true;
	else
		return false;
}

/*
  Transfer the max Node's mesh to PhysX unit and store it.
*/
void ccMaxNode::SyncFromMaxMesh()
{
	MaxMsgBox(NULL, _T("SyncFromMaxMesh"), _T("Error"), MB_OK);

	ShapeType           = NX_SHAPE_MESH;
	const TimeValue  t  = ccMaxWorld::MaxTime();

	Matrix3 tm = MaxINode->GetNodeTM(t);
	NodePosInPhysics = tm.GetRow(3);
	NodePosInPhysics = ccMaxWorld::ChangeToPhysXUnit(NodePosInPhysics);

	ScaleValue sv = MaxINode->GetObjOffsetScale();
	bool objectIsScaled = PivotScaled(sv);
	//if(objectIsScaled)
	//	RemovePivotScale(MaxINode);

	//------------------------
	MaxNodeTM = MaxINode->GetNodeTM(t);
	// get pivot TM
	MaxPivotTM.IdentityMatrix();
	MaxPivotTM.SetTrans(MaxINode->GetObjOffsetPos()); 
	PreRotateMatrix(MaxPivotTM, MaxINode->GetObjOffsetRot()); 
	ApplyScaling(MaxPivotTM, MaxINode->GetObjOffsetScale());

	// GetObjectTM() = MaxPivotTM * MaxNodeTM
	MaxNodeObjectTM = MaxINode->GetObjectTM(t);
	//-----------------------------------------------------------------------------------------------
	// object might be modified by world space modifiers. We need a solution in future for this case.
	bool isWorldSpace = MaxINode->GetObjTMAfterWSM(t).IsIdentity(); 
	//-----------------------------------------------------------------------------------------------
	Matrix3 PhysicsNodeTM = ccMaxWorld::ChangeToPhysXUnit(MaxNodeObjectTM);
	// get scale TM
	// PhysicsNodeTM == PhysicsNodeScaleTM * PhysicsNodePoseTM
	Point3 maxNodeScale = ccMaxWorld::ParseScale(PhysicsNodeTM, PhysicsNodeScaleTM, PhysicsNodePoseTM);      
	//Matrix3 right;
	//ParseMatrix(PhysicsNodePoseTM, right);
	//PhysicsNodePoseTM = right;
	//PhysicsNodeScaleTM = PhysicsNodeTM * Inverse(PhysicsNodePoseTM);
	//if(IsScaled(PhysicsNodeScaleTM))
	//	objectIsScaled = true;
	// check whether the Node's mesh is scaled equally at x/y/z
	ScaledIsUnified  = (fabs((maxNodeScale.x - maxNodeScale.z)/maxNodeScale.z) < gTolerenceEpsilon) && (fabs((maxNodeScale.y - maxNodeScale.z)/maxNodeScale.z) < gTolerenceEpsilon);
	ScaledIsUnified  = (! objectIsScaled) && ScaledIsUnified;
	//if(! ScaledIsUnified) {
	//	PhysicsNodePoseTM.IdentityMatrix();
	//	PhysicsNodePoseTM.SetRow(3, NodePosInPhysics);
	//	PhysicsNodeScaleTM = PhysicsNodeTM * Inverse(PhysicsNodePoseTM);
	//}
	PhysicsNodePoseTMInv = Inverse(PhysicsNodePoseTM);

	//gCurrentstream->printf("\nNxScaleTM = ");	MxUtils::PrintMatrix3(PhysicsNodeScaleTM); 
	//gCurrentstream->printf("\nNxPoseTM = ");	MxUtils::PrintMatrix3(PhysicsNodePoseTM); 
	//gCurrentstream->printf("\n");
	//Matrix3 t1 = PhysicsNodeScaleTM * PhysicsNodePoseTM;
	//bool e1 = (t1 == MaxNodeObjectTM);
	Object* obj   = MaxINode->EvalWorldState(t).obj;
	if (obj != NULL)
	{
		SimpleObject* so = (SimpleObject*)obj;
		Class_ID id = obj->ClassID();
		if (id == Class_ID(SPHERE_CLASS_ID, 0)) {
			ShapeType = NX_SHAPE_SPHERE;
			so->pblock->GetValue(SPHERE_RADIUS, 0, PrimaryShapePara.Radius, FOREVER);
			PrimaryShapePara.Radius *= maxNodeScale.x * ccMaxWorld::GetUnitChange();         // x/y/z is scaled with a same value
		}
		else if (id == Class_ID(BOXOBJ_CLASS_ID, 0)) {
			ShapeType = NX_SHAPE_BOX;
			so->pblock->GetValue(BOXOBJ_WIDTH , 0, PrimaryShapePara.BoxDimension[0], FOREVER);
			so->pblock->GetValue(BOXOBJ_LENGTH, 0, PrimaryShapePara.BoxDimension[1], FOREVER);
			so->pblock->GetValue(BOXOBJ_HEIGHT, 0, PrimaryShapePara.BoxDimension[2], FOREVER);
			PrimaryShapePara.BoxDimension *= (0.5f * maxNodeScale.x * ccMaxWorld::GetUnitChange());          // x/y/z is scaled with a same value     // Physics box is half the size
		}
		else if (id == CAPS_CLASS_ID) {
			ShapeType = NX_SHAPE_CAPSULE;
			int centersflag = 0;
			so->pblock->GetValue(CAPS_RADIUS , 0, PrimaryShapePara.Radius, FOREVER);
			so->pblock->GetValue(CAPS_HEIGHT , 0, PrimaryShapePara.Height, FOREVER);
			so->pblock->GetValue(CAPS_CENTERS, 0, centersflag, FOREVER);
			if(!centersflag) //there are some different ways in which you can specify a capsule in 3ds max, adjust length if "center" mode is not used
				PrimaryShapePara.Height -= PrimaryShapePara.Radius * 2.0f; 
			PrimaryShapePara.Radius *= maxNodeScale.x * ccMaxWorld::GetUnitChange();         // x/y/z is scaled with a same value
			PrimaryShapePara.Height *= maxNodeScale.x * ccMaxWorld::GetUnitChange();         // x/y/z is scaled with a same value
		}
	}
	if(! ScaledIsUnified)
		ShapeType     = NX_SHAPE_MESH;
	// Disable backface culling for cloth
	//MaxINode->BackCull(FALSE);
	// get mesh
	BOOL needDel = FALSE;
	TriObject* tri = MxUtils::GetTriObjectFromNode(MaxINode, t, needDel);
	if (tri == NULL) return;

	Mesh& mesh = tri->GetMesh();
	SimpleMesh.alloc(mesh.getNumVerts(), mesh.getNumFaces());
	//Matrix3 change = PhysicsNodeTM * Inverse(PhysicsNodePoseTM);
	for(NxU32 i = 0; i < SimpleMesh.numPoints; i++) 
	{
		Point3 tmp = mesh.verts[i] * ccMaxWorld::GetUnitChange() * PhysicsNodeScaleTM;
		((Point3*)SimpleMesh.points)[i] =  tmp;  // systemTM is unit change TM.
	}
	for(NxU32 i = 0; i < SimpleMesh.numFaces; i++) 
	{
		for(NxU32 j = 0; j < 3; j++) 
		{ 
			SimpleMesh.faces[i*3+j] = mesh.faces[i].v[j];
		}
	}
	if (needDel)
		tri->DeleteMe();
}

/*
  use simulated PhysX Pose to update Max Node's pose. Need consider unit scaling
*/
void ccMaxNode::SetSimulatedPose(Matrix3& p, bool useScaleTM)
{
	if(useScaleTM)
		p = (PhysicsNodeScaleTM * p);
	p = ccMaxWorld::ChangeToMaxUnit(p);
	Matrix3 t = Inverse(MaxPivotTM) * p;// * Inverse(parentTM);
	MaxINode->SetNodeTM(ccMaxWorld::MaxTime(), t);
}

/*
  Use stored original pose to reset the Max node
*/
void ccMaxNode::RestorePose()
{
	//gCurrentstream->printf("RestorePose for Node - %s\n", MaxINode->GetName());
	MaxINode->SetNodeTM(ccMaxWorld::MaxTime(), MaxNodeTM);
}

Matrix3  ccMaxNode::GetCurrentTM()
{
	const TimeValue t  = ccMaxWorld::MaxTime();  //GetCOREInterface()->GetTime();

	Matrix3 nodeTM     = MaxINode->GetObjectTM(t);
	Matrix3 tmp        = ccMaxWorld::ChangeToPhysXUnit(nodeTM);
	return Inverse(PhysicsNodeScaleTM) * tmp;
}

