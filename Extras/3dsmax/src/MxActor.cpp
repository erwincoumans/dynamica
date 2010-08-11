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
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "PxFunctions.h"

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
		{
			for (int i=0;i<m_bulletBody->getNumConstraintRefs();i++)
			{
				gDynamicsWorld->removeConstraint(m_bulletBody->getConstraintRef(i));
			}
			gDynamicsWorld->removeRigidBody(m_bulletBody);
		}
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

	actorDesc.localPose = node->PhysicsNodePoseTM * actorNode->PhysicsNodePoseTMInv;

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
			Matrix3 offset;
			offset.IdentityMatrix();
			offset.SetTrans(2,node->PrimaryShapePara.BoxDimension.z());

			actorDesc.localPose = actorDesc.localPose * offset;

			shape = new btBoxShape(node->PrimaryShapePara.BoxDimension.absolute());

			break;
		}
	case NX_SHAPE_CAPSULE:
		{

			char bla[1024];
			sprintf(bla,"capsule not properly supported yet, radius=%f,height=%f",node->PrimaryShapePara.Radius,node->PrimaryShapePara.Height);
			MaxMsgBox(NULL, _T(bla), _T("Error"), MB_OK);

			shape = new btCapsuleShape(node->PrimaryShapePara.Radius,node->PrimaryShapePara.Height);
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
						
						//for center of mass computation, simplify and assume mass is at the vertices
						btCompoundShape* compound = new btCompoundShape();
						btSphereShape sphere(0.1);
						btTransform tr;
						tr.setIdentity();
						btAlignedObjectArray<btScalar> masses;
						btScalar childMass = actorDesc.mass/(btScalar)numVerts;


						for (int i=0;i<numVerts;i++)
						{
							btVector3 pt(tri->GetPoint(i).x,tri->GetPoint(i).y,tri->GetPoint(i).z);
							convexHull->addPoint(pt);
							tr.setOrigin(pt);
							compound->addChildShape(tr,&sphere);
							masses.push_back(childMass);
						}
						
						btTransform principal;
						btVector3 inertia;
						compound->calculatePrincipalAxisTransform(&masses[0],principal,inertia);

						
						delete compound;

						btTransform principalInv = principal.inverse();
						compound = new btCompoundShape();
						compound->addChildShape(principalInv,convexHull);
						shape = compound;

						Matrix3 offset;
						bullet2Max(principal,offset);
						actorDesc.localPose = actorDesc.localPose * offset;


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

				if (mesh.getNumFaces()>0)
				{
					for (int i=0;i<mesh.getNumFaces();i++)
					{
						Point3 p0=tri->GetPoint(mesh.faces[i].v[0]);
						Point3 p1=tri->GetPoint(mesh.faces[i].v[1]);
						Point3 p2=tri->GetPoint(mesh.faces[i].v[2]);

						meshInterface->addTriangle( btVector3(p0.x,p0.y,p0.z),btVector3(p1.x,p1.y,p1.z),btVector3(p2.x,p2.y,p2.z));
					}

					
					if (actorDesc.mass>0)
					{
//						MaxMsgBox(NULL, _T("btGImpactMeshShape"), _T("Error"), MB_OK);

						btGImpactMeshShape* trimesh = new btGImpactMeshShape(meshInterface);
						shape = trimesh;

					} else
					{

//						MaxMsgBox(NULL, _T("btBvhTriangleMeshShape"), _T("Error"), MB_OK);
						btBvhTriangleMeshShape* trimesh = new btBvhTriangleMeshShape(meshInterface,true);
						shape = trimesh;
					}
					
					
				} else
				{
					MaxMsgBox(NULL, _T("Error: no faces"), _T("Error"), MB_OK);
				}
				if (needDel)
					delete tri;
				
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
	{
		baseNode = maxNodeProxy;
	}

	btAlignedObjectArray<btCollisionShape*> collisionShapes;
	btAlignedObjectArray<btTransform> localTransforms;



	for (int i = 0; i < shapes.size(); i++) 
	{
		INode* current = shapes[i];
		//DEBUG_F(" Debug adding shape node: %s\n", current->GetName());
		ccMaxNode* pn = ccMaxWorld::FindNode(current);
		assert(pn);
		btCollisionShape* collisionShape = createShape(actorDesc, pn, pActorNode);
		collisionShapes.push_back(collisionShape);

		btTransform localTrans;
		max2Bullet(actorDesc.localPose,localTrans);
		localTransforms.push_back(localTrans);
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
			btCompoundShape* compound = new btCompoundShape();
			for (int i=0;i<collisionShapes.size();i++)
			{
				compound->addChildShape(localTransforms[i],collisionShapes[i]);
			}
			//clear local pose
			actorDesc.localPose.IdentityMatrix();
			collisionShape = compound;
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

			btScalar restitution;
			if (MxUserPropUtils::GetUserPropFloat(m_node, "Restitution", restitution))
			{
				m_bulletBody->setRestitution(restitution);
			}
			btScalar staticFriction = 0.f;

			//take the maximum for now
			MxUserPropUtils::GetUserPropFloat(m_node, "StaticFriction", staticFriction);
			
			btScalar friction = 0.f;
			if (MxUserPropUtils::GetUserPropFloat(m_node, "Friction", friction))
			{
				if (staticFriction>friction)
					friction = staticFriction;

				m_bulletBody->setFriction(friction);
			}


//			char bla[1024];
//			sprintf(bla,"restitution=%f",restitution);
//			MaxMsgBox(NULL, _T(bla), _T("Error"), MB_OK);

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

	if (!(pActorNode->GetMaxNode()))
		return;

	const TimeValue  t  = ccMaxWorld::MaxTime();
	actorDesc.globalPose = pActorNode->GetMaxNode()->GetNodeTM(t);

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

