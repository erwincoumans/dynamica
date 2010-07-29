#ifndef MX_FORCEFIELD_H
#define MX_FORCEFIELD_H


class MxForceField : public MxObject {
public:
	virtual MxForceField* isForceField() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_FORCEFIELD) return this; return NULL; }

	virtual NxForceField* getNxForceField() { return m_nxForceField; }

	virtual void resetObject();
	virtual void updateMeshFromSimulation(TimeValue t);

protected:

	friend class MxPluginData;
	MxForceField(const char* name, INode* node);
	virtual ~MxForceField();

	void Free(bool deletingObject);
	//void UpdateMaxMeshState(Mesh& mesh);

	float GetFloatProp(const char* name, float def);
	int GetIntProp(const char* name, int def);
	bool GetBoolProp(const char* name, bool def);

	NxMat34                    m_localPoseInv;
	bool                       m_hasFrameActor;
	NxForceField*              m_nxForceField;
	NxForceFieldShape*         m_nxFFShape;

#if	NX_SDK_VERSION_NUMBER	>= 280
	NxForceFieldLinearKernel*  m_nxKernel;
#else
	void * m_nxKernel;
#endif

	MxCompartment*             m_compartment;

	Matrix3 m_nodeTM;
	NxMat34 m_nxInvNodeTM;

};

#endif //MX_SOFTBODY_H
