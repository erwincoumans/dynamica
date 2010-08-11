
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
#include <SimpObj.h>

#include "PxPlugin.h"
#include "MxUtils.h"
#include "MxUserOutputStream.h"
#include "MxPluginData.h"
//#include "MxFluid.h"
//#include "MxFluidEmitter.h"
#include "MxMesh.h"
#include "MxJoint.h"
//#include "MxShape.h"
#include "MxMaterial.h"
//#include "MxCloth.h"
#include "MxActor.h"
//#include "MxSoftBody.h"
//#include "MxForceField.h"

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
		if (objects[i]->getNode() == node) destination.push_back(objects[i]);
	}	
}

void MxPluginData::getObjectsFromName(const char* name, NxArray<MxObject*>& destination)
{
	if (m_instance == NULL) return;
	if (name == NULL) return;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		if (strcmp(name, objects[i]->getName().data()) == 0) destination.push_back(objects[i]);
	}
}

void MxPluginData::getAllObjects(NxArray<MxObject*>& destination)
{
	if (m_instance == NULL) return;

	NxArray<MxObject*>& objects = m_instance->getObjectArray();
	destination.reserve(objects.size());
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		destination.push_back(objects[i]);
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
//	if (m_instance->getScene() == NULL) return NULL;

	//check so that this node has not already been added, should it be possible to do that?
	if (getObjectFromNode(node) != NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: trying to create an actor of node \"%s\", but it is already added as another object.\n", node->GetName());
		return NULL;
	}

	MxActor* actor = new MxActor(node->GetName(), node);
	m_instance->getObjectArray().push_back(actor);
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
	//NxActor* pa = actor->getNxActor();
	//NxU32 shapeCount = pa? 1 : 0;
	actor->releaseAllShapes();
	//if (sideEffects != NULL && shapeCount > 0) *sideEffects = true;
	
	m_instance->removeObjectFromArray(actor);
	delete actor;
	return true;
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
//				m_instance->m_shapes.push_back(shape);
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
//				m_instance->m_shapes.push_back(shape);
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
//		m_instance->m_shapes.push_back(shape);
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
//		m_instance->m_shapes.push_back(shape);
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
//		m_instance->m_shapes.push_back(shape);
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
	m_instance->getObjectArray().push_back(mxJoint);

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
	m_instance->getObjectArray().push_back(mxJoint);

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

	return NULL;
}

bool MxPluginData::releaseFluid(MxFluid* fluid, bool* sideEffects)
{
	//the caller should make sure that there are no emitters referencing the fluid before releasing it

	return true;
}

MxFluidEmitter* MxPluginData::createFluidEmitter(INode* node)
{
	
	return NULL;
}

bool MxPluginData::releaseFluidEmitter(MxFluidEmitter* emitter)
{
	
	return true;
}

void MxPluginData::getAllEmitters(NxArray<MxFluidEmitter*>& destination)
{
	
}


MxCloth* MxPluginData::createCloth(INode* node, bool isMetalCloth)
{
	
	return NULL;
}

bool MxPluginData::releaseCloth(MxCloth* cloth)
{
	
	return true;
}

MxForceField* MxPluginData::createForceField(INode* node)
{
	
	return NULL;
}

bool MxPluginData::releaseForceField(MxForceField* forceField)
{
	
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
			m_instance->getObjectArray().push_back(softbody);
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
	return 0;
}

bool MxPluginData::releaseCompartment(MxCompartment* compartment)
{
	
	return true;
}

bool MxPluginData::removeObjectFromArray(MxObject* obj)
{
	if (obj == NULL) return false;
	m_objects.remove(obj);

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
#if 0
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
#endif

	return true;
}

MxPluginData::~MxPluginData()
{
#if 0
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
#endif

}

//creates a new mesh, or uses an already existing one and refcounts
MxMesh* MxPluginData::createMesh(INode* node, MxMeshType type)
{
#if 0
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
		m_objects.push_back(mxMesh);
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
#endif

	return NULL;
}

//decrease the refcount of the mesh, if it reaches zero, then release it
bool MxPluginData::releaseMesh(MxMesh* mesh)
{
#if 0
	if (mesh == NULL) return false;

	mesh->decRef();
	if (mesh->getRefCount() == 0)
	{
		removeObjectFromArray(mesh);
		delete mesh;
	}
#endif

	return true;
}


void MxPluginData::getAllObjectsOfType(MxObjectType type, NxArray<MxObject*>& destination)
{
	for (NxU32 i = 0; i < m_objects.size(); i++) 
	{
		if (m_objects[i]->isType(type) != NULL)
		{
			destination.push_back(m_objects[i]);
		}
	}
}

NxCookingInterface* MxPluginData::getCookingInterface()
{
#if 0
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
#endif

	return NULL;
}

void MxPluginData::debugPrint() 
{

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
