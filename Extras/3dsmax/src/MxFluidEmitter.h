#ifndef MX_FLUIDEMITTER_H
#define MX_FLUIDEMITTER_H


#include <NxPhysics.h>
#include "MxObjects.h"
class MxFluid;

class MxFluidEmitter : public MxObject {
public:
	virtual MxFluidEmitter* isFluidEmitter() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_FLUIDEMITTER) return this; return NULL; }

	MxFluid* getFluid() { return m_fluid; }
	NxFluidEmitter* getNxEmitter() { return m_fluidEmitter; }
protected:
	friend class MxPluginData;
	MxFluidEmitter(const char* name, INode* node, MxFluid* fluid);
	virtual ~MxFluidEmitter();

private:
	void Free(void);
	void InitFluidEmitterDescProps(NxFluidEmitterDesc &desc);

	void CreateFluidEmitter(NxFluidEmitterDesc& desc);

private:
	NxFluidEmitter*	m_fluidEmitter;
	MxFluid*		m_fluid;
};

#endif //MX_FLUIDEMITTER_H
