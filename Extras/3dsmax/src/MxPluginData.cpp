
#include <NxPhysics.h>
#include <NxCooking.h>

#include <max.h>
#include <SimpObj.h>

#include "PxPlugin.h"
#include "MxUtils.h"
#include "MxUserOutputStream.h"
#include "MxPluginData.h"
#include "MxFluid.h"
#include "MxFluidEmitter.h"
#include "MxMesh.h"
#include "MxJoint.h"
//#include "MxShape.h"
#include "MxMaterial.h"
#include "MxCloth.h"
#include "MxActor.h"
#include "MxSoftBody.h"
#include "MxForceField.h"

#include "PxFunctions.h"
#include "MxContactReport.h"

MxPluginData* MxPluginData::m_instance = NULL;
bool MxPluginData::m_hwPhysX = false;

CharStream* MxPluginData::m_outputStream = NULL;

#include "MxDebugVisualizer.h"
extern MxDebugVisualizer* gDebugVisualizer;

MxObject* MxPluginData::getObjectFromId(NxU32 id) 
{
	if (m_instance == NULL) return NULL;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		if (objects[i]->getID() == id) return objects[i];
	}
	return NULL;
}

MxObject* MxPluginData::getObjectFromNode(INode* node)
{
	if (m_instance == NULL) return NULL;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		if (objects[i]->getNode() == node) return objects[i];
	}
	return NULL;
}

void MxPluginData::getObjectsFromNode(INode* node, NxArray<MxObject*>& destination)
{
	if (m_instance == NULL) return;
	if (node == NULL) return;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		if (objects[i]->getNode() == node) destination.pushBack(objects[i]);
	}	
}

void MxPluginData::getObjectsFromName(const char* name, NxArray<MxObject*>& destination)
{
	if (m_instance == NULL) return;
	if (name == NULL) return;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		if (strcmp(name, objects[i]->getName().data()) == 0) destination.pushBack(objects[i]);
	}
}

void MxPluginData::getAllObjects(NxArray<MxObject*>& destination)
{
	if (m_instance == NULL) return;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	destination.reserve(objects.size());
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		destination.pushBack(objects[i]);
	}
}

//returns true if the deleting had side-effects
//Only deletes "top" objects, i.e. not shapes, meshes, materials
bool MxPluginData::releaseObject(MxObject* object, bool* sideEffects)
{
	if (sideEffects != NULL) *sideEffects = false;
	if (object == NULL) return false;

	if (object->isActor()) return releaseActor(object->isActor(), sideEffects);
	if (object->isCloth()) return releaseCloth(object->isCloth());
	if (object->isCompartment()) return releaseCompartment(object->isCompartment());
	if (object->isFluid()) return releaseFluid(object->isFluid(), sideEffects);
	if (object->isFluidEmitter()) return releaseFluidEmitter(object->isFluidEmitter());
	if (object->isJoint()) return releaseJoint(object->isJoint());
#ifdef PXPLUGIN_SOFTBODY
	if (object->isSoftBody()) return releaseSoftBody(object->isSoftBody());
#endif
	if (object->isForceField()) return releaseForceField(object->isForceField());

	if (gCurrentstream) gCurrentstream->printf("Can not release object \"%s\" of unknown type.\n", object->getName());
	return false;
}

//creates the rigid body, but no shapes
//have to check so that no joints are referencing the rigid body
MxActor* MxPluginData::createActor(INode* node, bool createShapes)
{
	if (createShapes)
	{
		if (gCurrentstream) gCurrentstream->printf("Called MxPluginData::createActor with \"createShapes\" flag, which is not yet implemented.\n");
		assert(false); //not implemented
		return NULL;
	}

	if (node == NULL) return false;
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	//check so that this node has not already been added, should it be possible to do that?
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create an actor of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	MxActor* actor = new MxActor(node->GetName(), node);
	m_instance->getObjectArray().pushBack(actor);
	return actor;
}

//releases the rigid body (including the shapes, which are not released by the MxActor object)
bool MxPluginData::releaseActor(MxActor* actor, bool* sideEffects)
{
	if (sideEffects != NULL) *sideEffects = false;
	if (actor == NULL) return false;

	//have to go through the list of joints
	NxArray<MxObject*> joints;
	m_instance->getAllObjectsOfType(MX_OBJECTTYPE_JOINT, joints);
	for (NxU32 i = 0; i < joints.size(); i++)
	{
		MxJoint* joint = joints[i]->isJoint();
		if (joint != NULL)
		{
			if (joint->getActor0() == actor || joint->getActor1() == actor)
			{
				if (gCurrentstream) gCurrentstream->printf("Warning: removing actor \"%s\" also required the joint \"%s\" to be removed.\n", actor->getName(), joint->getName());
				releaseJoint(joint); //TODO: should the whole joint object be removed, or should it be enough with only removing the reference?
				if (sideEffects != NULL) *sideEffects = true;
			}
		}
	}

	//release all shapes in the actor
	//NxU32 shapeCount = actor->m_shapes.size();
	NxActor* pa = actor->getNxActor();
	NxU32 shapeCount = pa? 1 : 0;
	actor->releaseAllShapes();
	if (sideEffects != NULL && shapeCount > 0) *sideEffects = true;
	
	m_instance->removeObjectFromArray(actor);
	delete actor;
	return true;
}


class PxCCDSkeleton
{
public:
	static NxCCDSkeleton* createSkeleton(NxConvexShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxSphereShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxBoxShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxCapsuleShapeDesc* shape, PX_CCD_SKELETON type);
	static NxCCDSkeleton* createSkeleton(NxTriangleMeshShapeDesc* shape, PX_CCD_SKELETON type);
	static float zoomScale;
};
float PxCCDSkeleton::zoomScale = 0.9f;

NxCCDSkeleton* PxCCDSkeleton::createSkeleton(NxConvexShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;

	NxConvexMeshDesc mesh;
	shape->meshData->saveToDesc(mesh);
	
	PxSimpleMesh sm;
	sm.buildFrom(mesh);

	MxUtils::scaleMesh(sm, PxCCDSkeleton::zoomScale);
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
	ret = MxPluginData::getPhysicsSDKStatic()->createCCDSkeleton(pointSkeleton);
	hull.release();
	return ret;
}

NxCCDSkeleton* PxCCDSkeleton::createSkeleton(NxSphereShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	NxReal radius = shape->radius * PxCCDSkeleton::zoomScale;
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
	ret = MxPluginData::getPhysicsSDKStatic()->createCCDSkeleton(stm);
	return ret;
}

NxCCDSkeleton* PxCCDSkeleton::createSkeleton(NxBoxShapeDesc* shape, PX_CCD_SKELETON type)
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
	NxVec3 size = shape->dimensions * PxCCDSkeleton::zoomScale;
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
	ret = MxPluginData::getPhysicsSDKStatic()->createCCDSkeleton(stm);
	return ret;
}

NxCCDSkeleton* PxCCDSkeleton::createSkeleton(NxCapsuleShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	NxReal radius = shape->radius * PxCCDSkeleton::zoomScale;
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
	ret = MxPluginData::getPhysicsSDKStatic()->createCCDSkeleton(stm);
	return ret;
}

NxCCDSkeleton* PxCCDSkeleton::createSkeleton(NxTriangleMeshShapeDesc* shape, PX_CCD_SKELETON type)
{
	NxCCDSkeleton* ret = NULL;
	return ret;
}

//MxShape* MxPluginData::createShape(INode* node, NxShapeType type, bool needCCD)
//{
//	if (node == NULL) return NULL;
//	if (m_instance == NULL) return NULL;
//	if (m_instance->getScene() == NULL) return NULL;
//
//	MxShape* resultShape = NULL;
//
//	TimeValue t = GetCOREInterface()->GetTime();
//
//	if (type == NX_SHAPE_MESH)
//	{
//		//need some way of telling the plugin to create a cloth mesh / (softbody mesh?)
//		MxMesh* mesh = m_instance->createMesh(node, MX_MESHTYPE_TRIMESH);
//		if (mesh != NULL)
//		{
//			if (mesh->getNxTriangleMesh() != NULL)
//			{
//				NxTriangleMeshShapeDesc* nxShapeDesc = new NxTriangleMeshShapeDesc();
//				nxShapeDesc->meshData = mesh->getNxTriangleMesh();
//
//				MxShape* shape = new MxShape(node->GetName(), node, nxShapeDesc, mesh);
//				m_instance->m_shapes.pushBack(shape);
//				resultShape = shape;
//			} else {
//				m_instance->releaseMesh(mesh);
//				return NULL;
//			}
//		}
//	} else if (type == NX_SHAPE_CONVEX)
//	{
//		MxMesh* mesh = m_instance->createMesh(node, MX_MESHTYPE_CONVEX);
//		if (mesh != NULL)
//		{
//			if (mesh->getNxConvexMesh() != NULL)
//			{
//				NxConvexShapeDesc* nxShapeDesc = new NxConvexShapeDesc();
//				nxShapeDesc->meshData = mesh->getNxConvexMesh();
//
//				if(needCCD)
//				{
//					nxShapeDesc->ccdSkeleton = PxCCDSkeleton::createSkeleton(nxShapeDesc, CONVEX_SKELETON);
//				}
//				MxShape* shape = new MxShape(node->GetName(), node, nxShapeDesc, mesh);
//				m_instance->m_shapes.pushBack(shape);
//				resultShape = shape;
//			} else {
//				m_instance->releaseMesh(mesh);
//				return NULL;
//			}
//		}
//	} else if (type == NX_SHAPE_SPHERE)
//	{
//		Object* obj = node->EvalWorldState(t).obj;
//		if (obj == NULL) 
//			return NULL;
//
//		Class_ID id = obj->ClassID();
//		if(id != Class_ID(SPHERE_CLASS_ID, 0)) 
//			return NULL;
//
//		SimpleObject* so = (SimpleObject*)obj;
//		NxSphereShapeDesc* sphereDesc = new NxSphereShapeDesc();
//		so->pblock->GetValue(SPHERE_RADIUS, 0, sphereDesc->radius, FOREVER);
//		if(needCCD)
//		{
//			sphereDesc->ccdSkeleton = PxCCDSkeleton::createSkeleton(sphereDesc, CONVEX_SKELETON);
//		}
//
//		MxShape* shape = new MxShape(node->GetName(), node, sphereDesc, NULL);
//		m_instance->m_shapes.pushBack(shape);
//		resultShape = shape;
//	} else if (type == NX_SHAPE_BOX)
//	{
//		Object* obj = node->EvalWorldState(t).obj;
//		if (obj == NULL) 
//			return NULL;
//
//		Class_ID id = obj->ClassID();
//		if(id != Class_ID(BOXOBJ_CLASS_ID, 0)) 
//			return NULL;
//
//		SimpleObject* so = (SimpleObject*)obj;
//		NxBoxShapeDesc* boxDesc = new NxBoxShapeDesc();
//		so->pblock->GetValue(BOXOBJ_WIDTH , 0, boxDesc->dimensions.x, FOREVER);
//		so->pblock->GetValue(BOXOBJ_LENGTH, 0, boxDesc->dimensions.y, FOREVER);
//		so->pblock->GetValue(BOXOBJ_HEIGHT, 0, boxDesc->dimensions.z, FOREVER);
//		boxDesc->localPose.t.z = boxDesc->dimensions.z * 0.5f; //adjust for difference in pivot points between 3ds max and PhysX
//		for (int i = 0; i < 3; i++) 
//			boxDesc->dimensions[i] = fabsf(boxDesc->dimensions[i])*0.5f; //PhysX stores half the actual size as the dimension (extents) of the box
//		
//		if(needCCD)
//		{
//			boxDesc->ccdSkeleton = PxCCDSkeleton::createSkeleton(boxDesc, CONVEX_SKELETON);
//		}
//
//		MxShape* shape = new MxShape(node->GetName(), node, boxDesc, NULL);
//		m_instance->m_shapes.pushBack(shape);
//		resultShape = shape;
//
//		/*/ test
//		Matrix3 m1 = node->GetNodeTM(t);
//		NxMat34 p1 = MxMathUtils::MaxMatrixToNx(m1);
//		NxMat34 p2 = p1;
//		NxReal d1 = p1.M.determinant();
//		NxQuat  q1;
//		p1.M.toQuat(q1);
//		p1.M.fromQuat(q1);
//		NxReal d2 = p1.M.determinant();
//		NxMat33 p1Inv;
//		p1.M.getInverse(p1Inv);
//		NxMat33 r1 = p2.M * p1Inv;
//		NxVec3 pos = p1.t;
//		/*/
//	} else if (type == NX_SHAPE_CAPSULE)
//	{
//		Object* obj = node->EvalWorldState(t).obj;
//		if (obj == NULL) 
//			return NULL;
//
//		Class_ID id = obj->ClassID();
//		if(id != CAPS_CLASS_ID) 
//			return NULL;
//
//		SimpleObject* so = (SimpleObject*) obj;
//		int centersflag = 0;
//		NxCapsuleShapeDesc* capsuleDesc = new NxCapsuleShapeDesc();
//		so->pblock->GetValue(CAPS_RADIUS , 0, capsuleDesc->radius, FOREVER);
//		so->pblock->GetValue(CAPS_HEIGHT , 0, capsuleDesc->height, FOREVER);
//		so->pblock->GetValue(CAPS_CENTERS, 0, centersflag, FOREVER);
//		capsuleDesc->localPose.M = NxMat33(NxVec3(1,0,0),NxVec3(0,0,1),NxVec3(0,-1,0));
//		if(!centersflag) capsuleDesc->height -= capsuleDesc->radius*2.0f; //there are some different ways in which you can specify a capsule in 3ds max, adjust length if "center" mode is not used
//		capsuleDesc->localPose.t.z = capsuleDesc->height/2 + capsuleDesc->radius; //adjust for difference in pivot points between 3ds max and PhysX
//		
//		if(needCCD)
//		{
//			capsuleDesc->ccdSkeleton = PxCCDSkeleton::createSkeleton(capsuleDesc, CONVEX_SKELETON);
//		}
//
//		MxShape* shape = new MxShape(node->GetName(), node, capsuleDesc, NULL);
//		m_instance->m_shapes.pushBack(shape);
//		resultShape = shape;
//	} else
//	{
//		//Unknown / unhandled geometry type
//		if (gCurrentstream) gCurrentstream->printf("Unable to create a shape of node \"%s\", unknown shape type: %d.\n", node->GetName(), type);
//	}
//
//	if (resultShape != NULL)
//	{
//		MxMaterial* mxMaterial = createMaterial(node, resultShape);
//	}
//
//	return resultShape;
//}
//
////also decreases the refcount on any meshes that the shape is holding on to
//bool MxPluginData::releaseShape(MxShape* shape)
//{
//	if (shape == NULL) return false;
//
//	//remove the shape from the actor if it is still in there
//	if (shape->getActor() != NULL)
//	{
//		MxActor* actor = shape->getActor();
//		actor->removeShape(shape);
//	}
//
//	//release (decrease reference count) on any mesh object the shape is referencing
//	if (shape->getMxMesh() != NULL)
//	{
//		MxMesh* mxMesh = shape->getMxMesh();
//		shape->m_mxMesh = NULL;
//		m_instance->releaseMesh(mxMesh);
//	}
//
//	shape->setMaterial(NULL); //releases the shape material
//
//	m_instance->removeObjectFromArray(shape);
//	delete shape;
//
//	return true;
//}

MxJoint* MxPluginData::createUninitializedJoint(INode* node)
{
	if (node == NULL) return NULL;
	if (m_instance == NULL) return NULL;

	MxJoint* mxJoint = new MxJoint(node->GetName(), node, NULL, NULL);
	m_instance->getObjectArray().pushBack(mxJoint);

	return mxJoint;
}

MxJoint* MxPluginData::createJoint(INode* node)
{
	if (node == NULL) return NULL;
	if (m_instance == NULL) return NULL;

	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState(t);
	if (os.obj == NULL) return 0;

	//os.obj->GetParamBlockByID(0) is the first parameter block
	IParamBlock2* pb = os.obj->GetParamBlockByID(0);

	INode* node0 = MxParamUtils::GetINodeParam(pb, t, "body0", NULL);
	INode* node1 = MxParamUtils::GetINodeParam(pb, t, "body1", NULL);
	if (node0 == NULL && node1 == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unable to add the joint \"%s\", both actor references can't be NULL.\n", node->GetName());
		return 0;
	}
	MxActor* mxActor0 = MxUtils::GetActorFromNode(node0);
	MxActor* mxActor1 = MxUtils::GetActorFromNode(node1);
	if (mxActor0 == NULL && mxActor1 == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unable to add the joint \"%s\", both actor references can't be NULL.\n", node->GetName());
		return 0;
	}
	if (mxActor0 != NULL) mxActor0->incRef();
	if (mxActor1 != NULL) mxActor1->incRef();

	MxJoint* mxJoint = new MxJoint(node->GetName(), node, mxActor0, mxActor1); //for some reason the old plugin code swaps these, all the calculations are done with this order, so need to keep it that way (don't want to play around with the joint properties code at this moment)
	m_instance->getObjectArray().pushBack(mxJoint);

	return mxJoint;
}

bool MxPluginData::releaseJoint(MxJoint* joint)
{
	if (joint == NULL) return false;

	if (joint->getActor0() != NULL) joint->getActor0()->decRef();
	if (joint->getActor1() != NULL) joint->getActor1()->decRef();
	m_instance->removeObjectFromArray(joint);
	delete joint;
	return true;
}
//
//MxMaterial* MxPluginData::createMaterial(INode* node, MxShape* shape)
//{
//	if (node == NULL) return NULL;
//	if (m_instance == NULL) return NULL;
//
//	NxScene* scene = MxPluginData::getSceneStatic();
//	if (scene == NULL)
//	{
//		if (gCurrentstream) gCurrentstream->printf("Warning: can't create NxMaterial, there is no scene object\n");
//		if (shape != NULL) shape->setMaterial(NULL);
//		return NULL;
//	}
//
//	MxMaterialType materialType = MX_MATERIALTYPE_USER;
//	int tempMaterialType = -1;
//	if (node->GetUserPropInt("MaterialType", tempMaterialType))
//	{
//		if (tempMaterialType != -1){
//			materialType = (MxMaterialType)tempMaterialType;
//		}
//	}
//	
//	bool materialHandled = false;
//
//	//TODO: add "Preset material"
//
//	if (materialType == MX_MATERIALTYPE_USER) //use material settings from node
//	{
//		NxMaterialDesc mdesc;
//		mdesc.restitution = mdesc.dynamicFriction = mdesc.staticFriction = -1.0f;
//		if (MxUserPropUtils::GetUserPropFloat(node,"Restitution",mdesc.restitution) == 0)
//			MxUserPropUtils::GetUserPropFloat(node,"Ellasticity",mdesc.restitution);
//		MxUserPropUtils::GetUserPropFloat(node,"StaticFriction",mdesc.staticFriction);
//		MxUserPropUtils::GetUserPropFloat(node,"Friction",mdesc.dynamicFriction);
//
//		//invalid material? -> skip out and use the default material instead
//		if (mdesc.restitution != -1.0f && mdesc.dynamicFriction != -1.0f && mdesc.staticFriction != -1.0f)
//		{
//			//go through scene and check if there is a material that fits this one (and was created as a "materialtype 2" material)
//			NxArray<MxObject*> materials;
//			m_instance->getAllObjectsOfType(MX_OBJECTTYPE_MATERIAL, materials);
//			for (NxU32 i = 0; i < materials.size(); i++)
//			{
//				MxMaterial* mxMaterial = materials[i]->isMaterial();
//				if (mxMaterial != NULL)
//				{
//					if ( (mxMaterial->getMaterialType() == MX_MATERIALTYPE_USER || mxMaterial->getMaterialType() == MX_MATERIALTYPE_INVALID) &&
//						mxMaterial->getRestitution() == mdesc.restitution &&
//						mxMaterial->getStaticFriction() == mdesc.staticFriction &&
//						mxMaterial->getDynamicFriction() == mdesc.dynamicFriction)
//					{ //Found a matching material, use it
//						mxMaterial->incRef();
//						if (shape != NULL) shape->setMaterial(mxMaterial);
//						return mxMaterial;
//					}
//				}
//			}
//
//			//if we get here, then there was no matching material -> create a new one
//			MxMaterial* mxMaterial = new MxMaterial("PxPropertyMaterial", node, mdesc, MX_MATERIALTYPE_USER);
//			mxMaterial->incRef();
//			if (shape != NULL) shape->setMaterial(mxMaterial);
//			return mxMaterial;
//		}
//		if (gCurrentstream) gCurrentstream->printf("Warning: The node \"%s\" has invalid material properties. Using default material.\n", node->GetName());
//	}
//
//	if (materialType == MX_MATERIALTYPE_3DS) //use 3ds max material
//	{
//		Mtl* nodeMaterial = node->GetMtl();
//		if (nodeMaterial != NULL) 
//		{
//			if (nodeMaterial->ClassID() == Class_ID(MULTI_CLASS_ID, 0) || nodeMaterial->IsSubClassOf(Class_ID(MULTI_CLASS_ID, 0)) || nodeMaterial->SuperClassID() == MULTI_CLASS_ID) 
//			{
//				//use the first sub-material that exists
//				//TODO: should return one that is actually used by the geometry
//				Mtl* oldNodeMaterial = nodeMaterial;
//				nodeMaterial = NULL;
//				for (int i = 0; i < oldNodeMaterial->NumSubMtls(); i++) 
//				{
//					Mtl* temp = oldNodeMaterial->GetSubMtl(i);
//					if (temp != NULL) 
//					{
//						nodeMaterial = temp;
//						break;
//					}
//				}
//			}
//		}
//
//		if (nodeMaterial != NULL)
//		{
//			NxMaterialDesc mdesc;
//			mdesc.restitution = nodeMaterial->GetDynamicsProperty(0, 0, DYN_BOUNCE);
//			mdesc.staticFriction = nodeMaterial->GetDynamicsProperty(0, 0, DYN_STATIC_FRICTION);
//			mdesc.dynamicFriction = nodeMaterial->GetDynamicsProperty(0, 0, DYN_SLIDING_FRICTION);
//
//			//first see if the same material has been used before
//			NxArray<MxObject*> materials;
//			getObjectsFromName(nodeMaterial->GetName(), materials);
//			for (NxU32 i = 0; i < materials.size(); i++)
//			{
//				MxMaterial* mxMaterial = materials[i]->isMaterial();
//				if (mxMaterial != NULL)
//				{
//					if ( (mxMaterial->getMaterialType() == MX_MATERIALTYPE_3DS) &&
//						mxMaterial->getRestitution() == mdesc.restitution &&
//						mxMaterial->getStaticFriction() == mdesc.staticFriction &&
//						mxMaterial->getDynamicFriction() == mdesc.dynamicFriction)
//					{ //Found a matching material, use it
//						mxMaterial->incRef();
//						if (shape != NULL) shape->setMaterial(mxMaterial);
//						return mxMaterial;
//					}
//				}
//			}
//
//			MxMaterial* mxMaterial = new MxMaterial(nodeMaterial->GetName(), node, mdesc, MX_MATERIALTYPE_3DS);
//			mxMaterial->incRef();
//			if (shape != NULL) shape->setMaterial(mxMaterial);
//			return mxMaterial;
//		}
//
//		//skip out and use the default material
//		if (gCurrentstream) gCurrentstream->printf("Warning: The node \"%s\" was specified to use 3ds max material for dynamics properties, but has no material. Using default material.\n", node->GetName());
//	}
//
//	//use PhysX default material
//	if (shape != NULL) shape->setMaterial(NULL);
//	return NULL;
//}
//
//bool MxPluginData::releaseMaterial(MxMaterial* material, bool* sideEffects)
//{
//	if (sideEffects != NULL) *sideEffects = false;
//	if (material == NULL) return false;
//	if (m_instance == NULL) return false;
//
//	//NxMaterialIndex values are not affected by the removal of a material, 
//	// the released material index is left as a "free slot" for creation
//	// of new materials.
//	material->decRef();
//	if (material->getRefCount() == 0)
//	{
//		m_instance->removeObjectFromArray(material);
//		delete material;
//	}
//	return true;
//}


MxFluid* MxPluginData::createFluid(INode* node)
{
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;
	
	//check so that this node has not already been added
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create a fluid of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	MxFluid* fluid = new MxFluid(node->GetName(), node);
	if (fluid != NULL)
	{
		if (fluid->getNxFluid() != NULL)
		{
			//add the fluid to the list and return it
			m_instance->getObjectArray().pushBack(fluid);
			return fluid;
		} else {
			//need to remove the fluid object, since the fluid could not be created
			delete fluid;
		}
	}
	return NULL;
}

bool MxPluginData::releaseFluid(MxFluid* fluid, bool* sideEffects)
{
	//the caller should make sure that there are no emitters referencing the fluid before releasing it

	if (sideEffects != NULL) *sideEffects = false;
	if (fluid == NULL) return false;
	if (m_instance == NULL) return false;

	NxArray<MxObject*> emitters;
	m_instance->getAllObjectsOfType(MX_OBJECTTYPE_FLUIDEMITTER, emitters);
	for (NxU32 i = 0; i < emitters.size(); i++)
	{
		MxFluidEmitter* emitter = emitters[i]->isFluidEmitter();
		if (emitter != NULL)
		{
			if (emitter->getFluid() == fluid)
			{
				if (gCurrentstream) gCurrentstream->printf("Warning: removing fluid \"%s\" also required the emitter \"%s\" to be removed.\n", fluid->getName(), emitter->getName());
				if (sideEffects != NULL) *sideEffects = true;
				releaseFluidEmitter(emitter);
			}
		}
	}

	//the fluid object itself will have to release the NxFluid
	m_instance->removeObjectFromArray(fluid);
	delete fluid;
	return true;
}

MxFluidEmitter* MxPluginData::createFluidEmitter(INode* node)
{
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	//check so that this node has not already been added
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create a fluid emitter of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	//first, find the fluid (must attach to a fluid)
	TimeValue t = GetCOREInterface()->GetTime();

	ObjectState osEmitter  = node->EvalWorldState(t); 
	if (osEmitter.obj == NULL) {
		if (gCurrentstream) gCurrentstream->printf("Unable to process fluid emitter \"%s\" - no worldstate.\n", node->GetName());
		return NULL;
	}
	Class_ID temptemp = osEmitter.obj->ClassID();
	if (osEmitter.obj->ClassID() != PXFLUIDEMITTER_CLASSID) {
		if (gCurrentstream) gCurrentstream->printf("Unable to process object \"%s\" as a fluid emitter.", node->GetName());
		return NULL;
	}
	IParamBlock2* pb = osEmitter.obj->GetParamBlockByID(0);
	if (pb == NULL) {
		if (gCurrentstream) gCurrentstream->printf("Unable to find parameter block for fluid emitter \"%s\".\n", node->GetName());
		return NULL;
	}
	INode* fluidNode = MxParamUtils::GetINodeParam(pb, t, "Fluid", NULL);
	MxFluid* mxFluid = NULL;
	if (fluidNode != NULL)
	{
		ObjectState os = fluidNode->EvalWorldState(t);
		if (os.obj)
		{
			if (os.obj->ClassID() == PXFLUID_CLASSID)
			{
				//it is a fluid, now make sure that it has been added
				MxObject* mxFluidObj = MxUtils::GetFirstObjectOfTypeFromNode(fluidNode, MX_OBJECTTYPE_FLUID);
				if (mxFluidObj == NULL || !mxFluidObj->isFluid())
				{
					if (gCurrentstream) gCurrentstream->printf("Unable to create fluid emitter \"%s\", the fluid it is attached to (\"%s\")must first be added to the simulation.\n", node->GetName(), fluidNode->GetName());
					return NULL;
				} else {
					mxFluid = mxFluidObj->isFluid();
				}
			}
		}
	}
	if (mxFluid == NULL) 
	{
		if (gCurrentstream) gCurrentstream->printf("Unable to create the fluid emitter \"%s\", it is not attached to a Fluid object\n", node->GetName());
		return NULL;
	}

	MxFluidEmitter* emitter = new MxFluidEmitter(node->GetName(), node, mxFluid);
	if (emitter != NULL)
	{
		if (emitter->getNxEmitter() != NULL)
		{
			m_instance->getObjectArray().pushBack(emitter);
			return emitter;
		} else {
			delete emitter;
			emitter = NULL;
		}
	}
	return NULL;
}

bool MxPluginData::releaseFluidEmitter(MxFluidEmitter* emitter)
{
	if (emitter == NULL) return false;
	if (m_instance == NULL) return false;

	//the emitter object itself will have to release the NxFluidEmitter
	m_instance->removeObjectFromArray(emitter);
	delete emitter;
	return true;
}

void MxPluginData::getAllEmitters(NxArray<MxFluidEmitter*>& destination)
{
	if (m_instance == NULL) return;

	NxArray<MxObject*> objects;
	m_instance->getAllObjectsOfType(MX_OBJECTTYPE_FLUIDEMITTER, objects);

	for (NxU32 i = 0; i < objects.size(); i++)
	{
		MxFluidEmitter* emitter = objects[i]->isFluidEmitter();
		if (emitter) destination.pushBack(emitter);
	}
}


MxCloth* MxPluginData::createCloth(INode* node, bool isMetalCloth)
{
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	//check so that this node has not already been added
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create a cloth of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	MxCloth* cloth = new MxCloth(node->GetName(), node, isMetalCloth);
	if (cloth != NULL)
	{
		if (cloth->getNxCloth() != NULL)
		{
			//add the cloth to the list and return it
			m_instance->getObjectArray().pushBack(cloth);
			return cloth;
		} else {
			//need to remove the cloth object, since the cloth could not be created
			delete cloth;
		}
	}
	return NULL;
}

bool MxPluginData::releaseCloth(MxCloth* cloth)
{
	if (cloth == NULL) return false;
	if (m_instance == NULL) return false;

	//the cloth object itself will have to release the NxCloth
	m_instance->removeObjectFromArray(cloth);
	delete cloth;
	return true;
}

MxForceField* MxPluginData::createForceField(INode* node)
{
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	//check so that this node has not already been added
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create a ForceField of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	MxForceField* mx = new MxForceField(node->GetName(), node);
	if (mx != NULL)
	{
		if (mx->getNxForceField() != NULL)
		{
			//add the SoftBody to the list and return it
			m_instance->getObjectArray().pushBack(mx);
			return mx;
		} else {
			//need to remove the SoftBody object, since the NxSoftBody could not be created
			delete mx;
		}
	}
	return NULL;
}

bool MxPluginData::releaseForceField(MxForceField* forceField)
{
	if (forceField == NULL) return false;
	if (m_instance == NULL) return false;

	//the cloth object itself will have to release the NxCloth
	m_instance->removeObjectFromArray(forceField);
	delete forceField;
	return true;
}

MxSoftBody* MxPluginData::createSoftBody(INode* node)
{
#ifdef PXPLUGIN_SOFTBODY

	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	//check so that this node has not already been added
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create a softbody of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	MxSoftBody* softbody = new MxSoftBody(node->GetName(), node);
	if (softbody != NULL)
	{
		if (softbody->getNxSoftBody() != NULL)
		{
			//add the SoftBody to the list and return it
			m_instance->getObjectArray().pushBack(softbody);
			return softbody;
		} else {
			//need to remove the SoftBody object, since the NxSoftBody could not be created
			delete softbody;
		}
	}
	return NULL;
#else
	return NULL;
#endif
}

bool MxPluginData::releaseSoftBody(MxSoftBody* softbody)
{
#ifdef PXPLUGIN_SOFTBODY
	if (softbody == NULL) return false;
	if (m_instance == NULL) return false;

	//the SoftBody object itself will have to release the NxSoftBody
	m_instance->removeObjectFromArray(softbody);
	delete softbody;
	return true;
#else
	return false;
#endif
}

MxCompartment* MxPluginData::createCompartment(NxCompartmentType type, NxU32 compartmentID)
{
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	NxArray<MxObject*> objects;
	m_instance->getAllObjectsOfType(MX_OBJECTTYPE_COMPARTMENT, objects);

	//first check if there is a compartment of the right kind and id
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		MxCompartment* comp = objects[i]->isCompartment();
		if (comp)
		{
			if (comp->getCompartmentID() == compartmentID && comp->getCompartmentType() == type)
			{
				comp->incRef();
				return comp;
			}
		}
	}

	//no matching (existing) compartment found, now try "dead" compartments
	MxCompartment* mxComp = NULL;
	NxArray<MxCompartment*>& deadList = m_instance->getDeadCompartments();
	for (NxU32 i = 0; i < deadList.size(); i++)
	{
		MxCompartment* tempComp = deadList[i];
		if (tempComp->getCompartmentType() == type) 
		{
			tempComp->m_compartmentID = compartmentID;
#if NX_SDK_VERSION_NUMBER >= 270
			tempComp->getNxCompartment()->setTimeScale(1.0f);
#endif
			deadList.replaceWithLast(i);
			mxComp = tempComp;
			break;
		}
	}

	if (mxComp == NULL)
	{
		//no matching "dead" compartments found, create a new one
		NxCompartmentDesc compartmentDesc;
		compartmentDesc.type = type;
		compartmentDesc.deviceCode = NX_DC_PPU_AUTO_ASSIGN;
		//TODO: some way for users to control the other settings of the compartment
		//perhaps a "px_setdefaultcompartmentsettings()" method in the plugin object?
		NxCompartment* compartment = m_instance->getScene()->createCompartment(compartmentDesc);
		if (compartment == NULL) 
		{
			return NULL;
		}

		MxCompartment* mxComp = new MxCompartment(NULL, type, compartmentID, compartment);
	}

	if (mxComp) 
	{
		mxComp->incRef();
		m_instance->getObjectArray().pushBack(mxComp);
	}
	return mxComp;
}

bool MxPluginData::releaseCompartment(MxCompartment* compartment)
{
	if (compartment == NULL) return false;
	if (m_instance == NULL) return false;

	//release one reference
	compartment->decRef();

	//if the reference count is down to zero, then it is time to release the object
	//which is however a problem with compartments, since they can't be released
	if (compartment->getRefCount() == 0)
	{
		NxCompartment* comp = compartment->getNxCompartment();
		//TODO: inactivate compartment, if possible
#if NX_SDK_VERSION_NUMBER >= 270
		comp->setTimeScale(0.0f);
#endif
		m_instance->getDeadCompartments().pushBack(compartment);
		m_instance->removeObjectFromArray(compartment);
	}
	return true;
}

bool MxPluginData::removeObjectFromArray(MxObject* obj)
{
	if (obj == NULL) return false;
	for (NxU32 i = 0; i < m_objects.size(); i++)
	{
		if (m_objects[i] == obj)
		{
			m_objects.replaceWithLast(i);
			return true;
		}
	}
	return false;
}


MxPluginData::MxPluginData(CharStream* outputStream) 
{
	m_outputStream = outputStream;
	m_userOutputStream = new MxUserOutputStream(m_outputStream);
	m_physicsSDK = NULL;
	m_scene = NULL;
	m_cookingInterface = NULL;
	m_cookingInitialized = false;
	m_instance = this;

	if (gDebugVisualizer == NULL) 
	{
		//TODO: delete this some time, but don't forget to unregister the listener first
		gDebugVisualizer = new MxDebugVisualizer();
	}
}

bool MxPluginData::init()
{
	assert(m_physicsSDK == NULL); //should not have been created
	assert(m_scene == NULL); //should not have been created

	NxPhysicsSDKDesc sdkDesc;
	if (!PxFunctions::mSetting_useHardware) {
		sdkDesc.flags |= NX_SDKF_NO_HARDWARE;
	}
	NxSDKCreateError errorCode;
	sdkDesc.isValid();
	m_physicsSDK = NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, m_userOutputStream, sdkDesc, &errorCode);

	if (! m_physicsSDK)
	{
		MessageBoxA(NULL, "Warning: Fail to create PhysX SDK. Please check whether you install correct PhysX driver.", "Warning!",MB_OK | MB_ICONWARNING | MB_SYSTEMMODAL);
	}

	m_hwPhysX = false;
	NxHWVersion hwCheck = m_physicsSDK->getHWVersion();
	if (hwCheck != NX_HW_VERSION_NONE) 
	{   
		m_hwPhysX = true;
	}

	//Uncomment if you want to connect to the Visual Remote Debugger (VRD)
	if (m_physicsSDK) m_physicsSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost");

	if (errorCode != NXCE_NO_ERROR && m_outputStream != NULL)
	{
		switch(errorCode) {
			case NXCE_PHYSX_NOT_FOUND:
				{
					m_outputStream->printf("Unable to find the PhysX libraries, can't initialize the plugin.\n");
					break;
				}

			case NXCE_WRONG_VERSION:
				{
					m_outputStream->printf("Found PhysX libraries but they are of the wrong version, can't initialize the plugin.\n");
					break;
				}

			case NXCE_DESCRIPTOR_INVALID:
				{
					m_outputStream->printf("Internal error (invalid descriptor), can't initialize the plugin.\n");
					break;
				}

			case NXCE_CONNECTION_ERROR:
				{
					m_outputStream->printf("The PhysX hardware could not be initialized, the plugin won't be able to create hardware objects.\n");
					break;
				}

			case NXCE_RESET_ERROR:
				{
					m_outputStream->printf("The PhysX hardware could not be initialized, the plugin won't be able to create hardware objects.\n");
					break;
				}

			case NXCE_IN_USE_ERROR:
				{
					m_outputStream->printf("The PhysX hardware could not be initialized (in use by another application), the plugin won't be able to create hardware objects.\n");
					break;
				}
#if NX_SDK_VERSION_NUMBER >= 270
			case NXCE_BUNDLE_ERROR:
				{
					m_outputStream->printf("The PhysX hardware could not be initialized, the plugin won't be able to create hardware objects.\n");
					break;
				}
#endif

			default:
				{
					m_outputStream->printf("An unknown error occurred when initializing the PhysX SDK, the plugin might work but be unstable.\n");
					break;
				}
		}
	}
	if (m_physicsSDK == NULL) return false;

	if (gSDKParamSettings != NULL)
	{
		for (NxU32 i = 0; i < (NxU32)NX_PARAMS_NUM_VALUES; i++)
		{
			gSDKParamSettings[i].defaultValue = m_physicsSDK->getParameter((NxParameter)i);
			if (gSDKParamSettings[i].inUse)
			{
				m_physicsSDK->setParameter((NxParameter)i, gSDKParamSettings[i].value);
			}
		}
	}

	NxSceneDesc sceneDesc;
	sceneDesc.gravity = (NxVec3&) PxFunctions::mSetting_gravity;
	sceneDesc.userContactReport = &MxContactReport::getMyContactReport();
	m_scene = m_physicsSDK->createScene(sceneDesc);

	m_scene->setActorGroupPairFlags(0,0,NX_NOTIFY_ON_START_TOUCH|NX_NOTIFY_ON_TOUCH|NX_NOTIFY_ON_END_TOUCH);

	return true;
}

MxPluginData::~MxPluginData()
{
	//first release all joints (they can reference rigid bodies)
	for (NxU32 i = 0; i < m_objects.size(); i++)
	{
		MxJoint* mxJoint = m_objects[i]->isJoint();
		if (mxJoint != NULL)
		{
			if (mxJoint->getActor0() != NULL) mxJoint->getActor0()->decRef();
			if (mxJoint->getActor1() != NULL) mxJoint->getActor1()->decRef();
			delete mxJoint;
			m_objects.replaceWithLast(i);

			i--; //since we replaced the current position
		}
	}

	//then release all rigid bodies (should get rid of most other objects as well, because of refcounting)
	NxArray<MxObject*> actors;
	getAllObjectsOfType(MX_OBJECTTYPE_ACTOR, actors);
	for (NxU32 i = 0; i < actors.size(); i++)
	{
		MxActor* mxActor = actors[i]->isActor();
		if (mxActor != NULL)
		{
			releaseActor(mxActor);
		}
	}

	//clear shapes
	//for (NxU32 i = 0; i < m_shapes.size(); i++)
	//{
	//	MxShape* p = m_shapes[i];
	//	delete p;
	//}
	//m_shapes.clear();

	//release cloths
	NxArray<MxObject*> cloths;
	getAllObjectsOfType(MX_OBJECTTYPE_CLOTH, cloths);
	for (NxU32 i = 0; i < cloths.size(); i++)
	{
		MxCloth* mxCloth = cloths[i]->isCloth();
		if (mxCloth != NULL)
		{
			releaseCloth(mxCloth);
		}
	}

	//release softbodies
	NxArray<MxObject*> softbodies;
	getAllObjectsOfType(MX_OBJECTTYPE_SOFTBODY, softbodies);
	for (NxU32 i = 0; i < softbodies.size(); i++)
	{
		MxSoftBody* mxSoftBody = softbodies[i]->isSoftBody();
		if (mxSoftBody != NULL)
		{
			releaseSoftBody(mxSoftBody);
		}
	}

	//release fluid emitters
	for (NxU32 i = 0; i < m_objects.size(); i++)
	{
		MxFluidEmitter* mxEmitter = m_objects[i]->isFluidEmitter();
		if (mxEmitter != NULL)
		{
			delete mxEmitter;
			m_objects.replaceWithLast(i);

			i--; //since we replaced the current position
		}
	}

	//release fluids
	for (NxU32 i = 0; i < m_objects.size(); i++)
	{
		MxFluid* mxFluid = m_objects[i]->isFluid();
		if (mxFluid != NULL)
		{
			delete mxFluid;
			m_objects.replaceWithLast(i);

			i--; //since we replaced the current position
		}
	}

	//compartments
	for (NxU32 i = 0; i < m_deadCompartments.size(); i++)
	{
		delete m_deadCompartments[i];
	}
	m_deadCompartments.clear();
	for (NxU32 i = 0; i < m_objects.size(); i++)
	{
		MxCompartment* mxComp = m_objects[i]->isCompartment();
		if (mxComp != NULL)
		{
			//can't release NxCompartments, but that should be done by the MxCompartment object anyhow...
			//NxCompartment* nxComp = mxComp->getNxCompartment();
			delete mxComp;
			m_objects.replaceWithLast(i);
			
			i--; //since we replaced the current position
		}
	}

	//now there should be no more objects
	if (m_objects.size() > 0) {
		if (m_outputStream) m_outputStream->printf("PhysX plugin error: There are still objects left when removing the plugin data tracker.");
		debugPrint();
	}

	if (m_scene)
	{
		if (m_physicsSDK) m_physicsSDK->releaseScene(*m_scene);
		m_scene = NULL;
	}

	if (m_cookingInterface != NULL)
	{
		if (m_cookingInitialized)
		{
			m_cookingInterface->NxCloseCooking();
			m_cookingInitialized = false;
		}
		//can't release the cooking interface itself
		m_cookingInterface = NULL;
	}

	if (m_physicsSDK)
	{
		NxReleasePhysicsSDK(m_physicsSDK);
		m_physicsSDK = NULL;
	}
	if (m_userOutputStream)
	{
		delete m_userOutputStream;
		m_userOutputStream = NULL;
	}
	m_outputStream = NULL;
	m_instance = NULL;
}

//creates a new mesh, or uses an already existing one and refcounts
MxMesh* MxPluginData::createMesh(INode* node, MxMeshType type)
{
	if (m_instance == NULL) return NULL;
	if (m_instance->getScene() == NULL) return NULL;

	//handle trimesh instancing
	NxArray<MxObject*> meshes;
	getAllObjectsOfType(MX_OBJECTTYPE_MESH, meshes);
	for (NxU32 i = 0; i < meshes.size(); i++)
	{
		MxMesh* tempMesh = meshes[i]->isMesh();
		if (tempMesh->canInstantiate(node, type))
		{
			tempMesh->incRef();
			return tempMesh;
		}
	}

	MxMesh* mxMesh = new MxMesh(node->GetName(), node, type);
	if (mxMesh != NULL && mxMesh->isInitialized())
	{
		m_objects.pushBack(mxMesh);
		mxMesh->incRef();
		return mxMesh;
	}

	//fail
	if (mxMesh != NULL) 
	{
		//the mesh object should not have been added to the object array
		delete mxMesh;
		mxMesh = NULL;
	}
	return NULL;
}

//decrease the refcount of the mesh, if it reaches zero, then release it
bool MxPluginData::releaseMesh(MxMesh* mesh)
{
	if (mesh == NULL) return false;

	mesh->decRef();
	if (mesh->getRefCount() == 0)
	{
		removeObjectFromArray(mesh);
		delete mesh;
	}
	return true;
}


void MxPluginData::getAllObjectsOfType(MxObjectType type, NxArray<MxObject*>& destination)
{
	for (NxU32 i = 0; i < m_objects.size(); i++) 
	{
		if (m_objects[i]->isType(type) != NULL)
		{
			destination.pushBack(m_objects[i]);
		}
	}
}

NxCookingInterface* MxPluginData::getCookingInterface()
{
	if (m_cookingInterface == NULL)
	{
		m_cookingInterface = NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
		if (m_cookingInterface == NULL)
		{
			if (gCurrentstream) gCurrentstream->printf("Error: Unable to find the PhysX cooking library, please make sure that PhysX is correctly installed.\n");
			return NULL;
		}
	}
	if (m_cookingInterface != NULL && !m_cookingInitialized)
	{
		m_cookingInitialized = m_cookingInterface->NxInitCooking(NULL, m_userOutputStream);
		if (!m_cookingInitialized)
		{
			if (gCurrentstream) gCurrentstream->printf("Error: The PhysX cooking library was found, but could not be initialized.\n");
			return NULL;
		}
	}
	if (m_cookingInterface != NULL && m_cookingInitialized)
	{
		return m_cookingInterface;
	}
	return NULL;
}

void MxPluginData::debugPrint() 
{
	if (gCurrentstream == NULL) return;

	if (m_physicsSDK)
	{
		gCurrentstream->printf("PhysX SDK version: %X, HW version: %d", NX_PHYSICS_SDK_VERSION, m_physicsSDK->getHWVersion());
		gCurrentstream->printf("   Number of PPUs: %d\n", m_physicsSDK->getNbPPUs());
#if NX_SDK_VERSION_NUMBER >= 270
		gCurrentstream->printf("   Number of Triangle meshes: %d\n", m_physicsSDK->getNbTriangleMeshes());
		for (NxU32 i = 0; i < m_physicsSDK->getNbTriangleMeshes(); i++)
		{
			NxArray<NxTriangleMesh*> meshes;
			for (NxU32 j = 0; j < m_physicsSDK->getNbScenes(); j++)
			{
				NxScene* scene = m_physicsSDK->getScene(j);
				for (NxU32 k = 0; k < scene->getNbActors(); k++)
				{
					NxActor* actor = scene->getActors()[k];
					for (NxU32 l = 0; l < actor->getNbShapes(); l++)
					{
						NxShape* shape = actor->getShapes()[l];
						NxTriangleMeshShape* meshShape = shape->isTriangleMesh();
						if (meshShape != NULL && ((&meshShape->getTriangleMesh()) != NULL))
						{
							NxTriangleMesh* mesh = &meshShape->getTriangleMesh();
							bool alreadyExists = false;
							for (NxU32 m = 0; m < meshes.size(); m++)
							{
								if (meshes[m] == mesh)
								{
									alreadyExists = true;
									break;
								}
							}
							if (!alreadyExists)
							{
								meshes.pushBack(mesh);
							}
						}
					}
				}
			}

			for (NxU32 j = 0; j < meshes.size(); j++)
			{
				NxTriangleMesh* mesh = meshes[j];
				NxTriangleMeshDesc meshDesc;
				mesh->saveToDesc(meshDesc);
				gCurrentstream->printf("      %X: Refcount: %d, vertices: %d, triangles: %d\n", (size_t) mesh, mesh->getReferenceCount(), meshDesc.numVertices, meshDesc.numTriangles);
			}
		}
		gCurrentstream->printf("   Number of HeightFields: %d\n", m_physicsSDK->getNbHeightFields());
#endif //NX_SDK_VERSION_NUMBER >= 270

		gCurrentstream->printf("   Number of Cloth meshes: %d\n", m_physicsSDK->getNbClothMeshes());
		for (NxU32 i = 0; i < m_physicsSDK->getNbClothMeshes(); i++)
		{
			NxClothMesh* mesh = m_physicsSDK->getClothMeshes()[i];
			NxClothMeshDesc meshDesc;
			mesh->saveToDesc(meshDesc);
			gCurrentstream->printf("      %X: Refcount: %d, vertices: %d, triangles: %d\n", (size_t) mesh, mesh->getReferenceCount(), meshDesc.numVertices, meshDesc.numTriangles);
		}
#ifdef PXPLUGIN_SOFTBODY
		gCurrentstream->printf("   Number of SoftBody meshes: %d\n", m_physicsSDK->getNbSoftBodyMeshes());
		for (NxU32 i = 0; i < m_physicsSDK->getNbSoftBodyMeshes(); i++)
		{
			NxSoftBodyMesh* mesh = m_physicsSDK->getSoftBodyMeshes()[i];
			NxSoftBodyMeshDesc meshDesc;
			mesh->saveToDesc(meshDesc);
			//gCurrentstream->printf("      %X: Refcount: %d, vertices: %d, triangles: %d\n", (int) mesh, mesh->getReferenceCount(), meshDesc.numVertices, meshDesc.numTriangles);
			gCurrentstream->printf("      %X: Refcount: %d\n", (size_t) mesh, mesh->getReferenceCount());
		}
#endif
#if NX_SDK_VERSION_NUMBER >= 270
		gCurrentstream->printf("   Number of CCD skeletons: %d\n", m_physicsSDK->getNbCCDSkeletons());
#endif
		gCurrentstream->printf("   Number of Scenes: %d\n", m_physicsSDK->getNbScenes());
		for (NxU32 i = 0; i < m_physicsSDK->getNbScenes(); i++)
		{
			NxScene* scene = m_physicsSDK->getScene(i);
			gCurrentstream->printf("   Scene object: %X\n", (size_t) scene);
			gCurrentstream->printf("      Number of NxActors: %d\n", scene->getNbActors());
			for (NxU32 j = 0; j < scene->getNbActors(); j++)
			{
				NxActor* actor = scene->getActors()[j];
				gCurrentstream->printf("         %X: Number of shapes: %d\n", (size_t) actor, actor->getNbShapes());
				for (NxU32 k = 0; k < actor->getNbShapes(); k++)
				{
					NxShape* shape = actor->getShapes()[k];
					gCurrentstream->printf("            %X: shape type: %d", (size_t) shape, (int) shape->getType());
					if (shape->isTriangleMesh())
					{
						gCurrentstream->printf(", mesh: %X", (size_t) (&shape->isTriangleMesh()->getTriangleMesh()));
					}
					if (shape->isConvexMesh())
					{
						gCurrentstream->printf(", convex mesh: %X", (size_t) (&shape->isConvexMesh()->getConvexMesh()));
					}
					gCurrentstream->printf("\n");
				}
			}
		}
	} else {
		gCurrentstream->printf("PhysX SDK: None\n");
	}

	gCurrentstream->printf("Our scene object: %X\n", (size_t) m_scene);
	gCurrentstream->printf("Cooking interface object: %X, initialized: %s\n", (size_t) m_cookingInterface, (m_cookingInitialized?"true":"false"));

	gCurrentstream->printf("---------------------------------------------------\n");

	gCurrentstream->printf("Number of objects in internal object array: %d\n", m_objects.size());
	for (NxU32 i = 0; i < m_objects.size(); i++)
	{
		MxObject* object = m_objects[i];
		gCurrentstream->printf("   %X: name: %s, id: %d, refcount: %d, INode: %X (%s)\n", (size_t) object, object->getName(), object->getID(), object->getRefCount(), object->getNode(), object->getNode()?object->getNode()->GetName():"");
		if (object->isActor())
		{
			MxActor* mxActor  = object->isActor();
			//gCurrentstream->printf("      Actor: number of shapes: %d\n", mxActor->m_shapes.size());
			NxActor* pa       = mxActor->getNxActor();
			if(pa)
			{
				gCurrentstream->printf("      Actor: number of shapes: %d\n", pa->getNbShapes());
			}
			
			//for (NxU32 j = 0; j < mxActor->m_shapes.size(); j++)
			//{
			//	MxShape* shape = mxActor->m_shapes[j];
			//	gCurrentstream->printf("         %X: NxShape: %X, name: %s, INode: %X (%s)\n", (size_t) shape, (size_t) shape->getNxShape(), shape->getName(), (size_t) shape->getNode(), shape->getNode()?shape->getNode()->GetName():"");
			//}
		} else if (object->isCloth())
		{
			gCurrentstream->printf("      Cloth: mesh: %X\n", 0);
		} else if (object->isCompartment())
		{
			gCurrentstream->printf("      Compartment\n");
		} else if (object->isFluid())
		{
			gCurrentstream->printf("      Fluid\n");
		} else if (object->isFluidEmitter())
		{
			gCurrentstream->printf("      Fluid emitter\n");
		} else if (object->isJoint())
		{
			gCurrentstream->printf("      Joint\n");
		} else if (object->isMaterial())
		{
			gCurrentstream->printf("      Material\n");
		} else if (object->isMesh())
		{
			gCurrentstream->printf("      Mesh\n");
		}
		//else if (object->isShape())
		//{
		//	gCurrentstream->printf("      Shape\n");
		//}
#ifdef PXPLUGIN_SOFTBODY
		else if (object->isSoftBody())
		{
			gCurrentstream->printf("      SoftBody\n");
#endif
		} else 
		{
			gCurrentstream->printf("      Error: unknown object type\n");
		}
	}

	gCurrentstream->printf("Number of dead compartments: %d\n", m_deadCompartments.size());
}

NxU32 MxPluginData::removeOldObjects(INode* node, const char* name)
{
	NxU32 nrRemoved = 0;

	NxArray<MxObject*> objects;
	getObjectsFromNode(node, objects);
	NxU32 i = 0;
	while (i < objects.size())
	{
		bool removed = releaseObject(objects[i]);
		if (removed)
		{
			nrRemoved++;
			objects.clear();
			getObjectsFromNode(node, objects);
			i = 0;
		} else
		{
			i++;
		}
	}

	objects.clear();
	getObjectsFromName(name, objects);
	i = 0;
	while (i < objects.size())
	{
		bool removed = releaseObject(objects[i]);
		if (removed)
		{
			nrRemoved++;
			objects.clear();
			getObjectsFromName(name, objects);
			i = 0;
		} else
		{
			i++;
		}
	}
	return nrRemoved;
}
