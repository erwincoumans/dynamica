//#include <NxPhysics.h>

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>
#include <vector>
#include <iterator>
#include "MxUtils.h"
#include "MxPluginData.h"
#include "MxObjects.h"
#include "MxActor.h"
//#include "MxShape.h"
#include "MaxNode.h"
#include "MaxWorld.h"


extern CharStream *gCurrentstream;





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
//	m_actor = NULL;
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

	
}

void MxActor::releaseAllShapes()
{


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

	MaxMsgBox(NULL, _T("createShape"), _T("Error"), MB_OK);

	int geomType = MxUserPropUtils::GetUserPropInt(m_node, "GeometryType",1);
	NxShapeType type = node->ShapeType;

	switch (geomType)
	{
	case 1:
		{
			MaxMsgBox(NULL, _T("automatic"), _T("Error"), MB_OK);
		
			switch (type)
			{
			case NX_SHAPE_SPHERE:
				{
					MaxMsgBox(NULL, _T("sphere shape"), _T("Error"), MB_OK);
					break;
				};
			case NX_SHAPE_BOX:
				{
					MaxMsgBox(NULL, _T("box shape"), _T("Error"), MB_OK);
					break;
				}
			case NX_SHAPE_CAPSULE:
				{
					MaxMsgBox(NULL, _T("capsule shape"), _T("Error"), MB_OK);
					break;
				}

			case NX_SHAPE_CONVEX:
				{
					MaxMsgBox(NULL, _T("convex shape"), _T("Error"), MB_OK);
					break;
				}
			case 	NX_SHAPE_MESH:
				{
					MaxMsgBox(NULL, _T("mesh shape"), _T("Error"), MB_OK);
					break;
				}

			default:
				{
					MaxMsgBox(NULL, _T("unknown shape type"), _T("Error"), MB_OK);
				}
			};
			break;
		}

	case 2:
		{
			MaxMsgBox(NULL, _T("override sphere shape"), _T("Error"), MB_OK);
			type = NX_SHAPE_SPHERE;
			break;
		}

	case 3:
		{
			MaxMsgBox(NULL, _T("override box shape"), _T("Error"), MB_OK);
			type = NX_SHAPE_BOX;
			break;
		}
	case 4:
		{
			MaxMsgBox(NULL, _T("override capsule shape"), _T("Error"), MB_OK);
			type = NX_SHAPE_CAPSULE;
			break;
		}

	case 5:
	{
		MaxMsgBox(NULL, _T("override convex shape"), _T("Error"), MB_OK);
		type = NX_SHAPE_CONVEX;
		break;
	}

	case 6:
	{
		MaxMsgBox(NULL, _T("override concave trimesh shape"), _T("Error"), MB_OK);
		type = NX_SHAPE_MESH;
		break;
	}

	default:
			{
				MaxMsgBox(NULL, _T("unknown shape type"), _T("Error"), MB_OK);
			}
	}

#if 0
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
	/*
	bool NeedCCDSkeleton = (Interactivity != RB_STATIC) && (MxUserPropUtils::GetUserPropBool(node->GetMaxNode(), "EnableCCD", false));
	if(NeedCCDSkeleton) 
	{
		NxPhysicsSDK* psdk = gPluginData->getPhysicsSDK();
		psdk->setParameter(NX_CONTINUOUS_CD, 1);
		psdk->setParameter(NX_CCD_EPSILON, 0.01f);
	}
	*/


	// create CCD skeleton for the shape
	//TODO("implement CCD skeleton creation");
	PX_CCD_SKELETON ccdType = (PX_CCD_SKELETON) MxUserPropUtils::GetUserPropInt(node->GetMaxNode(), "px_shape_ccdtype", 1);
	switch(type)
	{
	
	default:
		if (gCurrentstream) gCurrentstream->printf("Unable to create a shape of node \"%s\", unknown shape type: %d.\n", node->GetMaxNode()->GetName(), type);
		return false;
	}
	// load property settings and material things.
	//LoadShapeProperties(*pd, node->GetMaxNode());
	//CCD flag

	pd->name = node->GetMaxNode()->GetName();
	pd->userData = node;
	//
	bool isvalid = pd->isValid();
	actorDesc.shapes.push_back(pd);
#endif

	return true;
}

NxActor* MxActor::createNxActor()
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
	actorDesc.globalPose = pActorNode->PhysicsNodePoseTM;//MxMathUtils::MaxMatrixToNx(pActorNode->PhysicsNodePoseTM);
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
	
	//m_actor = gPluginData->getScene()->createActor(actorDesc);
	//if(! m_actor) return 0;

	//NxMat34 pose1 = m_actor->getGlobalPose();
	// sometimes pose1 != pose0; it is strange
	//m_actor->userData = this;

	if(MxUserPropUtils::GetUserPropBool(m_node, "PutToSleep", false))
	{
		//m_actor->putToSleep();
	}
	// for collision force when contact happens
	/*
	NxU32 num = m_actor->getNbShapes();
	NxShape*const* ps = m_actor->getShapes();
	for(NxU32 i = 0; i < num; ++i)
	{
		NxShape* nxShape = ps[i];
		nxShape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
	}
	*/


	//save last pose
	Matrix3 poseTM = maxNodeActor->GetCurrentTM();
	SaveLastPose(poseTM);


	//return m_actor;
	return 0;
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
#if 0
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
#endif 
}

/*
  Need store the pose after simulation
*/
void MxActor::ActionAfterSimulation()
{
#if 0
	// update pose to max node
	if(getInteractivity() != RB_DYNAMIC)
		return;

	// update rigid body's last pose
	//ccMaxNode* pn = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);
	maxNodeActor->SetSimulatedPose(m_actor->getGlobalPose());
	Matrix3 poseTM = maxNodeActor->GetCurrentTM();
	SaveLastPose(poseTM);
#endif 
}

