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
