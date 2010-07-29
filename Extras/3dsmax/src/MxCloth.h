#ifndef MX_CLOTH_H
#define MX_CLOTH_H

#include "MxUtils.h"

class ccMaxNode;

class MxClothVert
{
public:
	NxVec3  pos;
	NxVec3  normal;
};

/**
	The cloth class can only update the cloth mesh from the simulation back into 3ds max
	if the node object is a general mesh (i.e. does not have to be converted into a mesh)
*/
class MxCloth : public MxObject {
public:
	virtual MxCloth* isCloth() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_CLOTH) return this; return NULL; }

	virtual NxCloth* getNxCloth() { return m_nxCloth; }
	NxCloth* create();

	virtual void resetObject() { Reset(); }
	virtual void updateMeshFromSimulation(TimeValue t) { UpdateMaxMesh(t); }

	void releasePhysicsObject();

protected:
	friend class MxPluginData;

	MxCloth(const char* name, INode* node, bool isMetalCloth = false);
	virtual ~MxCloth();

	//Can only update the 3ds max object if it is of mesh type (i.e. doesn't need to be converted into a mesh)
	void UpdateMaxMesh(TimeValue t);
	void Reset(void);
	void UpdateMaxMeshState(Mesh& mesh);

	void InitClothDescProps(NxClothDesc& desc);
	void InitFloatProp(NxF32& f, const char* name);
	void InitIntProp(NxU32& i, const char* name);

	bool m_isMetalCloth;
	NxActor* m_coreActor;

	Mesh* m_backupMesh;
	//Matrix3 m_originalPose;
	bool m_updateTriMesh;
	//NxMat34 m_nxInvNodeTM;

	NxU32 m_numVerts;
	NxU32 m_numIndices;
	NxU32 m_numTriangles;
	//PxSimpleMesh m_meshCreation;
	PxSimpleMesh m_meshSimulate;

	NxCloth* m_nxCloth;
	NxClothMesh* m_nxMesh;
	NxMat34 ClothPose;

private:
	ccMaxNode*      maxNodeActor;	
};

#endif //MX_CLOTH_H
