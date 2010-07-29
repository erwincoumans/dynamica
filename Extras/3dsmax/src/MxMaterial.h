#ifndef MX_MATERIAL_H
#define MX_MATERIAL_H

#include "MxObjects.h"
//
//enum MxMaterialType
//{
//	MX_MATERIALTYPE_INVALID = 0,
//	MX_MATERIALTYPE_3DS = 1,
//	MX_MATERIALTYPE_USER = 2,
//	MX_MATERIALTYPE_DEFAULT = 3,
//	MX_MATERIALTYPE_PRESET = 4
//};
//
//class MxMaterial : public MxObject {
//public:
//	virtual MxMaterial* isMaterial() { return this; }
//	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_MATERIAL) return this; return NULL; }
//
//	NxMaterialIndex getMaterialIndex();
//
//	NxReal getRestitution() { return m_desc.restitution; }
//	NxReal getStaticFriction() { return m_desc.staticFriction; }
//	NxReal getDynamicFriction() { return m_desc.dynamicFriction; }
//
//	void setRestitution(NxReal restitution) { m_desc.restitution = restitution; }
//	void setStaticFriction(NxReal friction) { m_desc.staticFriction = friction; }
//	void setDynamicFriction(NxReal friction) { m_desc.dynamicFriction = friction; }
//
//	MxMaterialType getMaterialType() { return m_type; }
//
//protected:
//	friend class MxPluginData;
//	MxMaterial(const char* name, INode* node, NxMaterialDesc& materialDesc, MxMaterialType type) : MxObject(name, node) 
//	{
//		m_desc = materialDesc;
//		m_type = type;
//		m_nxMaterial = NULL;
//	}
//
//	virtual ~MxMaterial();
//
//	NxMaterialDesc m_desc;
//	MxMaterialType m_type;
//	NxMaterial* m_nxMaterial;
//};

#endif //MX_MATERIAL_H
