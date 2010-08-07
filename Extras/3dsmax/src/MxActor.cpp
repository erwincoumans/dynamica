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
#include "btBulletDynamicsCommon.h"

extern CharStream *gCurrentstream;
extern btDiscreteDynamicsWorld* gDynamicsWorld;



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
	m_bulletBody = NULL;
	m_sleep = false;
	m_isKenematic = false;
	m_mass = 0.0f;
	Interactivity = RB_DYNAMIC;
}

MxActor::~MxActor() 
{
	//all shapes should have been removed before deleting the actor
	//assert(m_shapes.size() == 0);

	if (m_bulletBody)
	{
		if (gDynamicsWorld)
			gDynamicsWorld->removeRigidBody(m_bulletBody);
		delete m_bulletBody;
	}
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
  Use the shape of node to create Bullet shape for the actor. ActorNode is the main Max node.
*/
btCollisionShape* MxActor::createShape(NxActorDesc& actorDesc, ccMaxNode* node, ccMaxNode* actorNode)
{
	actorDesc.localPose.IdentityMatrix();

	btCollisionShape* shape = NULL;

	const TimeValue t = ccMaxWorld::MaxTime();
	Matrix3 nodePose = node->PhysicsNodePoseTM;

//	MaxMsgBox(NULL, _T("createShape"), _T("Error"), MB_OK);

	int geomType = MxUserPropUtils::GetUserPropInt(m_node, "GeometryType",1);
	NxShapeType type = node->ShapeType;

	//first apply manual overrides
	switch (geomType)
	{
	case 2:
		{
			type = NX_SHAPE_SPHERE;
			break;
		}
	case 3:
		{
			type = NX_SHAPE_BOX;
			break;
		}
	case 4:
		{
			type = NX_SHAPE_CAPSULE;
			break;
		}
	case 5:
		{
			type = NX_SHAPE_CONVEX;
			break;
		}
	case 6:
		{
			type = NX_SHAPE_MESH;
			break;
		}
	default:
		{
		}
	};

	switch (type)
	{
	case NX_SHAPE_SPHERE:
		{
			btScalar radius = node->PrimaryShapePara.Radius;
			shape = new btSphereShape(radius);
			break;
		};
	case NX_SHAPE_BOX:
		{
			//adjust for difference in pivot points between 3ds max and Bullet (Bullet uses the box center)
			actorDesc.localPose.SetTrans(2,node->PrimaryShapePara.BoxDimension.z());

			//why is this "node->PhysicsNodePoseTM * actorNode->PhysicsNodePoseTMInv;" necessary, isn't it identity?
			//Matrix3 pose = localPose * node->PhysicsNodePoseTM * actorNode->PhysicsNodePoseTMInv;

			shape = new btBoxShape(node->PrimaryShapePara.BoxDimension.absolute());

			break;
		}
	case NX_SHAPE_CAPSULE:
		{
			MaxMsgBox(NULL, _T("capsule shape not supported (yet)"), _T("Error"), MB_OK);
			break;
		}

	case NX_SHAPE_CONVEX:
		{
			if(m_proxyNode)
			{
				MaxMsgBox(NULL, _T("Error: convex shape proxy not supported (yet)"), _T("Error"), MB_OK);
				//d->meshData = MxUtils::nodeToNxConvexMesh(proxyMesh);
				//Matrix3 pose = nodePose * actorNode->PhysicsNodePoseTMInv;
				//d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			else
			{
				if(node->SimpleMesh.numFaces > 255)
				{
					MaxMsgBox(NULL, _T("Error: number of vertices in a convex shape should be less than 256"), _T("Error"), MB_OK);
					//warning/Error
				} else
				{

					BOOL needDel = FALSE;
					TriObject* tri = MxUtils::GetTriObjectFromNode(node->GetMaxNode(),0.f,needDel);
					if (tri)
					{
						int numVerts = tri->NumPoints();
						btConvexHullShape* convexHull = new btConvexHullShape();
						for (int i=0;i<numVerts;i++)
						{
							convexHull->addPoint(btVector3(tri->GetPoint(i).x,tri->GetPoint(i).y,tri->GetPoint(i).z));
						}
						shape = convexHull;
						if (needDel)
							delete tri;
					}
				}
				//d->meshData = MxUtils::nodeToNxConvexMesh(node->SimpleMesh);
				//Matrix3 pose = nodePose * actorNode->PhysicsNodePoseTMInv;
				//d->localPose = MxMathUtils::MaxMatrixToNx(pose);
			}
			break;
		}
	case 	NX_SHAPE_MESH:
		{

			BOOL needDel = FALSE;
			TriObject* tri = MxUtils::GetTriObjectFromNode(node->GetMaxNode(),0.f,needDel);

			if (tri)
			{
				int numVerts = tri->NumPoints();
				btTriangleMesh* meshInterface = new btTriangleMesh();
				Mesh& mesh = tri->GetMesh();

				for (int i=0;i<mesh.getNumFaces();i++)
				{
					Point3 p0=tri->GetPoint(mesh.faces[i].v[0]);
					Point3 p1=tri->GetPoint(mesh.faces[i].v[1]);
					Point3 p2=tri->GetPoint(mesh.faces[i].v[2]);

					meshInterface->addTriangle( btVector3(p0.x,p0.y,p0.z),btVector3(p1.x,p1.y,p1.z),btVector3(p2.x,p2.y,p2.z));
				}


				btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(meshInterface,true);
				//might need btGImpactMeshShape for moving objects
				shape = trimesh;
				if (needDel)
					delete tri;
				shape = trimesh;
			} else
			{
				MaxMsgBox(NULL, _T("Error: couldn't GetTriObjectFromNode"), _T("Error"), MB_OK);
			}

			break;
		}

	default:
		{
			MaxMsgBox(NULL, _T("unknown shape type"), _T("Error"), MB_OK);
		}
	};


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

	return shape;
}

NxActor* MxActor::createNxActor()
{
	if (m_bulletBody)
	{
		MaxMsgBox(NULL, _T("Error: body was already added to the dynamics world"), _T("Error"), MB_OK);
		return 0;
	}

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

	btAlignedObjectArray<btCollisionShape*> collisionShapes;

	for (int i = 0; i < shapes.size(); i++) 
	{
		INode* current = shapes[i];
		//DEBUG_F(" Debug adding shape node: %s\n", current->GetName());
		ccMaxNode* pn = ccMaxWorld::FindNode(current);
		assert(pn);
		btCollisionShape* collisionShape = createShape(actorDesc, pn, pActorNode);
		collisionShapes.push_back(collisionShape);
	}

	//create a rigid body and add it to the world
	if (collisionShapes.size())
	{
		btCollisionShape* collisionShape = 0;
		if (collisionShapes.size()==1)
		{
			collisionShape = collisionShapes[0];

		} else
		{
			//not yet
		}
		if (collisionShape)
		{
			btVector3 localInertia(0,0,0);
			if (actorDesc.mass)
			{
				collisionShape->calculateLocalInertia(actorDesc.mass,localInertia);
			}

			m_bulletBody = new btRigidBody(actorDesc.mass,0,collisionShape,localInertia);
			m_bulletBody->setUserPointer(this);

			

			syncGlobalPose();
			

			gDynamicsWorld->addRigidBody(m_bulletBody);
//			MaxMsgBox(NULL, _T("adding rigid body"), _T("Error"), MB_OK);

		}
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

void MxActor::syncGlobalPose()
{
	NxActorDesc&  actorDesc  = m_desc;
	ccMaxNode* pActorNode = ccMaxWorld::FindNode(m_node);
	actorDesc.globalPose = pActorNode->PhysicsNodePoseTM;

	btTransform worldTrans;
	max2Bullet( actorDesc.globalPose,worldTrans);

	btTransform localTrans;
	max2Bullet(actorDesc.localPose,localTrans);

	worldTrans = worldTrans*localTrans;
	m_bulletBody->setWorldTransform(worldTrans);

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
	// update pose to max node
	if(getInteractivity() != RB_DYNAMIC)
		return;

	// update rigid body's last pose
	//ccMaxNode* pn = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);
	Matrix3 globalPose;
	globalPose.IdentityMatrix();

	if (m_bulletBody)
	{
		NxActorDesc&  actorDesc  = m_desc;

		btTransform localTrans;
		max2Bullet(actorDesc.localPose,localTrans);



		btTransform maxWorldTrans = m_bulletBody->getWorldTransform()*localTrans.inverse();

		bullet2Max(maxWorldTrans,globalPose);
		//@todo: need correction for boxes (off center etc)

		maxNodeActor->SetSimulatedPose(globalPose);
		Matrix3 poseTM = maxNodeActor->GetCurrentTM();
		SaveLastPose(poseTM);
	}
}

