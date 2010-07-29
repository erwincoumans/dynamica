#ifndef MX_FLUID_H
#define MX_FLUID_H

#include "MxObjects.h"

class MxCompartment;

struct ParticleSDK
{
	NxVec3	position;
	NxVec3  velocity;
	NxReal	density;
	NxReal  lifetime;
	NxU32	id;
	NxVec3	collisionNormal;
};


class MxFluid : public MxObject {
public:
	virtual MxFluid* isFluid() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_FLUID) return this; return NULL; }

	NxFluid* getNxFluid() { return m_fluid; }

	void invalidateMesh() 
	{ 
		m_updateNr++; 
		m_numParticlesAtLastUpdate = getNumParticles();
	}

	NxU32 getNumParticles() 
	{
#if NX_SDK_VERSION_NUMBER < 272
		if (m_updateNr == 1) //workaround for the initial particles not being present in the output buffer
		{
			return m_initParticlesNum + m_numParticles;
		}
#endif
		return m_numParticles; 
	}
	void getParticles(Tab<float>& dest);
	NxU32 getUpdateNr()
	{
		if (m_numParticlesAtLastUpdate != getNumParticles())
			invalidateMesh();
		return m_updateNr;
	}

	virtual void resetObject();

	virtual Mesh* createMesh(Mesh* currentMesh);

protected:
	friend class MxPluginData;
	MxFluid(const char* name, INode* node);
	virtual ~MxFluid();

	float* getParticles(int& nrParticles);

private:

	void Free(void);

	void InitFluidDescProps(NxFluidDesc &desc);

	void CreateFluid(NxFluidDesc& desc);

private:
	NxFluid      *m_fluid;
	MxCompartment* m_compartment;
	ParticleSDK* m_fluidData;
	ParticleSDK* m_initParticles;
	NxU32 m_initParticlesNum;
	NxU32 m_numParticles;
	NxU32 m_updateNr;
	NxU32 m_numParticlesAtLastUpdate;
};

#endif //MX_FLUID_H
