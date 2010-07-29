#include <NxPhysics.h>

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>
#include <vector>
#include <iterator>
#include "MxPluginData.h"
#include "MxObjects.h"
#include "MxActor.h"
//#include "MxShape.h"
#include "MaxNode.h"
#include "MaxWorld.h"
#include "MxUtils.h"

extern CharStream *gCurrentstream;

class ccCCDSkeleton
{
public:
	static NxCCDSkeleton* createSkeleton(NxConvexShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxSphereShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxBoxShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxCapsuleShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxTriangleMeshShapeDesc* shape, PX_CCD_SKELETON type);
	static float zoomScale;
};
float ccCCDSkeleton::zoomScale = 0.9f;

NxCCDSkeleton* ccCCDSkeleton::createSkeleton(NxConvexShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;

	NxConvexMeshDesc mesh;
	shape->meshData->saveToDesc(mesh);
	
	PxSimpleMesh sm;
	sm.buildFrom(mesh);

	MxUtils::scaleMesh(sm, ccCCDSkeleton::zoomScale);
	PxSimpleMesh hull;
	MxUtils::pxCreateConvexHull(hull, sm, 64, 0.0025f);  // max 64 faces
	sm.release();

	NxSimpleTriangleMesh pointSkeleton;
	pointSkeleton.numVertices = hull.numPoints;
	pointSkeleton.numTriangles = hull.numFaces;
	pointSkeleton.pointStrideBytes = sizeof(NxVec3);
	pointSkeleton.triangleStrideBytes = sizeof(NxU32) * 3;

	pointSkeleton.points = hull.points;
	pointSkeleton.triangles = hull.faces;
	pointSkeleton.flags |= NX_MF_FLIPNORMALS;
	ret = gPluginData->getPhysicsSDK()->createCCDSkeleton(pointSkeleton);
	hull.release();
	return ret;
}

NxCCDSkeleton* ccCCDSkeleton::createSkeleton(NxSphereShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	NxReal radius = shape->radius * ccCCDSkeleton::zoomScale;
	NxU32 triangles[] = {
		1, 0, 2,
		2, 0, 3,
		3, 0, 4,
		4, 0, 1,
		1, 5, 4,
		2, 5, 1,
		3, 5, 2,
		4, 5, 3
	};
	NxVec3 points[6];
	points[0].set(0, radius, 0);
	points[1].set(0, 0, radius);
	points[2].set(-radius, 0, 0);
	points[3].set(0, 0, -radius);
	points[4].set(radius, 0, 0);
	points[5].set(0, -radius, 0);

	NxSimpleTriangleMesh stm;
	stm.numVertices = sizeof(points)/sizeof(NxVec3);
	stm.numTriangles = sizeof(triangles)/sizeof(NxU32)/3;
	stm.pointStrideBytes = sizeof(NxVec3);
	stm.triangleStrideBytes = sizeof(NxU32)*3;

	stm.points = points;
	stm.triangles = triangles;
	stm.flags |= NX_MF_FLIPNORMALS;
	ret = gPluginData->getPhysicsSDK()->createCCDSkeleton(stm);
	return ret;
}

NxCCDSkeleton* ccCCDSkeleton::createSkeleton(NxBoxShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	NxU32 triangles[3 * 12] = { 
		0,1,3,
		0,3,2,
		3,7,6,
		3,6,2,
		1,5,7,
		1,7,3,
		4,6,7,
		4,7,5,
		1,0,4,
		5,1,4,
		4,0,2,
		4,2,6
	};
	NxVec3 points[8];

	//static mesh
	NxVec3 size = shape->dimensions * ccCCDSkeleton::zoomScale;
	points[0].set( size.x, -size.y, -size.z);
	points[1].set( size.x, -size.y,  size.z);
	points[2].set( size.x,  size.y, -size.z);
	points[3].set( size.x,  size.y,  size.z);

	points[4].set(-size.x, -size.y, -size.z);
	points[5].set(-size.x, -size.y,  size.z);
	points[6].set(-size.x,  size.y, -size.z);
	points[7].set(-size.x,  size.y,  size.z);

	NxSimpleTriangleMesh stm;
	stm.numVertices = sizeof(points)/sizeof(NxVec3);
	stm.numTriangles = sizeof(triangles)/sizeof(NxU32)/3;
	stm.pointStrideBytes = sizeof(NxVec3);
	stm.triangleStrideBytes = sizeof(NxU32)*3;

	stm.points = points;
	stm.triangles = triangles;
	stm.flags |= NX_MF_FLIPNORMALS;
	ret = gPluginData->getPhysicsSDK()->createCCDSkeleton(stm);
	return ret;
}

NxCCDSkeleton* ccCCDSkeleton::createSkeleton(NxCapsuleShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	NxReal radius = shape->radius * ccCCDSkeleton::zoomScale;
	NxReal height = shape->height;

	NxU32 triangles[] = {
		2, 0, 1,
		3, 0, 2,
		4, 0, 3,
		1, 0, 4,
		6, 2, 1,
		5, 6, 1,
		3, 2, 6,
		6, 7, 3,
		4, 3, 7,
		8, 4, 7,
		1, 4, 8,
		8, 5, 1,
		8, 9, 5,
		7, 9, 8,
		6, 9, 7,
		5, 9, 6
	};
	NxVec3 points[10];
	points[0].set(0, height + radius, 0);
	points[1].set(radius, height, 0);
	points[2].set(0, height, radius);
	points[3].set(-radius, height, 0);
	points[4].set(0, height, -radius);
	points[5].set(radius, -height, 0);
	points[6].set(0, -height, radius);
	points[7].set(-radius, -height, 0);
	points[8].set(0, -height, -radius);
	points[9].set(0, -height - radius, 0);

	NxSimpleTriangleMesh stm;
	stm.numVertices = sizeof(points)/sizeof(NxVec3);
	stm.numTriangles = sizeof(triangles)/sizeof(NxU32)/3;
	stm.pointStrideBytes = sizeof(NxVec3);
	stm.triangleStrideBytes = sizeof(NxU32)*3;

	stm.points = points;
	stm.triangles = triangles;
	stm.flags |= NX_MF_FLIPNORMALS;
	ret = gPluginData->getPhysicsSDK()->createCCDSkeleton(stm);
	return ret;
}

NxCCDSkeleton* ccCCDSkeleton::createSkeleton(NxTriangleMeshShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	return ret;
}


NxActor* MxActor::getNxActor() {
	//if (m_actor == NULL)
	//{
	//	if (m_desc.isValid())
	//	{
	//		m_actor = gPluginData->getScene()->createActor(m_desc);
	//		if (m_actor == NULL) 
	//		{
	//			if (gCurrentstream) gCurrentstream->printf("Unable to create the NxActor object for \"%s\".\n", getName());
	//			return NULL;
	//		}

	//		m_actor->userData = this;

	//		assert(m_actor->getNbShapes() == m_shapes.size());
	//		if (m_actor->getNbShapes() == m_shapes.size())
	//		{
	//			for (NxU32 i = 0; i < m_shapes.size(); i++)
	//			{
	//				m_shapes[i]->m_nxShape = m_actor->getShapes()[i];
	//				// need get force at contact
	//				m_shapes[i]->m_nxShape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
	//			}
	//		} else {
	//			//Warning! the number of shapes are not the same, we need to do something drastic
	//			while (m_actor->getNbShapes() > 0)
	//			{
	//				m_actor->releaseShape(*m_actor->getShapes()[0]);
	//			}
	//			for (NxU32 i = 0; i < m_shapes.size(); i++)
	//			{
	//				NxShapeDesc* shapeDesc = m_shapes[i]->getShapeDesc(this);
	//				if (shapeDesc != NULL)
	//				{
	//					NxShape* nxShape = m_actor->createShape(*shapeDesc);
	//					if (nxShape != NULL)
	//					{
	//						m_shapes[i]->m_nxShape = nxShape;
	//						// need get force at contact
	//						nxShape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
	//					}
	//				}
	//			}
	//		}
	//		m_mass = m_actor->getMass();
	//	}
	//}
	return m_actor; 
}
//
//bool MxActor::addShape(MxShape* shape)
//{
//	//for debugging purposes:
//	for (NxU32 i = 0; i < m_shapes.size(); i++)
//	{
//		if (m_shapes[i] == shape) 
//		{
//			assert(false);
//			return false;
//		}
//	}
//	m_shapes.pushBack(shape);
//
//	NxShapeDesc* desc = shape->getShapeDesc(this);
//	m_desc.shapes.pushBack(desc);
//
//	//if the actor has already been created, then we need to also add the shape
//	if (m_actor != NULL)
//	{
//		NxShape* nxShape = m_actor->createShape(*desc);
//		shape->m_nxShape = nxShape;
//		// need get force at contact
//		nxShape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
//	}
//	return true;
//}
//
//bool MxActor::removeShape(MxShape* shape)
//{
//	for (NxU32 i = 0; i < m_shapes.size(); i++)
//	{
//		if (m_shapes[i] == shape)
//		{
//			m_shapes.replaceWithLast(i);
//			//this should always be true (we always add and delete these items in the same order
//			assert(m_desc.shapes[i] == shape->getShapeDesc(this));
//			m_desc.shapes.replaceWithLast(i);
//
//			if (m_actor != NULL)
//			{
//				if (shape->getNxShape() != NULL)
//				{
//					m_actor->releaseShape(*shape->getNxShape());
//					shape->m_nxShape = NULL;
//				}
//				shape->removeFromActor(this);
//			}
//			return true;
//		}
//	}
//	//We should not try to remove a shape that does not exist in the actor
//	assert(false);
//	return false;
//}

MxActor::MxActor(const char* name, INode* node) : MxObject(name, node), maxNodeActor(0), maxNodeProxy(0)
{
	m_ObjectType = MX_OBJECTTYPE_ACTOR;
	m_actor = NULL;
	m_desc.name = m_name.data(); //safe, since MxActor lives longer than the NxActor created by it
	m_desc.userData = this;
	m_proxyNode = NULL;
	m_sleep = false;
	m_isKenematic = false;
	m_mass = 0.0f;
	Interactivity = RB_DYNAMIC;
}

MxActor::~MxActor() 
{
	//all shapes should have been removed before deleting the actor
	//assert(m_shapes.size() == 0);

	if (m_actor != NULL)
	{
		gPluginData->getScene()->releaseActor(*m_actor);
		m_actor = NULL;
	}
}

void MxActor::releaseAllShapes()
{
	if (m_actor != NULL)
	{
		gPluginData->getScene()->releaseActor(*m_actor);
		m_actor = NULL;
	}

	//for (NxU32 i = 0; i < m_shapes.size(); i++)
	//{
	//	m_shapes[i]->m_nxShape = NULL;
	//	m_shapes[i]->removeFromActor(this);
	//	MxPluginData::releaseShape(m_shapes[i]);
	//}
	//m_shapes.clear();
}

void MxActor::releaseNxActor()
{
	if (m_actor != NULL)
	{
		gPluginData->getScene()->releaseActor(*m_actor);
		m_actor = NULL;
	}
	//for (NxU32 i = 0; i < m_shapes.size(); i++)
	//{
	//	m_shapes[i]->m_nxShape = NULL;
	//}
}

/*
  Use the shape of node to create PhysX shape for the actor. ActorNode is the main Max node.
*/
bool MxActor::createShape(NxActorDesc& actorDesc, ccMaxNode* node, ccMaxNode* actorNode)
{
	const TimeValue t = ccMaxWorld::MaxTime();

	NxShapeDesc* pd = NULL;
	NxShapeType type = node->ShapeType;
	PxSimpleMesh proxyMesh;
	Matrix3 nodePose = node->PhysicsNodePoseTM;
	if(m_proxyNode)
	{
		// for proxy, using mesh
		//type = NX_SHAPE_MESH;
		proxyMesh.clone(node->SimpleMesh);
		Point3 pos = nodePose.GetRow(3);
		pos = pos + ProxyDistance;
		nodePose.SetRow(3, pos);
	}
	if((type == NX_SHAPE_MESH) && (Interactivity != RB_STATIC))
	{
		type = NX_SHAPE_CONVEX;
	}
	//
	bool NeedCCDSkeleton = (Interactivity != RB_STATIC) && (MxUserPropUtils::GetUserPropBool(node->GetMaxNode(), "EnableCCD", false));
	if(NeedCCDSkeleton) 
	{
		NxPhysicsSDK* psdk = gPluginData->getPhysicsSDK();
		psdk->setParameter(NX_CONTINUOUS_CD, 1);
		psdk->setParameter(NX_CCD_EPSILON, 0.01f);
	}

	// create CCD skeleton for the shape
	//TODO("implement CCD skeleton creation");
	PX_CCD_SKELETON ccdType = (PX_CCD_SKELETON) MxUserPropUtils::GetUserPropInt(node->GetMaxNode(), "px_shape_ccdtype", 1);
	switch(type)
	{
	case NX_SHAPE_MESH:
		{
			NxTriangleMeshShapeDesc*d = new NxTriangleMeshShapeDesc();
			if(m_proxyNode)
			{
				d->meshData = MxUtils::nodeToNxTriangleMesh(proxyMesh);
				Matrix3 pose = nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			else
			{
				d->meshData = MxUtils::nodeToNxTriangleMesh(node->SimpleMesh);
				Matrix3 pose = nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			if(! d->meshData)
			{
				delete d;
				return false;
			}
			// add to record for reuse
			//ccObjectManager::instance().triangleMeshes.add(node->GetMaxNode(), d->meshData);
			//
			pd = d;
			// support ccd skeleton later
		}
		break;
	case NX_SHAPE_CONVEX:
		{
			NxConvexShapeDesc* d = new NxConvexShapeDesc();
			if(m_proxyNode)
			{
				d->meshData = MxUtils::nodeToNxConvexMesh(proxyMesh);
				Matrix3 pose = nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			else
			{
				d->meshData = MxUtils::nodeToNxConvexMesh(node->SimpleMesh);
				Matrix3 pose = nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			if(! d->meshData)
			{
				delete d;
				return false;
			}
			// add to record reuse
			//ccObjectManager::instance().convexMeshes.add(node->GetMaxNode(), d->meshData);
			//
			pd = d;
			// do not support skeleton now
			if(NeedCCDSkeleton) 
				pd->ccdSkeleton = ccCCDSkeleton::createSkeleton(d, ccdType);
		}
		break;
	case NX_SHAPE_SPHERE:
		{
			NxSphereShapeDesc* d = new NxSphereShapeDesc();
			d->radius = node->PrimaryShapePara.Radius;
			if(m_proxyNode)
			{
				Matrix3 pose = MxMathUtils::NxMatrixToMax(d->localPose) * nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			else
			{
				Matrix3 pose = MxMathUtils::NxMatrixToMax(d->localPose) * nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			pd = d;
			// do not support skeleton now
			if(NeedCCDSkeleton) 
				pd->ccdSkeleton = ccCCDSkeleton::createSkeleton(d, ccdType);
		}
		break;
	case NX_SHAPE_BOX:
		{
			NxBoxShapeDesc* d = new NxBoxShapeDesc();
			d->dimensions.x = node->PrimaryShapePara.BoxDimension.x;
			d->dimensions.y = node->PrimaryShapePara.BoxDimension.y;
			d->dimensions.z = node->PrimaryShapePara.BoxDimension.z;
			//
			d->localPose.t.z = node->PrimaryShapePara.BoxDimension.z;      //adjust for difference in pivot points between 3ds max and PhysX
			if(m_proxyNode)
			{
				Matrix3 pose = MxMathUtils::NxMatrixToMax(d->localPose) * nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			else
			{
				Matrix3 pose = MxMathUtils::NxMatrixToMax(d->localPose) * nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			for (int i = 0; i < 3; i++) 
				d->dimensions[i] = fabsf(d->dimensions[i]);               // prevent values in minus
			pd = d;
			// do not support skeleton now
			if(NeedCCDSkeleton) 
				pd->ccdSkeleton = ccCCDSkeleton::createSkeleton(d, ccdType);
		}
		break;
	case NX_SHAPE_CAPSULE:
		{
			int centersflag = 0;
			NxCapsuleShapeDesc* d = new NxCapsuleShapeDesc();
			d->radius = node->PrimaryShapePara.Radius;
			d->height = node->PrimaryShapePara.Height;
			d->localPose.M = NxMat33(NxVec3(1,0,0),NxVec3(0,0,1),NxVec3(0,-1,0));
			d->localPose.t.z = d->height/2 + d->radius;                  //adjust for difference in pivot points between 3ds max and PhysX
			if(m_proxyNode)
			{
				Matrix3 pose = MxMathUtils::NxMatrixToMax(d->localPose) * nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			else
			{
				Matrix3 pose = MxMathUtils::NxMatrixToMax(d->localPose) * nodePose * actorNode->PhysicsNodePoseTMInv;
				d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			pd = d;
			// do not support skeleton now
			if(NeedCCDSkeleton) 
				pd->ccdSkeleton = ccCCDSkeleton::createSkeleton(d, ccdType);
		}
		break;
	default:
		if (gCurrentstream) gCurrentstream->printf("Unable to create a shape of node \"%s\", unknown shape type: %d.\n", node->GetMaxNode()->GetName(), type);
		return false;
	}
	// load property settings and material things.
	//LoadShapeProperties(*pd, node->GetMaxNode());
	//CCD flag
	if(NeedCCDSkeleton)
	{
		pd->shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD;
	}
	pd->name = node->GetMaxNode()->GetName();
	pd->userData = node;
	//
	bool isvalid = pd->isValid();
	actorDesc.shapes.push_back(pd);

	return true;
}

NxActor* MxActor::createActor()
{
	// find proxy node
	m_proxyNode = NULL;
	TSTR str = MxUserPropUtils::GetUserPropStr(m_node, "Proxy_Geometry");
	char* proxyName = str.data();
	if(proxyName && strlen(proxyName) > 0 && (stricmp(proxyName, "<None>") != 0))
	{
		m_proxyNode = GetCOREInterface()->GetINodeByName(proxyName);
	}

	//
	NxScene* scene = gPluginData->getScene();
	if (! scene) return 0;
	ccMaxNode* pActorNode = ccMaxWorld::FindNode(m_node);
	assert(pActorNode);
	maxNodeActor = pActorNode;

	maxNodeProxy = NULL;
	if(m_proxyNode)
	{
		maxNodeProxy       = ccMaxWorld::FindNode(m_proxyNode);
		assert(maxNodeProxy);
		//Point3 p1 = maxNodeActor->PhysicsNodePoseTM.GetRow(3), p2 = maxNodeProxy->PhysicsNodePoseTM.GetRow(3);
		Point3 p1 = maxNodeActor->NodePosInPhysics, p2 = maxNodeProxy->NodePosInPhysics;
		ProxyDistance = p1 - p2;
	}

	NxActorDesc&  actorDesc  = m_desc;
	NxBodyDesc&   bodyDesc   = m_bodyDesc;
	//LoadParameters(actorDesc, bodyDesc);
	actorDesc.globalPose = MxMathUtils::MaxMatrixToNx(pActorNode->PhysicsNodePoseTM);
	SaveLastPose(pActorNode->PhysicsNodePoseTM);

	//m_bodydesc.solverIterationCount = (NxU32)mSetting_solveriterationcount;   // support it in future?

	std::vector<INode*> stack;
	std::vector<INode*> shapes;

	//Check if the object is using a proxy, in that case only those shapes should be added
	INode* current = m_node;
	if(m_proxyNode)
		current = m_proxyNode;
	stack.push_back(current);
	//DEBUG_S("List collision shapes");
	while (stack.size() > 0)
	{
		current = stack[0];
		if(stack.size() > 1)
		{
			stack[0] = stack.back();
		}
		stack.pop_back();

		if(current->EvalWorldState(0).obj->SuperClassID()==GEOMOBJECT_CLASS_ID)
		{
			shapes.push_back(current);
		}

		//go through grouped objects
		for(int i = 0; i < current->NumberOfChildren(); i++)
		{
			INode *c = current->GetChildNode(i);
			if (c->IsGroupMember()) 
			{
				stack.push_back(c);
			}
		}
	}

	if (shapes.size() == 0)
	{
		if (gCurrentstream) gCurrentstream->printf("Unable to add %s as an actor, it has no shapes.\n", m_node->GetName());
		return 0;
	}

	ccMaxNode* baseNode = pActorNode;
	if(maxNodeProxy)
		baseNode = maxNodeProxy;
	for (int i = 0; i < shapes.size(); i++) 
	{
		INode* current = shapes[i];
		//DEBUG_F(" Debug adding shape node: %s\n", current->GetName());
		ccMaxNode* pn = ccMaxWorld::FindNode(current);
		assert(pn);
		createShape(actorDesc, pn, pActorNode);
	}
	bool isvalid = actorDesc.isValid();
	actorDesc.name = m_node->GetName();
	//NxMat34 pose0 = actorDesc.globalPose;
	m_actor = gPluginData->getScene()->createActor(actorDesc);
	if(! m_actor) return 0;
	//NxMat34 pose1 = m_actor->getGlobalPose();
	// sometimes pose1 != pose0; it is strange
	m_actor->userData = this;

	if(MxUserPropUtils::GetUserPropBool(m_node, "PutToSleep", false))
	{
		m_actor->putToSleep();
	}
	// for collision force when contact happens
	NxU32 num = m_actor->getNbShapes();
	NxShape*const* ps = m_actor->getShapes();
	for(NxU32 i = 0; i < num; ++i)
	{
		NxShape* nxShape = ps[i];
		nxShape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
	}

	//save last pose
	Matrix3 poseTM = maxNodeActor->GetCurrentTM();
	SaveLastPose(poseTM);
	return m_actor;
}

void MxActor::resetObject()
{
	//ccMaxNode* pActorNode = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);
	maxNodeActor->RestorePose();
}

void MxActor::SaveLastPose(const Matrix3 & pose)
{
	LastPose = pose;
}

/*
  1. to move kinematic and static RB with the Max animation or user's drag.
  2. to add the Dynamic RB with a speed if user drags and throws the Max object.
*/
void MxActor::ActionBeforeSimulation()
{
	//ccMaxNode* pn = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);
	Matrix3 poseTM = maxNodeActor->GetCurrentTM();
	if(GetLastPose() == poseTM)
		return;

	NxMat34 nxPose = MxMathUtils::MaxMatrixToNx(poseTM);
	if(getInteractivity() == RB_KINEMATIC)
	{
		// kinematic actor, driven by animation
		m_actor->moveGlobalPose(nxPose);
	}
	else if(getInteractivity() == RB_DYNAMIC)
	{
		// if user drag/throw one object, make it as speed
		//NxMat34 nxOldPose = MxMathUtils::MaxMatrixToNx(GetLastPose());
		//NxVec3  speed     = (nxPose.t - nxOldPose.t) * 2;
		//m_actor->setLinearVelocity(speed);
	}
	else
	{
		// static actor, driven by animation
		m_actor->setGlobalPose(nxPose);
	}
	// update rigid body's last pose
	SaveLastPose(poseTM);
}

/*
  Need store the pose after simulation
*/
void MxActor::ActionAfterSimulation()
{
	// update pose to max node
	if(getInteractivity() != RB_DYNAMIC)
		return;

	// update rigid body's last pose
	//ccMaxNode* pn = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);
	maxNodeActor->SetSimulatedPose(m_actor->getGlobalPose());
	Matrix3 poseTM = maxNodeActor->GetCurrentTM();
	SaveLastPose(poseTM);
}

