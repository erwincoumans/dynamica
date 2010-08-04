//#include <NxPhysics.h>
#include "MxUtils.h"

//#include <NxCooking.h>
#include <max.h>
#include <MAXScrpt\MAXScrpt.h>

#include "PxPlugin.h"
#include "PxStream.h"
#include "PxFunctions.h"

#include "MxMesh.h"
#include "MxUtils.h"

extern CharStream* gCurrentstream;

MxMesh::MxMesh(const char* name, INode* node, MxMeshType type) : MxObject(name, node), m_type(type), m_object(NULL)
{
	m_ObjectType = MX_OBJECTTYPE_MESH;
	m_triMesh = NULL;
	m_convexMesh = NULL;
	if (type == MX_MESHTYPE_TRIMESH || type == MX_MESHTYPE_CLOTH || type == MX_MESHTYPE_SOFTBODY || type == MX_MESHTYPE_TRIMESH_SW)
	{
		initTriMesh();
	} else if (type == MX_MESHTYPE_CONVEX)
	{
		initConvexMesh();
	} else 
	{
		//error!
	}
}

void MxMesh::initTriMesh()
{
	//TODO: handle scaling? (needed here?)
#if 0
	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	if (cookingInterface == NULL)
	{
		return;
	}

	MxTriangleMesh* tmd = NULL;

	TimeValue t = GetCOREInterface()->GetTime();
	BOOL needDel = FALSE;
	TriObject* tri = MxUtils::GetTriObjectFromNode(m_node, t, needDel);
	if (tri == NULL) {
		return;
	}
	Mesh& mesh = tri->GetMesh();

	tmd = new MxTriangleMesh();
	m_object = m_node->GetObjectRef();
	tmd->numVertices  = mesh.getNumVerts();
	tmd->points    =  new NxVec3[tmd->numVertices];
	for(NxU32 i = 0; i < tmd->numVertices; i++) 
	{
		((Point3*)tmd->points)[i] = mesh.verts[i]; 
	}
	tmd->pointStrideBytes = sizeof(Point3);
	tmd->triangleStrideBytes = sizeof(int)*3;
	int triangles_count   = mesh.getNumFaces();
	int* triangles = new int[triangles_count*3];
	for(NxU32 i = 0; i < triangles_count; i++) 
		for(NxU32 j = 0; j < 3; j++) 
		{ 
			triangles[i*3+j] = mesh.faces[i].v[j];
		}
	tmd->triangles = triangles;
	tmd->numTriangles = triangles_count;

	MemoryWriteBuffer cookresult;
	bool status = cookingInterface->NxCookTriangleMesh(*tmd,cookresult);
	if (needDel) tri->DeleteMe();
	delete []tmd->points; tmd->points=NULL;
	delete []triangles;   tmd->triangles=NULL;
	if(!status) 
	{
		gCurrentstream->printf("ERROR: failed to cook trimesh %s\n",m_node->GetName());
		return;
	}
	tmd->m_nxTriangleMesh = gPluginData->getPhysicsSDK()->createTriangleMesh(MemoryReadBuffer(cookresult.data));
	if(!tmd->m_nxTriangleMesh)
	{
		gCurrentstream->printf("ERROR: failed to create trimesh %s\n", m_node->GetName());
		return;
	}
	tmd->m_nxTriangleMesh->saveToDesc(*tmd); //read back the triangle mesh to the descriptor
	m_triMesh = tmd;
#endif

}

void MxMesh::initConvexMesh()
{
#if 0
	MxConvexMesh* cmd = NULL;

	//mesh sharing is handled by MxPluginData::createMesh()

	BOOL needDel;
	TriObject* tri = MxUtils::GetTriObjectFromNode(m_node, 0, needDel);
	if (!tri) {
		return;
	}
	bool needDelete = false;
	Mesh *mesh=&tri->GetMesh();
	if(mesh->getNumFaces() > 255)
	{
		// physics does not allow a convex ploygon with more than 255 faces.
		NxPhysicsSDK* sdk = MxPluginData::getPhysicsSDKStatic();
		int faceNum = 32;
		if(sdk)
		{
			if (MxPluginData::HwAvailable()) faceNum = 255;
		}
		mesh = PxFunctions::pxCreateHull(mesh, faceNum, 0.0025f);
		needDelete = true;
		faceNum = mesh->getNumFaces();  // test it
		faceNum = mesh->getNumVerts();  // test it
	}

	cmd = new MxConvexMesh();
	m_object = m_node->GetObjectRef();
	cmd->numVertices  = mesh->getNumVerts();
	cmd->points    =  new NxVec3[cmd->numVertices];

	for(NxU32 i = 0; i < cmd->numVertices; i++) 
	{ 
		((Point3*)cmd->points)[i] = mesh->verts[i]; 
	}
	cmd->pointStrideBytes = sizeof(Point3);
	cmd->flags  =NX_CF_COMPUTE_CONVEX;
	cmd->triangles=NULL;
	cmd->triangleStrideBytes = sizeof(int)*3;
	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	bool status = false;
	MemoryWriteBuffer cookresult;
	if (cookingInterface != NULL)
	{
		status = cookingInterface->NxCookConvexMesh(*cmd,cookresult);
	}
	delete []cmd->points; cmd->points=NULL;
	if(needDelete)
	{
		delete mesh;
		mesh = NULL;
	}
	if(!status) 
	{
		gCurrentstream->printf("ERROR: failed to cook convex mesh %s\n",m_node->GetName());
		delete cmd;
		return;
	}
	cmd->m_nxConvexMesh = gPluginData->getPhysicsSDK()->createConvexMesh(MemoryReadBuffer(cookresult.data));
	if(!cmd->m_nxConvexMesh)
	{
		gCurrentstream->printf("ERROR: failed to create convex mesh %s\n",m_node->GetName());
		delete cmd;
		return;
	}
	cmd->m_nxConvexMesh->saveToDesc(*cmd); //read back the settings to the descriptor
	m_convexMesh = cmd;
#endif

}

MxMesh::~MxMesh() 
{
#if 0
	if (m_type == MX_MESHTYPE_TRIMESH || m_type == MX_MESHTYPE_CLOTH || m_type == MX_MESHTYPE_SOFTBODY || m_type == MX_MESHTYPE_TRIMESH_SW)
	{
		if (m_triMesh != NULL)
		{
			if (m_triMesh->m_nxTriangleMesh != NULL)
			{
				gPluginData->getPhysicsSDK()->releaseTriangleMesh(*m_triMesh->m_nxTriangleMesh);
				m_triMesh->m_nxTriangleMesh = NULL;
			}
			delete m_triMesh;
			m_triMesh = NULL;
		}
	}
	if (m_type == MX_MESHTYPE_CONVEX)
	{
		if (m_convexMesh != NULL)
		{
			if (m_convexMesh->m_nxConvexMesh != NULL)
			{
				gPluginData->getPhysicsSDK()->releaseConvexMesh(*m_convexMesh->m_nxConvexMesh);
				m_convexMesh->m_nxConvexMesh = NULL;
			}
			delete m_convexMesh;
			m_convexMesh = NULL;
		}
	}
#endif
}

bool MxMesh::canInstantiate(INode* node, MxMeshType type)
{
	//I had a look at instancing in 3ds max:
	//1. Instance node1 -> node2
	//node1.os.obj == node1.getObjectRef() == node2.os.obj == node2.getObjectRef()
	//
	//2. Reference node1 -> node2
	//node1.os.obj == node1.getObjectRef() == node1.os.obj != node2.getObjectRef()

	//TODO: handle scaling - have to check that the scaling is the same in the two nodes

	if (m_type != type) return false;
	if (node == NULL) return false;
	if (!PxFunctions::mSetting_exploitinstances) return false;

	if (node->GetObjectRef() == m_object)
	{
		//TODO: also check scaling
		return true;
	}

	return false;
}
