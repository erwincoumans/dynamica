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
