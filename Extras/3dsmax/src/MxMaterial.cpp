#include "MxUtils.h"

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>

#include "PxPlugin.h"

#include "MxPluginData.h"
#include "MxMaterial.h"

extern CharStream *gCurrentstream;
//
//NxMaterialIndex MxMaterial::getMaterialIndex()
//{
//	if (m_nxMaterial == NULL)
//	{
//		NxScene* scene = MxPluginData::getSceneStatic();
//		if (scene == NULL)
//		{
//			if (gCurrentstream) gCurrentstream->printf("Error: No scene available when trying to create a material\n");
//			return (NxMaterialIndex) 0;
//		}
//
//		m_nxMaterial = scene->createMaterial(m_desc);
//		if (m_nxMaterial == NULL)
//		{
//			if (gCurrentstream) gCurrentstream->printf("Error: unable to create a material, invalid material descriptor?\n");
//			return (NxMaterialIndex) 0;
//		}
//	}
//
//	return m_nxMaterial->getMaterialIndex();
//}
//
//MxMaterial::~MxMaterial()
//{
//	if (m_nxMaterial != NULL)
//	{
//		NxScene* scene = MxPluginData::getSceneStatic();
//		if (scene != NULL)
//		{
//			scene->releaseMaterial(*m_nxMaterial);
//		}
//		m_nxMaterial = NULL;
//	}
//}
