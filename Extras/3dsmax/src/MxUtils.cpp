
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

#include "MxUtils.h"
#include "MxObjects.h"
#include "MxPluginData.h"
//#include "MxShape.h"
#include "MxJoint.h"
#include "PxStream.h"
//#include <nxcooking.h>

extern CharStream* gCurrentstream;
struct MxSDKParam;
extern MxSDKParam* gSDKParamSettings;

///Bullet data structures, in PxFunctions.cpp
extern btDiscreteDynamicsWorld* gDynamicsWorld;
extern btCollisionConfiguration* gCollisionConfiguration;
extern btCollisionDispatcher* gDispatcher;
extern btBroadphaseInterface* gBroadphase;
extern btConstraintSolver* gConstraintSolver;

bool MxUtils::PreparePlugin()
{
	//should we remember that we failed the initialization, or try every time we are called?
	static bool setupFailed = false;
	if (gPluginData == NULL)
	{
		gPluginData = new MxPluginData(gCurrentstream);
		setupFailed = !gPluginData->init();
		if (setupFailed)
		{
			delete gPluginData;
			gPluginData = NULL;
		}
	}

	if (gDynamicsWorld == NULL)
	{

		gCollisionConfiguration= new btDefaultCollisionConfiguration();
		gDispatcher = new btCollisionDispatcher(gCollisionConfiguration);
		gBroadphase = new btDbvtBroadphase();
		gConstraintSolver = new btSequentialImpulseConstraintSolver();
		gDynamicsWorld = new btDiscreteDynamicsWorld(gDispatcher,gBroadphase,gConstraintSolver,gCollisionConfiguration);
	}

	return gPluginData != NULL;
}

bool MxUtils::ReleasePlugin(bool quitting)
{
	if (gPluginData)
	{
		delete gPluginData;
		gPluginData = NULL;
	}
	if (quitting)
	{
		if (gSDKParamSettings != NULL)
		{
			delete[] gSDKParamSettings;
			gSDKParamSettings = NULL;
		}
	}

	if (gDynamicsWorld)
	{
			delete gDynamicsWorld;
			gDynamicsWorld = NULL;
			delete gConstraintSolver;
			gConstraintSolver = NULL;
			delete gDispatcher;
			gDispatcher = NULL;
			delete gBroadphase;
			gBroadphase = NULL;
			delete gCollisionConfiguration;
			gCollisionConfiguration= NULL;
	}
	return true;
}

#if 0
NxShape* MxUtils::GetNxShapeFromName(const char* name) 
{
	if (gPluginData == NULL) return NULL;
	
	NxShape* nxShape = NULL;

	//NxArray<MxObject*> objects;
	////There can possibly be several objects with the same name, take the first shape found
	//MxPluginData::getObjectsFromName(name, objects);
	//for (NxU32 i = 0; i < objects.size(); i++)
	//{
	//	MxShape* mxShape = objects[i]->isShape();
	//	if (mxShape != NULL)
	//	{
	//		nxShape = mxShape->getNxShape();
	//		break;
	//	}
	//}

	NxScene*   s       = gPluginData->getScene();
	NxU32      na      = s->getNbActors();
	NxActor**  actors  = s->getActors();
	for(NxU32 i = 0; i < na; ++i)
	{
		NxActor*        actor   = actors[i];
		NxU32           ns      = actor->getNbShapes();
		NxShape*const*  shapes  = actor->getShapes();
		for(NxU32 j = 0; j < ns; ++j)
		{
			NxShape*      s     = shapes[j];
			if(stricmp(name, s->getName()) == 0)
				return s;
		}
	}

	return nxShape;
}

MxActor* MxUtils::GetActorFromNode(INode* node)
{
	if (node == NULL) return NULL;

	//There can possibly be several objects with the same name, take the first actor found
	NxArray<MxObject*> objects;
	MxPluginData::getObjectsFromNode(node, objects);
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		MxActor* mxActor = objects[i]->isActor();
		if (mxActor != NULL)
		{
			return mxActor;
		}
	}
	return NULL;
}



MxObject* MxUtils::GetFirstObjectOfTypeFromNode(INode* node, MxObjectType type)
{
	if (node == NULL) return NULL;

	//There can possibly be several objects with the same name, take the first actor found
	NxArray<MxObject*> objects;
	MxPluginData::getObjectsFromNode(node, objects);
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		if (objects[i]->isType(type)) return objects[i];
	}
	return NULL;
}




NxMaterial* MxUtils::findExistingMaterial(NxScene* scene, const NxMaterialDesc& desc)
{
	NxU32 number = scene->getNbMaterials();
	for(NxU32 i = 0; i < number; ++i)
	{
		NxMaterialDesc t;
		NxMaterial* m = scene->getMaterialFromIndex(i);
		m->saveToDesc(t);
		if(memcmp(&desc, &t, sizeof(t)) == 0)
		{
			return m;
		}
	}
	return NULL;
}


NxVec3 MxUtils::meshCenter(PxSimpleMesh& mesh)
{
	NxVec3 center(0.0f, 0.0f, 0.0f);
	for(NxU32 i = 0; i < mesh.numPoints; ++i)
	{
		center += mesh.points[i];
	}
	center /= mesh.numPoints;
	return center;
}

void MxUtils::scaleMesh(PxSimpleMesh& mesh, float scale)
{
	for(NxU32 i = 0; i < mesh.numPoints; ++i)
	{
		mesh.points[i] *= scale; 
	}
}

bool MxUtils::nodeToPxSimpleMesh(PxSimpleMesh& result, INode* node)
{
	TimeValue t = GetCOREInterface()->GetTime();

	BOOL needDel = FALSE;
	TriObject* tri = MxUtils::GetTriObjectFromNode(node, t, needDel);
	if (tri == NULL) return false;

	Mesh& mesh = tri->GetMesh();

	result.numPoints  = mesh.getNumVerts();
	result.points    =  new NxVec3[result.numPoints];
	for(NxU32 i = 0; i < result.numPoints; i++) 
	{
		((Point3*)result.points)[i] = mesh.verts[i]; 
	}
	result.numFaces   = mesh.getNumFaces();
	result.faces = new NxU32[result.numFaces * 3];
	for(NxU32 i = 0; i < result.numFaces; i++) {
		for(NxU32 j = 0; j < 3; j++) 
		{ 
			result.faces[i*3+j] = mesh.faces[i].v[j];
		}
	}
	if(needDel)
		tri->DeleteMe();
	return true;
}

#include "NXU_Hull.h"

void	MxUtils::pxCreateConvexHull(PxSimpleMesh& ret, PxSimpleMesh& src, int vertLimit, float inflation)
{
	NXU::HullLibrary hullLibrary; // = new NXU::HullLibrary();
	NXU::HullDesc hullDesc;
	hullDesc.mMaxVertices = vertLimit;
	
	hullDesc.mVcount = src.numPoints;
	hullDesc.mVertices = (float*)src.points;
	hullDesc.mVertexStride = sizeof(NxVec3);
	hullDesc.mFlags = NXU::QF_TRIANGLES;

	hullDesc.mSkinWidth = inflation;
	if (hullDesc.mSkinWidth < 0.0f)
	{
		hullDesc.mSkinWidth = 0.025f;
	}
	if (hullDesc.mSkinWidth > 0.0f)
	{
		hullDesc.mFlags |= NXU::QF_SKIN_WIDTH;
	}

	NXU::HullResult hullResult;
	NXU::HullError error = hullLibrary.CreateConvexHull(hullDesc, hullResult);

	if (error == NXU::QE_FAIL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unable to create a convex hull out of the supplied geometry\n");
		return;// ret;
	}
	
	ret.numPoints = hullResult.mNumOutputVertices;
	ret.points = new NxVec3[ret.numPoints];

	for (unsigned int i = 0; i < hullResult.mNumOutputVertices; i++) 
		ret.points[i] = NxVec3(hullResult.mOutputVertices[i*3+0], hullResult.mOutputVertices[i*3+1], hullResult.mOutputVertices[i*3+2]);
	ret.numFaces = hullResult.mNumFaces;
	NxU32 size = ret.numFaces * 3;
	ret.faces = new NxU32[size];
	for (unsigned int i = 0; i < size; i++)  
	{
		ret.faces[i] = hullResult.mIndices[i];
	}

	hullLibrary.ReleaseResult(hullResult);
	return;// ret;
}



Mesh* MxUtils::pxCreateConvexHull(Mesh* src, int vertLimit, float inflation)
{
	NXU::HullLibrary hullLibrary; // = new NXU::HullLibrary();
	NXU::HullDesc hullDesc;
	hullDesc.mMaxVertices = vertLimit;
	
	hullDesc.mVcount = src->numVerts;
	hullDesc.mVertices = (float*)src->verts;
	hullDesc.mVertexStride = sizeof(Point3);
	hullDesc.mFlags = NXU::QF_TRIANGLES;

	hullDesc.mSkinWidth = inflation;
	if (hullDesc.mSkinWidth < 0.0f)
	{
		hullDesc.mSkinWidth = 0.025f;
	}
	if (hullDesc.mSkinWidth > 0.0f)
	{
		hullDesc.mFlags |= NXU::QF_SKIN_WIDTH;
	}

	NXU::HullResult hullResult;
	NXU::HullError error = hullLibrary.CreateConvexHull(hullDesc, hullResult);

	if (error == NXU::QE_FAIL)
	{
		gCurrentstream->printf("Unable to create a convex hull out of the supplied geometry\n");
		return NULL;
	}
	
	Mesh* mesh = new Mesh();
	mesh->setNumVerts(hullResult.mNumOutputVertices); // src->numVerts);
	for (unsigned int i = 0; i < hullResult.mNumOutputVertices; i++) 
		mesh->verts[i] = Point3(hullResult.mOutputVertices[i*3+0], hullResult.mOutputVertices[i*3+1], hullResult.mOutputVertices[i*3+2]); //src->verts[i];
	mesh->setNumFaces(hullResult.mNumFaces);
	for (unsigned int i = 0; i < hullResult.mNumFaces; i++)  
	{
		mesh->faces[i].setVerts(hullResult.mIndices[i*3+0], hullResult.mIndices[i*3+1], hullResult.mIndices[i*3+2]);// tris[i][0],tris[i][1],tris[i][2]);
		mesh->faces[i].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
	}
	mesh->DeleteIsoVerts();
	mesh->InvalidateGeomCache();
	mesh->InvalidateTopologyCache();
	mesh->buildBoundingBox();
	mesh->BuildStripsAndEdges();
	mesh->BuildVisEdgeList();
	mesh->buildNormals();

	hullLibrary.ReleaseResult(hullResult);
	return mesh;
}


Mesh* MxUtils::pxSimpleMeshToNode(PxSimpleMesh& src)
{
	Mesh* mesh = new Mesh();
	mesh->setNumVerts(src.numPoints); // src->numVerts);
	for (unsigned int i = 0; i < src.numPoints; i++) 
		mesh->verts[i] = Point3(src.points[i].x, src.points[i].y, src.points[i].z); //src->verts[i];
	mesh->setNumFaces(src.numFaces);
	NxU32* p = src.faces;
	for (unsigned int i = 0; i < src.numFaces; i++)  
	{
		mesh->faces[i].setVerts(p[0], p[1], p[2]);// tris[i][0],tris[i][1],tris[i][2]);
		mesh->faces[i].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
		p += 3;
	}
	mesh->DeleteIsoVerts();
	mesh->InvalidateGeomCache();
	mesh->InvalidateTopologyCache();
	mesh->buildBoundingBox();
	mesh->BuildStripsAndEdges();
	mesh->BuildVisEdgeList();
	mesh->buildNormals();
	return mesh;
}

NxTriangleMesh* MxUtils::nodeToNxTriangleMesh(PxSimpleMesh& mesh)
{
	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	if (cookingInterface == NULL) return NULL;

	NxTriangleMeshDesc desc;

	desc.numVertices         = mesh.numPoints;
	desc.points              = mesh.points;
	desc.pointStrideBytes    = sizeof(NxVec3);
	desc.triangleStrideBytes = sizeof(NxU32)*3;
	desc.triangles           = mesh.faces;
	desc.numTriangles        = mesh.numFaces;

	MemoryWriteBuffer cookresult;
	bool status = cookingInterface->NxCookTriangleMesh(desc,cookresult);
	if(!status) 
	{
		//ccUserOutputStream::printf("ERROR: failed to cook trimesh %s\n", MaxINode->GetName());
		return NULL;
	}
	NxTriangleMesh* triangleMesh = gPluginData->getPhysicsSDK()->createTriangleMesh(MemoryReadBuffer(cookresult.data));
	if(!triangleMesh)
	{
		//ccUserOutputStream::printf("ERROR: failed to create trimesh %s\n", MaxINode->GetName());
		return NULL;
	}
	//triangleMesh->saveToDesc(desc); //read back the triangle mesh to the descriptor
	return triangleMesh;
}

NxConvexMesh* MxUtils::nodeToNxConvexMesh(PxSimpleMesh& mesh)
{
	NxCookingInterface* cookingInterface = gPluginData->getCookingInterface();
	if (cookingInterface == NULL) return NULL;

	bool freeMesh = false;
	PxSimpleMesh tmesh;
	if(mesh.numFaces > 255)
	{
		// physics does not allow a convex ploygon with more than 255 faces.
		int faceNum = 32;
		//if(ccPhysX::IsHwAvailable())
		//	faceNum = 64; //255;		
		MxUtils::pxCreateConvexHull(tmesh, mesh, faceNum, 0.0025f);
		freeMesh = true;
	}
	else
	{
		tmesh.clone(mesh);
	}

	NxConvexMeshDesc cmd;
	cmd.numVertices         = tmesh.numPoints;
	cmd.points              = tmesh.points;
	cmd.pointStrideBytes    = sizeof(NxVec3);
	cmd.flags               = NX_CF_COMPUTE_CONVEX;
	cmd.triangles           = NULL;
	cmd.triangleStrideBytes = sizeof(NxU32) * 3;
	//
	bool status = false;
	MemoryWriteBuffer cookresult;
	if (cookingInterface != NULL)
	{
		status = cookingInterface->NxCookConvexMesh(cmd,cookresult);
	}
	if(!status) 
	{
		//ccUserOutputStream::printf("ERROR: failed to cook convex mesh %s\n",MaxINode->GetName());
		return NULL;
	}
	NxConvexMesh* convexMesh = gPluginData->getPhysicsSDK()->createConvexMesh(MemoryReadBuffer(cookresult.data));
	if(! convexMesh)
	{
		//ccUserOutputStream::printf("ERROR: failed to create convex mesh %s\n",MaxINode->GetName());
		return NULL;
	}
	//
	return convexMesh;
}


#endif

MxActor* MxUtils::GetActorFromName(const char* name)
{
	if (name == NULL) return NULL;

	//There can possibly be several objects with the same name, take the first actor found
	NxArray<MxObject*> objects;
	MxPluginData::getObjectsFromName(name, objects);
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		MxActor* mxActor = objects[i]->isActor();
		if (mxActor != NULL)
		{
			return mxActor;
		}
	}
	return NULL;
}

MxJoint* MxUtils::GetJointFromNode(INode* node)
{
	if (node == NULL) return NULL;

	//There can possibly be several objects with the same name, take the first actor found
	NxArray<MxObject*> objects;
	MxPluginData::getObjectsFromNode(node, objects);
	for (NxU32 i = 0; i < objects.size(); i++)
	{
		MxJoint* mxJoint = objects[i]->isJoint();
		if (mxJoint != NULL)
		{
			return mxJoint;
		}
	}
	return NULL;
}

int MxUtils::checkNodeValidity(INode* node, const ObjectState& os)
{
	if (os.obj == NULL)
	{
		//gCurrentstream->printf("PhysX SDK Error: Object \"%s\" has no object state. Aborting the current action.\n", node->GetName());
		return -1;
	}
	int sid = os.obj->SuperClassID();
	if (sid != HELPER_CLASS_ID && sid != GEOMOBJECT_CLASS_ID)
	{
		//gCurrentstream->printf("PhysX SDK Warning: Can't submit node \"%s\" with superclass %d to PhysX, only 3D geometry and helpers are accepted.\n", node->GetName(), sid);
		return -2;
	}
	if (sid == HELPER_CLASS_ID && !node->IsGroupHead())
	{
		//gCurrentstream->printf("PhysX SDK Warning: Can't submit 'helper' nodes to PhysX (unless they are group-heads). Current object: \"%s\".\n", node->GetName());
		return -3;
	}
	return 1;
}


void MxUtils::PrintMatrix3(Matrix3& m)
{
	Point3 r0 = m.GetRow(0);
	Point3 r1 = m.GetRow(1);
	Point3 r2 = m.GetRow(2);
	Point3 r3 = m.GetRow(3);
	gCurrentstream->printf("(matrix3 [%f, %f, %f] [%f, %f, %f] [%f, %f, %f] [%f, %f, %f])", r0.x, r0.y, r0.z, r1.x, r1.y, r1.z, r2.x, r2.y, r2.z, r3.x, r3.y, r3.z);
}

// Return a pointer to a TriObject given an INode or return NULL
// if the node cannot be converted to a TriObject
TriObject* MxUtils::GetTriObjectFromNode(INode* node, TimeValue t, int& needDelete)
{
	needDelete = FALSE;
	Object *obj = node->EvalWorldState(t).obj;
	if (obj->CanConvertToType(Class_ID(TRIOBJ_CLASS_ID, 0))) { 
		TriObject *tri = (TriObject *) obj->ConvertToType(t, 
			Class_ID(TRIOBJ_CLASS_ID, 0));
		// Note that the TriObject should only be deleted
		// if the pointer to it is not equal to the object
		// pointer that called ConvertToType()
		if (obj != tri) needDelete = TRUE;
		return tri;
	}
	else {
		return NULL;
	}
}
