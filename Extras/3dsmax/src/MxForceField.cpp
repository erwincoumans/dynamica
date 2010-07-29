#include "PxPlugin.h"

#include <NxPhysics.h>
#include <NxCooking.h>

#include <max.h>
#include <SimpObj.h>
#include <MAXScrpt\MAXScrpt.h>

#include "NXU_Streaming.h"

#include "MxActor.h"
#include "MxUtils.h"
#include "MxPluginData.h"
#include "MxObjects.h"
#include "MxForceField.h"
#include "MxMesh.h"


MxForceField::MxForceField(const char* name, INode* node) : MxObject(name, node), 
	m_nxForceField(NULL), m_nxKernel(NULL), m_nxFFShape(NULL), m_compartment(NULL), m_hasFrameActor(false)
{
	m_ObjectType = MX_OBJECTTYPE_FORCEFIELD;
	if (node == NULL) return;

#if	NX_SDK_VERSION_NUMBER	>= 280
	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = m_node->EvalWorldState(t); 
	int sid = os.obj->SuperClassID();
	IParamBlock2* pb = os.obj->GetParamBlockByID(0);

	m_nodeTM = m_node->GetNodeTM(t); //GetObjectTM(t);
	NxMat34 nxNodeTM = MxMathUtils::MaxMatrixToNx(m_nodeTM);
	//Matrix3 actorinv = Inverse(m_nodeTM);
	//m_nxInvNodeTM = MxMathUtils::MaxMatrixToNx(actorinv);

	NxPhysicsSDK* sdk = MxPluginData::getPhysicsSDKStatic();
	if (sdk == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: Unable to get hold of the PhysX SDK\n");
		return ;
	}
	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	if (cookingInterface == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: Unable to find the NxCookingInterface (make sure that PhysXCooking.dll can be found by 3dsmax.exe)\n");
		return ;
	}
	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: There is no scene created yet, can't create a SoftBody\n");
		return ;
	}


	NxU32 ffFlag = 0;
	FlagHolder flagTable[] =
	{
		FlagHolder("px_fff_volumetric_scaling_fluid",			NX_FFF_VOLUMETRIC_SCALING_FLUID),
		FlagHolder("px_fff_volumetric_scaling_cloth",			NX_FFF_VOLUMETRIC_SCALING_CLOTH),
		FlagHolder("px_fff_volumetric_scaling_softbody",		NX_FFF_VOLUMETRIC_SCALING_SOFTBODY),
		FlagHolder("px_fff_volumetric_scaling_rigidbody",		NX_FFF_VOLUMETRIC_SCALING_RIGIDBODY),
	};
	const NxU32 numFlags = sizeof(flagTable)/sizeof(FlagHolder);
	//bool hwSimulation = (scene!=NULL)?scene->getSimType()==NX_SIMULATION_HW:false;
	for (NxU32 i = 0; i < numFlags; i++)
	{
		BOOL val = FALSE;
		if (m_node->GetUserPropBool(flagTable[i].name, val))
		{
			if (val)
			{
				ffFlag |= flagTable[i].flagValue;
			}
		}
	}

	NxForceFieldDesc ffDesc;
	NxForceFieldLinearKernelDesc	lKernelDesc;
	int iV;
	if (m_node->GetUserPropInt("px_interaction_rigidbody", iV))
	{
		ffDesc.rigidBodyType = (NxForceFieldType) (iV - 1);
	}
	if (m_node->GetUserPropInt("px_interaction_fluid", iV))
	{
		ffDesc.fluidType = (NxForceFieldType) (iV - 1);
	}
	if (m_node->GetUserPropInt("px_interaction_cloth", iV))
	{
		ffDesc.clothType = (NxForceFieldType) (iV - 1);
	}
	if (m_node->GetUserPropInt("px_interaction_softbody", iV))
	{
		ffDesc.softBodyType = (NxForceFieldType) (iV - 1);
	}
	float fx = 0, fy = 0, fz = 0;
	if (m_node->GetUserPropFloat("px_lk_contant_force_x", fx) && 
		m_node->GetUserPropFloat("px_lk_contant_force_y", fy) &&
		m_node->GetUserPropFloat("px_lk_contant_force_z", fz))
	{
		lKernelDesc.constant = NxVec3(fx, fy, fz);
	}

	fx = 0, fy = 0, fz = 0;
	if (m_node->GetUserPropFloat("px_lk_position_target_x", fx) && 
		m_node->GetUserPropFloat("px_lk_position_target_y", fy) &&
		m_node->GetUserPropFloat("px_lk_position_target_z", fz))
	{
		lKernelDesc.positionTarget = NxVec3(fx, fy, fz);
	}

	fx = 0, fy = 0, fz = 0;
	if (m_node->GetUserPropFloat("px_lk_velocity_target_x", fx) && 
		m_node->GetUserPropFloat("px_lk_velocity_target_y", fy) &&
		m_node->GetUserPropFloat("px_lk_velocity_target_z", fz))
	{
		lKernelDesc.velocityTarget = NxVec3(fx, fy, fz);
	}

	fx = 0, fy = 0, fz = 0;
	if (m_node->GetUserPropFloat("px_lk_falloff_linear_x", fx) && 
		m_node->GetUserPropFloat("px_lk_falloff_linear_y", fy) &&
		m_node->GetUserPropFloat("px_lk_falloff_linear_z", fz))
	{
	
		lKernelDesc.falloffLinear = NxVec3(fx, fy, fz);
	}

	fx = 0, fy = 0, fz = 0;
	if (m_node->GetUserPropFloat("px_lk_falloff_quadratic_x", fx) && 
		m_node->GetUserPropFloat("px_lk_falloff_quadratic_y", fy) &&
		m_node->GetUserPropFloat("px_lk_falloff_quadratic_z", fz))
	{
		lKernelDesc.falloffQuadratic = NxVec3(fx, fy, fz);
	}

	fx = 0, fy = 0, fz = 0;
	if (m_node->GetUserPropFloat("px_lk_noise_x", fx) && 
		m_node->GetUserPropFloat("px_lk_noise_y", fy) &&
		m_node->GetUserPropFloat("px_lk_noise_z", fz))
	{
		lKernelDesc.noise = NxVec3(fx, fy, fz);
	}

	//The forces do not depend on where the objects are positioned
	NxMat33 m;
	m.zero();
	lKernelDesc.positionMultiplier = m;
	lKernelDesc.velocityMultiplier = m;

	// create linear kernel
	m_nxKernel = scene->createForceFieldLinearKernel(lKernelDesc);
	ffDesc.kernel = m_nxKernel;
	
	ffDesc.pose = nxNodeTM;
	//Attach the force field to an actor (kinematic) so that we can move it around 
	// (spawn the explosions in different places)
	TSTR str = MxUserPropUtils::GetUserPropStr(m_node, "px_ff_frame_actor");
	char* nodeName = str.data();
	if(nodeName && strlen(nodeName) > 0 && (stricmp(nodeName, "undefined") != 0))
	{
		MxActor* actor = MxUtils::GetActorFromName(nodeName);
		if(actor) {
			ffDesc.actor = actor->getNxActor();
			NxMat34 ap = ffDesc.actor->getGlobalPose(); //MxMathUtils::MaxMatrixToNx(actor->getNode()->GetNodeTM(t)); //GetObjectTM(t);
			NxMat34 pose;
			ap.getInverse(pose);
			pose = nxNodeTM * pose;
			ffDesc.pose = pose;
			m_hasFrameActor = true;
		}
	}

	if (m_node->GetUserPropInt("px_coordinate_space", iV))
	{
			ffDesc.coordinates = (NxForceFieldCoordinates)(iV - 1);
	}

	ffDesc.flags = ffFlag;

	//Attach an include shape, we will animate the size of this later on, so that it grows (like a slow explosion)
	NxForceFieldShapeGroupDesc sgInclDesc;
	//NxForceFieldShapeGroup* inclusionGroup = scene->createForceFieldShapeGroup(sgInclDesc);
	NxForceFieldShape* shape = NULL;

	NxMat34 pose; // = MxMathUtils::MaxMatrixToNx(m_node->GetNodeTM(t));
	pose.id();

	Class_ID id = os.obj->ClassID();
	SimpleObject* so = (SimpleObject*) os.obj;
	if (id == Class_ID(SPHERE_CLASS_ID, 0)) {
		NxSphereForceFieldShapeDesc * s = new NxSphereForceFieldShapeDesc;
		so->pblock->GetValue(SPHERE_RADIUS, 0, s->radius, FOREVER);
		s->pose = pose;
		s->pose.getInverse(m_localPoseInv);
		//m_localPoseInv = s->pose;
		//shape = inclusionGroup->createShape(s);
		s->userData = m_node;
		ffDesc.includeGroupShapes.push_back(s);
	} else if (id == Class_ID(BOXOBJ_CLASS_ID, 0)) 
	{
		NxBoxForceFieldShapeDesc * b = new NxBoxForceFieldShapeDesc;
		so->pblock->GetValue(BOXOBJ_WIDTH , 0, b->dimensions.x, FOREVER);
		so->pblock->GetValue(BOXOBJ_LENGTH, 0, b->dimensions.y, FOREVER);
		so->pblock->GetValue(BOXOBJ_HEIGHT, 0, b->dimensions.z, FOREVER);
		b->pose = pose;
		b->pose.t.z += b->dimensions.z * 0.5f; //adjust for difference in pivot points between 3ds max and PhysX
		b->pose.getInverse(m_localPoseInv);
		//m_localPoseInv = b->pose;
		for (int i = 0; i < 3; i++) 
			b->dimensions[i] = fabsf(b->dimensions[i])*0.5f; //PhysX stores half the actual size as the dimension (extents) of the box
		//shape = inclusionGroup->createShape(b);
		b->userData = m_node;
		ffDesc.includeGroupShapes.push_back(b);
	} else if (id == CAPS_CLASS_ID)
	{
		int centersflag = 0;
		NxCapsuleForceFieldShapeDesc * c = new NxCapsuleForceFieldShapeDesc;
		so->pblock->GetValue(CAPS_RADIUS , 0, c->radius, FOREVER);
		so->pblock->GetValue(CAPS_HEIGHT , 0, c->height, FOREVER);
		so->pblock->GetValue(CAPS_CENTERS, 0, centersflag, FOREVER);
		c->pose.M = NxMat33(NxVec3(1,0,0),NxVec3(0,0,1),NxVec3(0,-1,0));
		if(!centersflag) c->height -= c->radius*2.0f; //there are some different ways in which you can specify a capsule in 3ds max, adjust length if "center" mode is not used
		c->pose.t.z += (c->height/2 + c->radius); //adjust for difference in pivot points between 3ds max and PhysX
		c->pose.getInverse(m_localPoseInv);
		//m_localPoseInv = c->pose;
		//shape = inclusionGroup->createShape(c);
		c->userData = m_node;
		ffDesc.includeGroupShapes.push_back(c);
	} else
	{
		MxMesh* mesh = MxPluginData::getInstance()->createMesh(m_node, MX_MESHTYPE_CONVEX);
		if (mesh && mesh->getNxConvexMesh() != NULL)
		{
			NxConvexForceFieldShapeDesc * d = new NxConvexForceFieldShapeDesc;
			d->pose = pose;
			d->pose.getInverse(m_localPoseInv);
			//m_localPoseInv = d->pose;
			d->meshData = mesh->getNxConvexMesh();
			//MxShape* shape = MxPluginData::createShape(m_node, NX_SHAPE_CONVEX, false);
			d->userData = m_node;
			ffDesc.includeGroupShapes.push_back(d);
		}
		else
		{
			if (gCurrentstream) 
				gCurrentstream->printf("Failure: ForceField is not created because the shape is concave or it has more than 32 faces!");
		}
		//pose = m_localPoseInv * ffDesc.pose;
		//pose.getInverse(m_localPoseInv);
		/*
		int isConcaveSetting = 0;
		bool isConcave = false;
		if (node->GetUserPropInt("IsConcave", isConcaveSetting))
		{
			isConcave = isConcaveSetting != 0;
		}
		if (isConcave) 
		{
			shape = MxPluginData::createShape(node, NX_SHAPE_MESH);
		} else 
		{
			shape = MxPluginData::createShape(node, NX_SHAPE_CONVEX);
		}
		*/
	}

	//// exclude group
	//NxForceFieldShapeGroupDesc sgExclDesc;
	//sgExclDesc.flags = NX_FFSG_EXCLUDE_GROUP;
	//m_excludeGroup = gScene->createForceFieldShapeGroup(sgExclDesc);
	//NxSphereForceFieldShapeDesc exclude;
	//exclude.radius = 0.2f;
	//exclude.pose.t = NxVec3(0, 0, 0);
	//m_excludeShape = m_excludeGroup->createShape(exclude);

	//gForceField->addShapeGroup(*m_excludeGroup);

	m_nxForceField = scene->createForceField(ffDesc);
	if (m_nxForceField != NULL)
	{
		m_nxForceField->userData = this;
	}

	//m_nxForceField->addShapeGroup(*inclusionGroup);
	NxForceFieldShapeGroup& shapes = m_nxForceField->getIncludeShapeGroup();
	shapes.resetShapesIterator();
	m_nxFFShape = shapes.getNextShape();

	BOOL needDel = FALSE;
	TriObject* triObj = NULL;
	triObj = MxUtils::GetTriObjectFromNode(node, t, needDel);
	if (needDel && (triObj != NULL))
		triObj->DeleteMe();
#else
	if (gCurrentstream) gCurrentstream->printf("Error: Force Field for SDK 273 is not supported yet in this Plugin release!\n");
#endif
}

void MxForceField::Free(bool deletingObject)
{
	if (deletingObject)
	{
	}

	if (m_nxForceField != NULL)
	{
		NxScene* scene = MxPluginData::getSceneStatic();
		if (scene != NULL)
		{
			scene->releaseForceField(*m_nxForceField);
		}
		m_nxForceField = NULL;
	}
#if	NX_SDK_VERSION_NUMBER	>= 280
	if (m_nxKernel != NULL)
	{
		NxScene* scene = MxPluginData::getSceneStatic();
		if (scene != NULL)
		{
			scene->releaseForceFieldLinearKernel(*m_nxKernel);
		}
		m_nxForceField = NULL;
	}
#endif
	if (m_compartment != NULL)
	{
		gPluginData->releaseCompartment(m_compartment);
		m_compartment = NULL;
	}
}

MxForceField::~MxForceField() 
{
	Free(true);
}

//Can only update the 3ds max object if it is of mesh type (i.e. doesn't need to be converted into a mesh)
void MxForceField::updateMeshFromSimulation(TimeValue t)
{
	//if (!m_updateTriMesh) return;
	if(m_node == NULL) return;
	if(m_nxForceField == NULL) return;

	if(!m_hasFrameActor) return;

	NxMat34 ffPose = m_nxForceField->getPose();
	if(m_nxFFShape && m_nxForceField->getActor())
	{
		NxMat34 p = m_nxForceField->getActor()->getGlobalPose();
		ffPose = p * ffPose * m_nxFFShape->getPose() * m_localPoseInv;
		m_node->SetNodeTM(t,MxMathUtils::NxMatrixToMax(ffPose));
	}


	int deleteIt = 0;
	TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, t, deleteIt);
	if(triObj)
	{
		if(!deleteIt)
		{
		}
		else
		{
			//delete to;
			triObj->DeleteMe();
		}
	}
}

void MxForceField::resetObject()
{
	if (m_node == NULL) return;
	TimeValue t = GetCOREInterface()->GetTime();
	m_node->SetNodeTM(t, m_nodeTM);
	int needDel = 0;
	TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, 0, needDel);
	if (triObj)
	{
		if (needDel) //Can only reset objects if they already are of trimesh type
		{
			triObj->DeleteMe();
		}
	}
}


float MxForceField::GetFloatProp(const char* name, float def)
{
	float v;
	if (m_node->GetUserPropFloat(name, v))
		return v;
	else
		return def;
}

int MxForceField::GetIntProp(const char* name, int def)
{
	int v;
	if (m_node->GetUserPropInt(name, v))
		return v;
	else
		return def;
}

bool MxForceField::GetBoolProp(const char* name, bool def)
{
	BOOL v = FALSE;
	if (m_node->GetUserPropBool(TSTR(name), v))
		return v == TRUE;
	else
		return def;
}

