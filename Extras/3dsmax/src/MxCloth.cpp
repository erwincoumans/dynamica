#include <NxPhysics.h>
#include <NxCooking.h>

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>

#include "PxPlugin.h"
#include "MxUtils.h"
#include "MxPluginData.h"
#include "MxObjects.h"
#include "MxCloth.h"
#include "PxStream.h"
#include "MaxNode.h"
#include "MaxWorld.h"

extern CharStream *gCurrentstream;

MxCloth::MxCloth(const char* name, INode* node, bool isMetalCloth) : MxObject(name, node), m_nxCloth(0), m_updateTriMesh(false), 
                                                                     m_nxMesh(0), m_backupMesh(0), m_isMetalCloth(isMetalCloth), m_coreActor(NULL), maxNodeActor(0)
{
	m_ObjectType = MX_OBJECTTYPE_CLOTH;
	create();
}


void MxCloth::releasePhysicsObject()
{
	if(m_backupMesh) {
		delete m_backupMesh;
		m_backupMesh = 0;
	}
	NxScene* scene = MxPluginData::getSceneStatic();
	if (! scene) return;
	if(m_nxCloth)
		scene->releaseCloth(*m_nxCloth);
	m_nxCloth = NULL;
	if(m_coreActor)
		scene->releaseActor(*m_coreActor);
	m_coreActor = NULL;
	m_meshSimulate.release();
	//m_meshCreation.release();
}

MxCloth::~MxCloth() 
{
	releasePhysicsObject();
}

/*
NxCloth* MxCloth::createOld()
{
	releasePhysicsObject();

	NxScene* scene = MxPluginData::getSceneStatic();
	if (! scene) return 0;

	maxNodeActor = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);

	NxClothDesc clothDesc;
	clothDesc.name = m_node->GetName();
	InitClothDescProps(clothDesc);
	//
	bool ok = MxUtils::nodeToPxSimpleMesh(m_meshCreation, m_node);
	if (! ok)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: Unable to convert \"%s\" into a trimesh object, can't create a cloth out of it.\n", m_node->GetName());
		return 0;
	}
	// Disable backface culling for cloth
	m_node->BackCull(FALSE);

	TimeValue t = GetCOREInterface()->GetTime();
	m_originalPose = m_node->GetNodeTM(t);

	//
	Matrix3 actorinv = Inverse(m_originalPose);
	m_nxInvNodeTM = MxMathUtils::MaxMatrixToNx(actorinv);

	NxClothMeshDesc clothMeshDesc;
	clothMeshDesc.numVertices           = m_meshCreation.numPoints;
	clothMeshDesc.numTriangles          = m_meshCreation.numFaces;
	clothMeshDesc.pointStrideBytes      = sizeof(NxVec3);
	clothMeshDesc.triangleStrideBytes   = sizeof(NxU32)*3;
	clothMeshDesc.points                = m_meshCreation.points;
	clothMeshDesc.triangles             = m_meshCreation.faces;
#if NX_SDK_VERSION_NUMBER > 240
	clothMeshDesc.flags                 = 0;
#else
	clothMeshDesc.flags                 = NX_MDF_INDEXED_MESH;
#endif

	if(clothDesc.flags & NX_CLF_TEARABLE) 
	{
		clothMeshDesc.flags |= NX_CLOTH_MESH_TEARABLE;
	}

	MemoryWriteBuffer clothMeshStream;

	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	bool cookok = false;
	if (cookingInterface != NULL)
	{
		cookok = cookingInterface->NxCookClothMesh(clothMeshDesc, clothMeshStream);
	}
	if(cookok)
	{
		NxPhysicsSDK* sdk = MxPluginData::getPhysicsSDKStatic();
		if (sdk == NULL) return NULL;
		MemoryReadBuffer clothReadStream(clothMeshStream.data);
		m_nxMesh = sdk->createClothMesh(clothReadStream);

		if(m_nxMesh)
		{
			//clothDesc.name = m_name.data();
			clothDesc.clothMesh   = m_nxMesh;
			clothDesc.globalPose = MxMathUtils::MaxMatrixToNx(m_originalPose);

			m_meshSimulate.alloc(m_meshCreation.numPoints * 2, m_meshCreation.numFaces * 2);   // to hold more points for broken cloth

			clothDesc.meshData.verticesPosBegin         = &m_meshSimulate.points[0].x;
			clothDesc.meshData.maxVertices              = m_meshSimulate.numPoints;
			clothDesc.meshData.numVerticesPtr           = &m_numVerts;
			clothDesc.meshData.verticesPosByteStride    = sizeof(NxVec3);

			clothDesc.meshData.verticesNormalBegin      = &m_meshSimulate.normals[0].x;
			clothDesc.meshData.verticesNormalByteStride = sizeof(NxVec3);

			clothDesc.meshData.indicesBegin             = m_meshSimulate.faces;
			clothDesc.meshData.maxIndices               = m_meshSimulate.numFaces * 3;
#if NX_SDK_VERSION_NUMBER >= 250
			clothDesc.meshData.numIndicesPtr            = &m_numIndices;
#else
			clothDesc.meshData.numTrianglesPtr          = &m_numTriangles;
#endif
			clothDesc.meshData.indicesByteStride        = sizeof(NxU32);

			bool checkValid = clothDesc.meshData.isValid();

			clothDesc.toFluidResponseCoefficient		 = 0.8f; 
			clothDesc.fromFluidResponseCoefficient		 = 0.8f;
			bool valid = clothDesc.isValid();
			NxScene* scene = MxPluginData::getSceneStatic();
			if (scene != NULL)
				m_nxCloth = scene->createCloth(clothDesc);

			if(m_nxCloth)
			{
				if(m_isMetalCloth)
				{
					// add metal cloth core
					int mode = 1, number_sphere = 1;
					float impulseThreshold = 0.0f, penetrationDepth = 0.0f, maxDeformationDistance = 0.0f;
					float coreMass = 1.0f;
					m_node->GetUserPropInt("px_mc_core_shape", mode);
					m_node->GetUserPropInt("px_mc_num_sphere", number_sphere);
					m_node->GetUserPropFloat("px_mc_core_mass", coreMass);
					m_node->GetUserPropFloat("px_mc_impulse_threshold", impulseThreshold);
					m_node->GetUserPropFloat("px_mc_penetration_depth", penetrationDepth);
					m_node->GetUserPropFloat("px_mc_max_deform_distance", maxDeformationDistance);
					NxActorDesc coreActorDesc;
					NxBodyDesc  coreBodyDesc;
					coreBodyDesc.mass = coreMass;		// if body mass is set to zero;
					coreBodyDesc.linearDamping = 0.2f;
					coreBodyDesc.angularDamping = 0.2f;
					coreActorDesc.body = &coreBodyDesc;
					bool createCore = true;
					if (mode == 1) { // sphere as core
						coreActorDesc.shapes.pushBack(new NxSphereShapeDesc());
					}
					else if (mode == 2) { // capsule as core
						coreActorDesc.shapes.pushBack(new NxCapsuleShapeDesc());
					}
					else if (mode == 3) { // box as core
						coreActorDesc.shapes.pushBack(new NxBoxShapeDesc());
					}
					else if (mode == 4) { // compound of spheres as core
						createCore = number_sphere > 0;
						for (NxU32 i = 0; i < number_sphere; i++) 
							coreActorDesc.shapes.pushBack(new NxSphereShapeDesc());
					}
					else
					{
						createCore = false;
					}
					if(createCore)
					{
						m_coreActor = scene->createActor(coreActorDesc);
						m_nxCloth->attachToCore(m_coreActor, impulseThreshold, penetrationDepth, maxDeformationDistance); 
						// Clean up allocations
						for (NxU32 i = 0; i < coreActorDesc.shapes.size(); i++)
							delete coreActorDesc.shapes[i];
					}
				}

				m_nxCloth->userData = this;
				BOOL autoattach = FALSE;
				if(m_node->GetUserPropBool("px_cll_autoattach", autoattach) && autoattach)
				{
#if NX_SDK_VERSION_NUMBER >= 250
					m_nxCloth->attachToCollidingShapes(NX_CLOTH_ATTACHMENT_TWOWAY);
#endif
				}
			}
		}

		//Make a backup copy of the mesh (for reset-purposes)
		BOOL needDel = FALSE;
		TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, t, needDel);
		if (triObj != NULL)
		{
			Mesh& mesh = triObj->GetMesh();
			m_backupMesh = new Mesh(mesh);
			if(needDel)
				triObj->DeleteMe();
		}
	}
	return m_nxCloth;
}
*/

NxCloth* MxCloth::create()
{
	releasePhysicsObject();

	NxScene* scene = MxPluginData::getSceneStatic();
	if (! scene) return 0;

	maxNodeActor = ccMaxWorld::FindNode(m_node);
	assert(maxNodeActor);

	NxClothDesc clothDesc;
	clothDesc.name = m_node->GetName();
	InitClothDescProps(clothDesc);
	//
	//bool ok = MxUtils::nodeToPxSimpleMesh(m_meshCreation, m_node);
	//if (! ok)
	//{
	//	if (gCurrentstream) gCurrentstream->printf("Error: Unable to convert \"%s\" into a trimesh object, can't create a cloth out of it.\n", m_node->GetName());
	//	return 0;
	//}
	//// Disable backface culling for cloth
	//m_node->BackCull(FALSE);

	//TimeValue t = GetCOREInterface()->GetTime();
	//m_originalPose = m_node->GetNodeTM(t);

	//
	//Matrix3 actorinv = Inverse(m_originalPose);
	//m_nxInvNodeTM = MxMathUtils::MaxMatrixToNx(actorinv);

	NxClothMeshDesc clothMeshDesc;
	clothMeshDesc.numVertices           = maxNodeActor->SimpleMesh.numPoints;
	clothMeshDesc.numTriangles          = maxNodeActor->SimpleMesh.numFaces;
	clothMeshDesc.pointStrideBytes      = sizeof(NxVec3);
	clothMeshDesc.triangleStrideBytes   = sizeof(NxU32)*3;
	clothMeshDesc.points                = maxNodeActor->SimpleMesh.points;
	clothMeshDesc.triangles             = maxNodeActor->SimpleMesh.faces;
#if NX_SDK_VERSION_NUMBER > 240
	clothMeshDesc.flags                 = 0;
#else
	clothMeshDesc.flags                 = NX_MDF_INDEXED_MESH;
#endif

	if(clothDesc.flags & NX_CLF_TEARABLE) 
	{
		clothMeshDesc.flags |= NX_CLOTH_MESH_TEARABLE;
	}

	MemoryWriteBuffer clothMeshStream;

	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	bool cookok = false;
	if (cookingInterface != NULL)
	{
		cookok = cookingInterface->NxCookClothMesh(clothMeshDesc, clothMeshStream);
	}
	if(cookok)
	{
		NxPhysicsSDK* sdk = MxPluginData::getPhysicsSDKStatic();
		if (sdk == NULL) return NULL;
		MemoryReadBuffer clothReadStream(clothMeshStream.data);
		m_nxMesh = sdk->createClothMesh(clothReadStream);

		if(m_nxMesh)
		{
			//clothDesc.name = m_name.data();
			clothDesc.clothMesh  = m_nxMesh;
			clothDesc.globalPose = MxMathUtils::MaxMatrixToNx(maxNodeActor->PhysicsNodePoseTM); 
			ClothPose = clothDesc.globalPose;
			//if (gCurrentstream) gCurrentstream->printf("cloth position at creation: [%f, %f, %f]\n", clothDesc.globalPose.t.x, clothDesc.globalPose.t.y, clothDesc.globalPose.t.z);

			m_meshSimulate.alloc(maxNodeActor->SimpleMesh.numPoints * 2, maxNodeActor->SimpleMesh.numFaces * 2);   // to hold more points for broken cloth

			clothDesc.meshData.verticesPosBegin         = &m_meshSimulate.points[0].x;
			clothDesc.meshData.maxVertices              = m_meshSimulate.numPoints;
			clothDesc.meshData.numVerticesPtr           = &m_numVerts;
			clothDesc.meshData.verticesPosByteStride    = sizeof(NxVec3);

			clothDesc.meshData.verticesNormalBegin      = &m_meshSimulate.normals[0].x;
			clothDesc.meshData.verticesNormalByteStride = sizeof(NxVec3);

			clothDesc.meshData.indicesBegin             = m_meshSimulate.faces;
			clothDesc.meshData.maxIndices               = m_meshSimulate.numFaces * 3;
#if NX_SDK_VERSION_NUMBER >= 250
			clothDesc.meshData.numIndicesPtr            = &m_numIndices;
#else
			clothDesc.meshData.numTrianglesPtr          = &m_numTriangles;
#endif
			clothDesc.meshData.indicesByteStride        = sizeof(NxU32);

			bool checkValid = clothDesc.meshData.isValid();

			clothDesc.toFluidResponseCoefficient		 = 0.8f; 
			clothDesc.fromFluidResponseCoefficient		 = 0.8f;
			bool valid = clothDesc.isValid();
			NxScene* scene = MxPluginData::getSceneStatic();
			if (scene != NULL)
				m_nxCloth = scene->createCloth(clothDesc);

			if(m_nxCloth)
			{
				if(m_isMetalCloth)
				{
					// add metal cloth core
					int mode = 1, number_sphere = 1;
					float impulseThreshold = 0.0f, penetrationDepth = 0.0f, maxDeformationDistance = 0.0f;
					float coreMass = 1.0f;
					m_node->GetUserPropInt("px_mc_core_shape", mode);
					m_node->GetUserPropInt("px_mc_num_sphere", number_sphere);
					m_node->GetUserPropFloat("px_mc_core_mass", coreMass);
					m_node->GetUserPropFloat("px_mc_impulse_threshold", impulseThreshold);
					m_node->GetUserPropFloat("px_mc_penetration_depth", penetrationDepth);
					m_node->GetUserPropFloat("px_mc_max_deform_distance", maxDeformationDistance);
					NxActorDesc coreActorDesc;
					NxBodyDesc  coreBodyDesc;
					coreBodyDesc.mass = coreMass;		// if body mass is set to zero;
					coreBodyDesc.linearDamping = 0.2f;
					coreBodyDesc.angularDamping = 0.2f;
					coreActorDesc.body = &coreBodyDesc;
					bool createCore = true;
					if (mode == 1) { // sphere as core
						coreActorDesc.shapes.pushBack(new NxSphereShapeDesc());
					}
					else if (mode == 2) { // capsule as core
						coreActorDesc.shapes.pushBack(new NxCapsuleShapeDesc());
					}
					else if (mode == 3) { // box as core
						coreActorDesc.shapes.pushBack(new NxBoxShapeDesc());
					}
					else if (mode == 4) { // compound of spheres as core
						createCore = number_sphere > 0;
						for (NxU32 i = 0; i < number_sphere; i++) 
							coreActorDesc.shapes.pushBack(new NxSphereShapeDesc());
					}
					else
					{
						createCore = false;
					}
					if(createCore)
					{
						m_coreActor = scene->createActor(coreActorDesc);
						m_nxCloth->attachToCore(m_coreActor, impulseThreshold, penetrationDepth, maxDeformationDistance); 
						// Clean up allocations
						for (NxU32 i = 0; i < coreActorDesc.shapes.size(); i++)
							delete coreActorDesc.shapes[i];
					}
				}

				m_nxCloth->userData = this;
				BOOL autoattach = FALSE;
				if(m_node->GetUserPropBool("px_cll_autoattach", autoattach) && autoattach)
				{
#if NX_SDK_VERSION_NUMBER >= 250
					m_nxCloth->attachToCollidingShapes(NX_CLOTH_ATTACHMENT_TWOWAY);
#endif
				}
			}
		}

		//Make a backup copy of the mesh (for reset-purposes)
		BOOL needDel = FALSE;
		TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, ccMaxWorld::MaxTime(), needDel);
		if (triObj != NULL)
		{
			Mesh& mesh = triObj->GetMesh();
			m_backupMesh = new Mesh(mesh);
			if(needDel)
				triObj->DeleteMe();
		}
	}
	return m_nxCloth;
}

//Can only update the 3ds max object if it is of mesh type (i.e. doesn't need to be converted into a mesh)
void MxCloth::UpdateMaxMesh(TimeValue t)
{
	//if (!m_updateTriMesh) return;
	if (m_node == NULL) return;

	int deleteIt = 0;
	TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, t, deleteIt);
	if(triObj)
	{
		if(!deleteIt)
		{
			Matrix3 tmp(true);
			NxMat34 pose = MxMathUtils::MaxMatrixToNx(tmp);
			maxNodeActor->SetSimulatedPose(pose, false);

			Mesh& mesh = triObj->mesh;
			mesh.setNumVerts(m_numVerts);
#if NX_SDK_VERSION_NUMBER > 240
			m_numTriangles = m_numIndices / 3;
#endif
			mesh.setNumFaces(m_numTriangles);
			for(NxU32 i = 0; i < m_numVerts; i++)
			{
				Point3 p = (Point3&)(m_meshSimulate.points[i]);
				p = p / ccMaxWorld::GetUnitChange();
				mesh.setVert(i, p);
			}
			for(NxU32 i = 0; i < m_numTriangles; i++)
			{
				mesh.faces[i].setVerts(m_meshSimulate.faces[i*3+0], m_meshSimulate.faces[i*3+1], m_meshSimulate.faces[i*3+2]);
				mesh.faces[i].setSmGroup(1);
			}
			UpdateMaxMeshState(mesh);
		}
		else
		{
			m_updateTriMesh = false; //Stop trying to update the trimesh
			//delete to;
			triObj->DeleteMe();
		}
	}
}

void MxCloth::Reset()
{
	if (m_node == NULL) return;
	if (m_backupMesh == NULL) return;

	//TimeValue t = GetCOREInterface()->GetTime();
	//m_node->SetNodeTM(t, m_originalPose);
	maxNodeActor->RestorePose();
	int needDel = 0;
	TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, 0, needDel);
	if (triObj)
	{
		if (needDel) //Can only reset objects if they already are of trimesh type
		{
			triObj->DeleteMe();
		}
		else
		{
			triObj->mesh = *m_backupMesh;
			UpdateMaxMeshState(triObj->mesh);
		}
	}
	create();
}

void MxCloth::UpdateMaxMeshState(Mesh& mesh)
{
	mesh.InvalidateStrips();
	mesh.InvalidateGeomCache();
	mesh.InvalidateTopologyCache();
	mesh.buildNormals();
	mesh.BuildStripsAndEdges();
	mesh.buildBoundingBox();
	m_node->InvalidateTreeTM();
	m_node->InvalidateTM();
	m_node->InvalidateWS();
}

void MxCloth::InitClothDescProps(NxClothDesc& desc)
{
	NxScene* scene = MxPluginData::getSceneStatic();

	FlagHolder flagtable[] =
	{
		FlagHolder("px_clf_pressure"           , NX_CLF_PRESSURE),
		FlagHolder("px_clf_static"             , NX_CLF_STATIC),
		FlagHolder("px_clf_disable_collision"  , NX_CLF_DISABLE_COLLISION),
		FlagHolder("px_clf_selfcollision"      , NX_CLF_SELFCOLLISION),
		FlagHolder("px_clf_gravity"            , NX_CLF_GRAVITY),
		FlagHolder("px_clf_bending"            , NX_CLF_BENDING),
		FlagHolder("px_clf_bending_ortho"      , NX_CLF_BENDING_ORTHO),
		FlagHolder("px_clf_damping"            , NX_CLF_DAMPING),
		FlagHolder("px_clf_comdamping"         , NX_CLF_COMDAMPING),
		FlagHolder("px_clf_collision_twoway"   , NX_CLF_COLLISION_TWOWAY),
		FlagHolder("px_clf_triangle_collision" , NX_CLF_TRIANGLE_COLLISION),
		FlagHolder("px_clf_tearable"           , NX_CLF_TEARABLE),
		FlagHolder("px_clf_hardware"           , NX_CLF_HARDWARE),
		FlagHolder("px_clf_fluid_collision"           , NX_CLF_FLUID_COLLISION),
	};

	const NxU32 numFlags = sizeof(flagtable)/sizeof(flagtable[0]);
	desc.flags = 0;
	//bool hwSimulation = (scene!=NULL)?scene->getSimType()==NX_SIMULATION_HW:false;
	for (NxU32 i = 0; i < numFlags; i++)
	{
		BOOL val = FALSE;
		if(m_node->GetUserPropBool(flagtable[i].name, val))
		{
			if(val)
			{
				desc.flags |= flagtable[i].flagValue;
			}
		}
	}

	if(!MxPluginData::HwAvailable()) desc.flags &= ~NX_CLF_HARDWARE;

	InitFloatProp(desc.thickness,                     "px_clp_thickness");
	InitFloatProp(desc.density,                       "px_clp_density");
	InitFloatProp(desc.bendingStiffness,              "px_clp_bendingStiffness");
	InitFloatProp(desc.stretchingStiffness,           "px_clp_stretchingStiffness");
	InitFloatProp(desc.dampingCoefficient,            "px_clp_dampingCoefficient");
	InitFloatProp(desc.friction,                      "px_clp_friction");
	InitFloatProp(desc.pressure,                      "px_clp_pressure");
	InitFloatProp(desc.tearFactor,                    "px_clp_tearFactor");
	InitFloatProp(desc.collisionResponseCoefficient,  "px_clp_collisionResponseCoef");
	InitFloatProp(desc.attachmentResponseCoefficient, "px_clp_attachResponseCoef");
	InitIntProp(  desc.solverIterations,              "px_clp_solverIterations");
}

void MxCloth::InitFloatProp(NxF32& f, const char* name)
{
	float v;
	int ret = m_node->GetUserPropFloat(name, v);
	if (ret != 0) f = (NxF32)v;
}

void MxCloth::InitIntProp(NxU32& i, const char* name)
{
	int v;
	if(m_node->GetUserPropInt(name, v))
	{
		i = (NxU32)v;
	}
}
