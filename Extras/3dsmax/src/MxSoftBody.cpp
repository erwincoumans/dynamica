#include "PxPlugin.h"
#ifdef PXPLUGIN_SOFTBODY

#include <NxPhysics.h>
#include <NxCooking.h>

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>

#include "NXU_Streaming.h"


#include "MxUtils.h"
#include "MxPluginData.h"
#include "MxObjects.h"
#include "MxSoftBody.h"

#include "NxTetra.h"

class SoftBodyBuffers
{
public:
	SoftBodyBuffers(NxSoftBodyMeshDesc& desc, int maxTearing=2)
	{
		NxU32 numVertices  = desc.numVertices;
		NxU32 numTetra     = desc.numTetrahedra;

		assert(maxTearing >= 1 && maxTearing <= 12);
		NxU32 maxVertices  = maxTearing * numVertices;
		m_meshData.verticesPosBegin = new NxVec3[maxVertices];
		m_meshData.verticesPosByteStride = sizeof(NxVec3);
		m_meshData.maxVertices = maxVertices;
		m_meshData.numVerticesPtr = &m_numVertices;
		m_numVertices = 0;

		// the number of tetrahedra is constant, even if the softbody is torn
		NxU32 maxIndices = 4*numTetra;
		m_meshData.indicesBegin = new NxU32[maxIndices];
		m_meshData.indicesByteStride = sizeof(NxU32);
		m_meshData.maxIndices = maxIndices;
		m_meshData.numIndicesPtr = &m_numIndices;
		m_numIndices = 0;
	}

	~SoftBodyBuffers()
	{
		if (m_meshData.verticesPosBegin != NULL)
			delete[] m_meshData.verticesPosBegin;
		m_meshData.verticesPosBegin = NULL;

		if (m_meshData.indicesBegin != NULL)
			delete[] m_meshData.indicesBegin;
		m_meshData.indicesBegin = NULL;
	}

	NxMeshData m_meshData;
	NxU32 m_numVertices;
	NxU32 m_numIndices;
};

class SoftBodyTetrahedron
{
public:
	unsigned int orgVerts[4];
	unsigned int orgFaces[4];
	unsigned int newVerts[4];
	unsigned int triangles[4][3];
};

class SoftBodyMeshData
{
public:
	SoftBodyMeshData()
	{
		m_tetras = new NxArray<SoftBodyTetrahedron>();
		m_verts = new NxArray<NxVec3>();
	}

	~SoftBodyMeshData()
	{
		if (m_tetras) delete m_tetras;
		m_tetras = NULL;
		if (m_verts) delete m_verts;
	}

	NxArray<SoftBodyTetrahedron>& getTetraArray() { return *m_tetras; }
	NxArray<NxVec3>& getVertArray() { return *m_verts; }

	NxArray<SoftBodyTetrahedron>* m_tetras;
	NxArray<NxVec3>* m_verts;
};

class VertFinder {
public:
	static unsigned int findOrInsertVert(NxArray<NxVec3>& currentVerts, NxVec3 vert)
	{
		float tolerance = FLT_EPSILON*10.0f;
		for (unsigned int i = 0; i < currentVerts.size(); i++)
		{
			NxVec3 v = currentVerts[i];
			if (  (fabsf(v.x - vert.x) < tolerance) && (fabsf(v.y - vert.y) < tolerance) && (fabsf(v.y - vert.y) < tolerance))
			{
				return i;
			}
		}
		currentVerts.pushBack(vert);
		return currentVerts.size()-1;
	}

	static void findAllFacesUsingVertex(Mesh& mesh, NxArray<int>& faceStack, int currentVertex, bool* faceUsed)
	{
		/*
		//store the mesh
		FILE* fp = fopen("C:\\mesh01.txt", "w+");
		if(fp){
			fprintf(fp, "mesh.faces[i].getVert(j)\n");
			for (int i = 0; i < mesh.numFaces; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					fprintf(fp, "%d ", mesh.faces[i].getVert(j));
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
		}
		fp = fopen("C:\\mesh02.txt", "w+");
		if(fp){
			fprintf(fp, "mesh.faces[i].v[j]\n");
			for (int i = 0; i < mesh.numFaces; i++)
			{
				for (int j = 0; j < 3; j++)
				{
					fprintf(fp, "%d ", mesh.faces[i].v[j]);
				}
				fprintf(fp, "\n");
			}
			fclose(fp);
		}
		*/
		//
		for (int i = 0; i < mesh.numFaces; i++)
		{
			if (i == currentVertex) continue;
			if (faceUsed[i]) continue;
			for (int j = 0; j < 3; j++)
				if (mesh.faces[i].getVert(j) == currentVertex)
				{
					faceUsed[i] = true;
					faceStack.pushBack(i);
				}
		}
	}

	static void findAllConnectedFaces(Mesh& mesh, NxArray<int>& faceStack, bool* faceUsed)
	{
		for (int i = 0; i < faceStack.size(); i++) //automatically adjusts for new inserted faces
		{
			int currentFace = faceStack[i];
			for (int j = 0; j < 3; j++)
			{
				findAllFacesUsingVertex(mesh, faceStack, mesh.faces[currentFace].getVert(j), faceUsed);
			}
		}
	}
};

MxSoftBody::MxSoftBody(const char* name, INode* node) : MxObject(name, node), 
	m_receiveBuffers(NULL), m_softBodyData(NULL), m_nxSoftBody(NULL), m_nxSoftBodyMesh(NULL), m_compartment(NULL), maxNodeActor(0)
{
	m_ObjectType = MX_OBJECTTYPE_SOFTBODY;
	if (node == NULL) return;

	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = m_node->EvalWorldState(t); 
	int sid = os.obj->SuperClassID();

	//Check to see if the input mesh is a tetrahedra mesh
	if (!GetBoolProp("px_sb_meshistetra", true))
	{
		int subDivision = GetIntProp("px_sb_subdivisionlevel", 20);
		bool createIso = GetBoolProp("px_sb_createisosurface", true);
		bool singleIso = GetBoolProp("px_sb_createsingleisosurface", false);
		float simplification = GetFloatProp("px_sb_surfacesimplificationfactor", 0.5f);
		Mesh* mesh = createSoftBodyMesh(node, false, subDivision, simplification, createIso, singleIso);
		if (mesh != NULL)
		{
			BuildSoftBodyFromMesh(*mesh);
			delete mesh;
		}
	} else
	{
		BOOL needDel = FALSE;
		TriObject* triObj = NULL;
		triObj = MxUtils::GetTriObjectFromNode(node, t, needDel);

		if (triObj == NULL)
		{
			if (gCurrentstream) gCurrentstream->printf("Error: Unable to convert \"%s\" into a trimesh object, can't create a SoftBody out of it.\n", node->GetName());
			return;
		}

		//if (!needDel) m_updateTriMesh = true; //can update the mesh if the object is a trimesh

		//Create the NxSoftBody
		Mesh& mesh = triObj->GetMesh();
		m_backupMesh = mesh;
		BuildSoftBodyFromMesh(mesh);

		if (needDel && (triObj != NULL))
			triObj->DeleteMe();
	}
}

void MxSoftBody::Free(bool deletingObject)
{
	if (deletingObject)
	{
//		if (m_backupMesh != NULL)
//		{
//			delete m_backupMesh;
//			m_backupMesh = NULL;
//		}
	}

	if (m_nxSoftBody != NULL)
	{
		NxScene* scene = MxPluginData::getSceneStatic();
		if (scene != NULL)
		{
			scene->releaseSoftBody(*m_nxSoftBody);
		}
		m_nxSoftBody = NULL;
	}
	if (m_nxSoftBodyMesh != NULL)
	{
		NxPhysicsSDK* sdk = MxPluginData::getPhysicsSDKStatic();
		if (sdk != NULL)
		{
			sdk->releaseSoftBodyMesh(*m_nxSoftBodyMesh);
		}
		m_nxSoftBodyMesh = NULL;
	}
	if (m_compartment != NULL)
	{
		gPluginData->releaseCompartment(m_compartment);
		m_compartment = NULL;
	}
	if (m_softBodyData != NULL)
		delete m_softBodyData;
	m_softBodyData = NULL;
	if (m_receiveBuffers != NULL)
		delete m_receiveBuffers;
	m_receiveBuffers = NULL;
}

MxSoftBody::~MxSoftBody() 
{
	Free(true);
}

//Can only update the 3ds max object if it is of mesh type (i.e. doesn't need to be converted into a mesh)
void MxSoftBody::updateMeshFromSimulation(TimeValue t)
{
	//if (!m_updateTriMesh) return;
	if (m_node == NULL) return;
	int deleteIt = 0;
	TriObject* triObj = MxUtils::GetTriObjectFromNode(m_node, t, deleteIt);
	if(triObj)
	{
		if(!deleteIt)
		{
			NxSoftBodyDesc softBodyDesc;
			m_nxSoftBody->saveToDesc(softBodyDesc);
			m_node->SetNodeTM(t,MxMathUtils::NxMatrixToMax(softBodyDesc.globalPose));

			Mesh& mesh = triObj->mesh;
			m_tetraHelper.updateTetraLinks(m_receiveBuffers->m_meshData);

			Mesh& meshNx = m_tetraHelper.getMesh();
			for(NxU32 i = 0; i < mesh.numVerts; i++)
			{
				const Point3& pNx = meshNx.getVert(i);
				NxVec3 vNx(pNx.x, pNx.y, pNx.z);
				NxVec3 v;
				m_nxInvNodeTM.multiply(vNx, v);
				mesh.setVert(i, MxMathUtils::NxVec3ToPoint3(v));
			}
			UpdateMaxMeshState(mesh);
		}
		else
		{
			//delete to;
			triObj->DeleteMe();
		}
	}
}

void MxSoftBody::resetObject()
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
		else
		{
			triObj->mesh = m_backupMesh;
			BuildSoftBodyFromMesh(triObj->mesh);
			UpdateMaxMeshState(triObj->mesh);
		}
	}
}

void MxSoftBody::UpdateMaxMeshState(Mesh& mesh)
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

bool MxSoftBody::BuildSoftBodyFromMesh(Mesh& inputMesh)
{
	//Delete existing softbody (this method can be called from resetObject())
	Free(false);
	bool is32Bits = true;
	BOOL val = FALSE;
	if(m_node->GetUserPropBool("px_sbmf_16bit", val))
	{
		if (val) is32Bits = false;
	}

	// majun begin
	TimeValue t           = GetCOREInterface()->GetTime();

	m_nodeTM = m_node->GetObjectTM(t);
	NxMat34 nxNodeTM = MxMathUtils::MaxMatrixToNx(m_nodeTM);
	Matrix3 actorinv = Inverse(m_nodeTM);
	m_nxInvNodeTM = MxMathUtils::MaxMatrixToNx(actorinv);
	Mesh& meshNx = inputMesh;
	/*
	Mesh meshNx(inputMesh);
	NxVec3 nxPos;
	for(NxU32 i = 0; i < inputMesh.numVerts; i++)
	{
		NxVec3 nxVert = MxMathUtils::Point3ToNxVec3(inputMesh.getVert(i));
		nxNodeTM.multiply(nxVert, nxPos);
		meshNx.setVert(i, nxPos.x, nxPos.y, nxPos.z);
	}
	*/
	// majun end

	NxPhysicsSDK* sdk = MxPluginData::getPhysicsSDKStatic();
	if (sdk == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: Unable to get hold of the PhysX SDK\n");
		return false;
	}
	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	if (cookingInterface == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: Unable to find the NxCookingInterface (make sure that PhysXCooking.dll can be found by 3dsmax.exe)\n");
		return false;
	}
	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Error: There is no scene created yet, can't create a SoftBody\n");
		return false;
	}


	NxTetraInterface* tetraInterface = getNxTetraInterface();
	if (tetraInterface != NULL)
	{
		//First, check to see that we have unique vertices (we only support meshes created of stand-alone tetrahedrons at the moment)
		//None of this code is very fast, or even robust..

		//Keep track of which faces have already been used
		bool* faceUsed = new bool[meshNx.numFaces];
		for (int i = 0; i < meshNx.numFaces; i++)
		{
			faceUsed[i] = false;
		}

		//Resulting softbody mesh
		m_softBodyData = new SoftBodyMeshData();
		NxArray<SoftBodyTetrahedron>& tetras = m_softBodyData->getTetraArray();
		NxArray<NxVec3> newVerts = m_softBodyData->getVertArray();

		//temp variables used while building the softbody mesh
		NxArray<int> faceStack;
		NxArray<int> vertStack;

		//keep track of the number of tetrahedra that failed in building
		int numInvalid = 0;

		//iterate over all faces in the mesh and try to build tetrahedra
		for (int i = 0; i < meshNx.numFaces; i++)
		{
			if (faceUsed[i]) continue;
			faceStack.clear();
			faceUsed[i] = true;
			faceStack.pushBack(i);

			VertFinder::findAllConnectedFaces(meshNx, faceStack, faceUsed);

			if (faceStack.size() != 4)
			{
				//invalid part of the mesh
				numInvalid++;
				continue;
			}

			//Found 4 faces, try to 
			SoftBodyTetrahedron currentTetra;
			vertStack.clear();
			for (int j = 0; j < 4; j++)
			{
				currentTetra.orgFaces[j] = faceStack[j];
				for (int k = 0; k < 3; k++)
				{
					int vert = meshNx.faces[currentTetra.orgFaces[j]].getVert(k);
					bool alreadyIncluded = false;
					for (int m = 0; m < vertStack.size(); m++)
					{
						if (vertStack[m] == vert)
						{
							alreadyIncluded = true;
							break;
						}
					}
					if (!alreadyIncluded) vertStack.pushBack(vert);
				}
			}
			if (vertStack.size() != 4)
			{
				//invalid part of the mesh
				numInvalid++;
				continue;
			}

			for (int j = 0; j < 4; j++)
			{
				currentTetra.orgVerts[j] = vertStack[j];
				currentTetra.newVerts[j] = VertFinder::findOrInsertVert(newVerts, MxMathUtils::Point3ToNxVec3(meshNx.verts[vertStack[j]]));
			}

			//build triangles
			for (int j = 0; j < 4; j++)
			{
				Face& face = meshNx.faces[currentTetra.orgFaces[j]];
				for (int k = 0; k < 3; k++)
				{
					int orgVert = face.getVert(k);
					for (int m = 0; m < 4; m++)
					{
						if (currentTetra.orgVerts[m] == orgVert)
							currentTetra.triangles[j][k] = currentTetra.newVerts[m];
					}
				}
			}
			tetras.pushBack(currentTetra);
		}
		delete[] faceUsed;

		if (numInvalid > 0)
		{
			if (gCurrentstream) gCurrentstream->printf("Warning: There were %d invalid tetrahedra in the input mesh\n", numInvalid);
		}

		if (tetras.size() < 1)
		{
			if (gCurrentstream) gCurrentstream->printf("Error: Unable to create any tetrahedra from the supplied object\n");
			return false;
		}

		NxU32 tetraCount = tetras.size();

		NxSoftBodyMeshDesc meshDesc;

		meshDesc.numVertices            = newVerts.size();
		meshDesc.numTetrahedra          = tetraCount;
		meshDesc.vertexStrideBytes      = sizeof(NxVec3);
		meshDesc.vertices               = &newVerts[0].x;
		meshDesc.flags					= 0;

		if (is32Bits)
			meshDesc.tetrahedronStrideBytes = sizeof(NxU32)*4;
		else
			meshDesc.tetrahedronStrideBytes = sizeof(NxU16)*4;

		//If we want to tear the softbody in HW:
		//NX_SOFTBODY_MESH_TEARABLE

		val = FALSE;
		if (m_node->GetUserPropBool("px_sbmf_tearable", val))
		{
			if (val)
				meshDesc.flags |= NX_SOFTBODY_MESH_TEARABLE;
		}
		if(is32Bits)
		{
			NxU32* indices = new NxU32[tetraCount*4];
			for (int i = 0; i < tetraCount; i++)
			{
				for (int j = 0; j < 4; j++)
					indices[i*4+j] = tetras[i].newVerts[4-j]; //reverse the order of the indices, it seems that we were creating softbodies inside-out
			}
			meshDesc.tetrahedra             = indices;
		}
		else
		{
			meshDesc.flags |= NX_SOFTBODY_MESH_16_BIT_INDICES;

			NxU16* indices = new NxU16[tetraCount*4];
			for (int i = 0; i < tetraCount; i++)
			{
				for (int j = 0; j < 4; j++)
					indices[i*4+j] = tetras[i].newVerts[4-j]; //reverse the order of the indices, it seems that we were creating softbodies inside-out
			}
			meshDesc.tetrahedra             = indices;
		}

		meshDesc.vertexMassStrideBytes  = 0;
		meshDesc.vertexFlagStrideBytes  = 0;
		meshDesc.vertexMasses           = 0;
		meshDesc.vertexFlags            = 0;

		val = FALSE;
		if (m_node->GetUserPropBool("px_sbmp_vertex", val))
			if (val)
				meshDesc.flags |= NX_SOFTBODY_VERTEX_TEARABLE;

		NXU::ImportMemoryWriteBuffer writeBuffer;
		bool result = cookingInterface->NxCookSoftBodyMesh(meshDesc, writeBuffer);

		if (!result)
		{
			if (gCurrentstream) gCurrentstream->printf("Error: Unable to cook the SoftBody mesh\n");
			return 0;
		}

		NXU::ImportMemoryReadBuffer importMemoryReadBuffer(writeBuffer.data);

		m_nxSoftBodyMesh = sdk->createSoftBodyMesh(importMemoryReadBuffer);

		//////////////////////////////////////////////////////////////////////////

		NxSoftBodyDesc softBodyDesc;
		TimeValue t = GetCOREInterface()->GetTime();
		softBodyDesc.globalPose = nxNodeTM;

		softBodyDesc.volumeStiffness = GetFloatProp("px_sbp_volumestiffness", 1.0f);
		softBodyDesc.stretchingStiffness = GetFloatProp("px_sbp_stretchingstiffness", 1.0f);
		softBodyDesc.friction = GetFloatProp("px_sbp_friction", 0.5f);
		softBodyDesc.attachmentResponseCoefficient = GetFloatProp("px_sbp_attachmentresponse", 0.2f);
		softBodyDesc.density = GetFloatProp("px_sbp_density", 1.0f);
		softBodyDesc.dampingCoefficient = GetFloatProp("px_sbp_damping", 0.5f);
		softBodyDesc.particleRadius = GetFloatProp("px_sbp_particleradius", 0.1f);
		softBodyDesc.tearFactor = GetFloatProp("px_sbp_tearfactor", 1.5f);
		softBodyDesc.collisionResponseCoefficient = GetFloatProp("px_sbp_collisionresponse", 0.2f);
		softBodyDesc.attachmentTearFactor = GetFloatProp("px_sbp_attachmenttearfactor", 1.5f);
		softBodyDesc.toFluidResponseCoefficient = GetFloatProp("px_sbp_tofluidresponse", 1.0f);
		softBodyDesc.fromFluidResponseCoefficient = GetFloatProp("px_sbp_fromfluidresponse", 1.0f);
#if NX_SDK_VERSION_NUMBER >= 272
		softBodyDesc.relativeGridSpacing = GetFloatProp("px_sbp_relativegridspacing", 0.25f);
#endif
		softBodyDesc.solverIterations = GetIntProp("px_sbp_solveriterations", 5);

		softBodyDesc.flags = 0;
		FlagHolder softbodyFlagTable[] =
		{
			FlagHolder("px_sbf_static",				NX_SBF_STATIC),
			FlagHolder("px_sbf_disablecollision",	NX_SBF_DISABLE_COLLISION),
			FlagHolder("px_sbf_selfcollision",		NX_SBF_SELFCOLLISION),
			FlagHolder("px_sbf_visualization",		NX_SBF_VISUALIZATION),
			FlagHolder("px_sbf_gravity",			NX_SBF_GRAVITY),
			FlagHolder("px_sbf_volumeconservation",	NX_SBF_VOLUME_CONSERVATION),
			FlagHolder("px_sbf_damping",			NX_SBF_DAMPING),
			FlagHolder("px_sbf_twowaycollision",	NX_SBF_COLLISION_TWOWAY),
			FlagHolder("px_sbf_tearable",			NX_SBF_TEARABLE),
			FlagHolder("px_sbf_hardware",			NX_SBF_HARDWARE),
			FlagHolder("px_sbf_comdamping",			NX_SBF_COMDAMPING),
			FlagHolder("px_sbf_validbounds",		NX_SBF_VALIDBOUNDS),
			FlagHolder("px_sbf_fluidcollision",		NX_SBF_FLUID_COLLISION)
		};
		const NxU32 numFlags = sizeof(softbodyFlagTable)/sizeof(FlagHolder);
		//bool hwSimulation = (scene!=NULL)?scene->getSimType()==NX_SIMULATION_HW:false;
		for (NxU32 i = 0; i < numFlags; i++)
		{
			BOOL val = FALSE;
			if (m_node->GetUserPropBool(softbodyFlagTable[i].name, val))
			{
				if (val)
				{
					softBodyDesc.flags |= softbodyFlagTable[i].flagValue;
				}
			}
		}

		if(!MxPluginData::HwAvailable()) softBodyDesc.flags &= ~NX_SBF_HARDWARE;

		softBodyDesc.softBodyMesh = m_nxSoftBodyMesh;

		m_receiveBuffers = new SoftBodyBuffers(meshDesc, 2);
		softBodyDesc.meshData = m_receiveBuffers->m_meshData;


		//Compartment
		softBodyDesc.compartment = NULL;
		int compartmentID = GetIntProp("px_sbp_compartment", 0);
		if (compartmentID != 0)
		{
			m_compartment = gPluginData->createCompartment(NX_SCT_SOFTBODY, compartmentID);
			if (m_compartment != NULL)
			{
				softBodyDesc.compartment = m_compartment->getNxCompartment();
			}
		}

		m_nxSoftBody = scene->createSoftBody(softBodyDesc);
		if (m_nxSoftBody != NULL)
		{
			m_nxSoftBody->userData = this;
			NxVec3 gravity;
			scene->getGravity(gravity);
			//m_nxSoftBody->setExternalAcceleration(gravity);

			// majun tetra helper
			m_tetraHelper.setMesh(meshNx);
			m_tetraHelper.updateBounds();
			m_tetraHelper.buildTetraLinks(meshDesc);
		}
	} //tetrainterface != NULL
	return false;
}

Mesh* MxSoftBody::createSoftBodyMesh(INode* node, bool createEditableMesh, int subDivision, float simplificationFactor, bool createIsoSurface, bool singleIsoSurface)
{
	if (node == NULL) return NULL;

	NxTetraInterface* tetraInterface = getNxTetraInterface();
	if (tetraInterface != NULL)
	{
		TimeValue t = GetCOREInterface()->GetTime();
		int needDelete = 0;
		TriObject* triObj = MxUtils::GetTriObjectFromNode(node, t, needDelete);
		if (triObj == NULL)
		{
			if (gCurrentstream) gCurrentstream->printf("Unable to convert \"%s\" to a triangle mesh, can't create a SoftBody mesh out of it.\n", node->GetName());
			return NULL;
		}
		Mesh& inputMesh = triObj->GetMesh();

		NxTetraMesh tetraMeshTmp;

		float* verts = new float[inputMesh.numVerts*3];
		unsigned int* indices = new unsigned int[inputMesh.numFaces*3];
		for (int i = 0; i < inputMesh.numVerts; i++)
		{
			verts[i*3+0] = inputMesh.verts[i].x;
			verts[i*3+1] = inputMesh.verts[i].y;
			verts[i*3+2] = inputMesh.verts[i].z;
		}
		for (int i = 0; i < inputMesh.numFaces; i++)
		{
			indices[i*3+0] = inputMesh.faces[i].v[0];
			indices[i*3+1] = inputMesh.faces[i].v[1];
			indices[i*3+2] = inputMesh.faces[i].v[2];
		}

		bool result = tetraInterface->createTetraMesh(tetraMeshTmp, inputMesh.numVerts, verts, inputMesh.numFaces, indices, false);
		delete[] verts;
		delete[] indices;

		if (needDelete)
			triObj->DeleteMe();
		if (!result)
		{
			if (gCurrentstream) gCurrentstream->printf("Unable to create a tetra mesh from the supplied mesh\n");
			return NULL;
		}

		NxTetraMesh tetraMesh;

		tetraInterface->setSubdivisionLevel(subDivision); //10
		/* 
		// original codes; it causes a crash bug TTP 7607, 7610,7625
		if (createIsoSurface) 
		{
			tetraInterface->createIsoSurface(tetraMeshTmp, tetraMesh, singleIsoSurface); //false
			tetraInterface->releaseTetraMesh(tetraMeshTmp);
		} else
		{
			tetraMesh = tetraMeshTmp;
			//don't release tetraMeshTmp, we just assigned it over to tetraMesh
		}
		tetraInterface->simplifySurface(simplificationFactor, tetraMesh, tetraMeshTmp); //0.5
		tetraInterface->releaseTetraMesh(tetraMesh);
		tetraInterface->createTetraMesh(tetraMeshTmp, tetraMesh);
		tetraInterface->releaseTetraMesh(tetraMeshTmp);
		*/
		// Ma Jun's temporary solution. Begin
		tetraInterface->createIsoSurface(tetraMeshTmp, tetraMesh, singleIsoSurface); //false
		tetraInterface->releaseTetraMesh(tetraMeshTmp);
		tetraInterface->simplifySurface(simplificationFactor, tetraMesh, tetraMeshTmp); //0.5
		tetraInterface->releaseTetraMesh(tetraMesh);
		tetraInterface->createTetraMesh(tetraMeshTmp, tetraMesh);
		tetraInterface->releaseTetraMesh(tetraMeshTmp);
		// Ma Jun's temporary solution. End

		//////////////////////////////////////////////////////////////////////////

		Mesh* mesh = new Mesh();
		int numVerts = tetraMesh.mVcount;
		int numTetras = tetraMesh.mTcount;
		int numFaces = tetraMesh.mTcount * 4;

		//Specifies if each tetrahedron should have its own unique vertices (so that the user easily can edit it)
		//bool createEditable = true;
		bool createEditable = createEditableMesh;
		if (createEditable)
		{
			numVerts = tetraMesh.mTcount * 4;
		}

		mesh->setNumVerts(numVerts);
		if (!createEditable)
		{
			for(int i = 0; i < numVerts; i++)
			{
				mesh->verts[i] = Point3(tetraMesh.mVertices[i*3+0], tetraMesh.mVertices[i*3+1], tetraMesh.mVertices[i*3+2]);
			}
		} else
		{
			for (int i = 0; i < numTetras; i++)
			{
				for (int j = 0; j < 4; j++)
					mesh->verts[i*4+j] = Point3(tetraMesh.mVertices[tetraMesh.mIndices[i*4+j]*3+0], tetraMesh.mVertices[tetraMesh.mIndices[i*4+j]*3+1], tetraMesh.mVertices[tetraMesh.mIndices[i*4+j]*3+2]);
			}
		}
		mesh->setNumFaces(numFaces);
		mesh->setNumVCFaces(numFaces);
		mesh->setNumVertCol(numTetras);

		static Point3 gColors[18]=
		{
			Point3(1.0f, 1.0f, 0.0f),
			Point3(0.0f, 1.0f, 1.0f),
			Point3(1.0f, 0.0f, 1.0f),
			Point3(1.0f, 0.0f, 0.0f),
			Point3(0.0f, 1.0f, 0.0f),
			Point3(0.0f, 0.0f, 1.0f),
			Point3(1.0f, 1.0f, 0.5f),
			Point3(0.5f, 1.0f, 1.0f),
			Point3(1.0f, 0.5f, 1.0f),
			Point3(1.0f, 0.5f, 0.5f),
			Point3(0.5f, 1.0f, 0.5f),
			Point3(0.5f, 0.5f, 1.0f),
			Point3(0.6f, 0.6f, 0.0f),
			Point3(0.0f, 0.6f, 0.6f),
			Point3(0.6f, 0.0f, 0.6f),
			Point3(0.6f, 0.0f, 0.0f),
			Point3(0.0f, 0.6f, 0.0f),
			Point3(0.0f, 0.0f, 0.6f),
		};

		for (int i = 0; i < numTetras; i++) 
		{
			mesh->vertCol[i] = gColors[i%18];
		}

		int faceNr = 0;
		unsigned int currentTetra[4];
		unsigned int tetraIndices[12] = 
		{	
			2,1,0,
			0,1,3,
			1,2,3,
			2,0,3
		};

		if (tetraMesh.mIsTetra) 
		{
			for (int i = 0; i < numTetras; i++)
			{
				if (createEditable)
				{
					//triangles
					for (int j = 0; j < 4; j++)
					{
						mesh->faces[faceNr].setVerts(i*4+tetraIndices[j*3+0], i*4+tetraIndices[j*3+1], i*4+tetraIndices[j*3+2]);
						mesh->faces[faceNr].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
						mesh->vcFace[faceNr].setTVerts(i,i,i);
						faceNr++;
					}

				} else {
					for (int j = 0; j < 4; j++)
						currentTetra[j] = tetraMesh.mIndices[i*4+j];

					//triangles
					for (int j = 0; j < 4; j++)
					{
						mesh->faces[faceNr].setVerts(currentTetra[tetraIndices[j*3+0]], currentTetra[tetraIndices[j*3+1]], currentTetra[tetraIndices[j*3+2]]);
						mesh->faces[faceNr].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
						mesh->vcFace[faceNr].setTVerts(i,i,i);
						faceNr++;
					}
				}
			}
		} else
		{
			for (int i = 0; i < numTetras; i++)
			{
				//triangles
				mesh->faces[i].setVerts(tetraMesh.mIndices[i*3+0], tetraMesh.mIndices[i*3+1], tetraMesh.mIndices[i*3+2]);
			}
		}

		tetraInterface->releaseTetraMesh(tetraMesh);

		mesh->InvalidateGeomCache();
		mesh->InvalidateTopologyCache();
		mesh->setVCDisplayData(MESH_USE_EXT_CVARRAY);
		mesh->buildBoundingBox();
		mesh->BuildStripsAndEdges();
		mesh->BuildVisEdgeList();
		mesh->buildNormals();

		return mesh;
	}

	if (gCurrentstream) gCurrentstream->printf("Unable to load the TetraMesh interface\n");
	return NULL;
}

float MxSoftBody::GetFloatProp(const char* name, float def)
{
	float v;
	if (MxUserPropUtils::GetUserPropFloat(m_node, name, v))
		return v;
	else
		return def;
}

int MxSoftBody::GetIntProp(const char* name, int def)
{
	int v;
	if (m_node->GetUserPropInt(name, v))
		return v;
	else
		return def;
}

bool MxSoftBody::GetBoolProp(const char* name, bool def)
{
	BOOL v = FALSE;
	if (m_node->GetUserPropBool(TSTR(name), v))
		return v == TRUE;
	else
		return def;
}

#endif //PXPLUGIN_SOFTBODY
