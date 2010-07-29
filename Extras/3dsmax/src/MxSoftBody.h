#ifndef MX_SOFTBODY_H
#define MX_SOFTBODY_H

#include "TetraMeshHelper.h"

#ifdef PXPLUGIN_SOFTBODY

class SoftBodyBuffers;
class SoftBodyMeshData;
class NxSoftBody;
class ccMaxNode;


class MxSoftBody : public MxObject {
public:
	virtual MxSoftBody* isSoftBody() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_SOFTBODY) return this; return NULL; }

	virtual NxSoftBody* getNxSoftBody() { return m_nxSoftBody; }

	virtual void resetObject();
	virtual void updateMeshFromSimulation(TimeValue t);

	static Mesh* createSoftBodyMesh(INode* node, bool createEditableMesh, int subDivision, float simplificationFactor, bool createIsoSurface, bool singleIsoSurface);

protected:

	friend class MxPluginData;
	MxSoftBody(const char* name, INode* node);
	virtual ~MxSoftBody();

	void Free(bool deletingObject);
	void UpdateMaxMeshState(Mesh& mesh);
	bool BuildSoftBodyFromMesh(Mesh& inputMesh);

	float GetFloatProp(const char* name, float def);
	int GetIntProp(const char* name, int def);
	bool GetBoolProp(const char* name, bool def);

	SoftBodyBuffers* m_receiveBuffers;
	SoftBodyMeshData* m_softBodyData;
	NxSoftBody* m_nxSoftBody;
	NxSoftBodyMesh* m_nxSoftBodyMesh;
	MxCompartment* m_compartment;

	Matrix3 m_nodeTM;
	NxMat34 m_nxInvNodeTM;

	TetraMeshHelper m_tetraHelper;
	Mesh m_backupMesh;

private:
	ccMaxNode*      maxNodeActor;	
};

#else //PXPLUGIN_SOFTBODY

class MxSoftBody : public MxObject {
public:
	virtual MxSoftBody* isSoftBody() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_SOFTBODY) return this; return NULL; }
	static Mesh* createSoftBodyMesh(INode* node, bool createEditableMesh, int subDivision, float simplificationFactor, bool createIsoSurface, bool singleIsoSurface) { return NULL; }
protected:
	friend class MxPluginData;
	MxSoftBody(const char* name, INode* node) : MxObject(name, node) {}
	virtual ~MxSoftBody() {}
};

#endif //PXPLUGIN_SOFTBODY
#endif //MX_SOFTBODY_H
