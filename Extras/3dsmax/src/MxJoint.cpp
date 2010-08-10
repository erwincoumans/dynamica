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

#include "MxPluginData.h"
#include "MxJoint.h"
#include "MxActor.h"

extern CharStream *gCurrentstream;

MxJoint::MxJoint(const char* name, INode* node, MxActor* actor0, MxActor* actor1) : MxObject(name, node), m_driverNode(NULL), m_actor0(actor0), m_actor1(actor1), fromstartframe(0), m_joint(NULL) 
{
	m_ObjectType = MX_OBJECTTYPE_JOINT;
	translimitmin = NxVec3(0, 0, 0);
	translimitmax = NxVec3(0, 0, 0);
    m_desc.name = m_name.data(); //safe, since MxJoint lives longer than the NxJoint created by it
	//m_desc.actor[0] = (actor0 != NULL)?actor0->getNxActor():NULL;
	//m_desc.actor[1] = (actor1 != NULL)?actor1->getNxActor():NULL;
}

MxJoint::~MxJoint() 
{
	//reference counting is done by MxPluginData
}

NxJoint* MxJoint::getNxJoint()
{
	if (m_joint == NULL)
	{
		if (m_desc.isValid())
		{
			m_joint = 0;//gPluginData->getScene()->createJoint(m_desc);
			if (m_joint == NULL)
			{
				if (gCurrentstream) gCurrentstream->printf("Unable to create the NxJoint object for \"%s\".\n", getName());
				return NULL;
			}
			m_joint->userData = this;
		}
	}
	return m_joint;
}

bool MxJoint::setActor0(MxActor* actor)
{
	if (m_actor0 != NULL)
	{
		m_actor0->decRef();
	}
	m_actor0 = actor;
	m_desc.actor[0] = NULL;
	if (m_actor0)
	{
		//m_desc.actor[0] = m_actor0->getNxActor();
		m_actor0->incRef();
	}
	return true;
}

bool MxJoint::setActor1(MxActor* actor)
{
	if (m_actor1 != NULL)
	{
		m_actor1->decRef();
	}
	m_actor1 = actor;
	m_desc.actor[1] = NULL;
	if (m_actor1)
	{
		//m_desc.actor[1] = m_actor1->getNxActor();
		m_actor1->incRef();
	}
	return true;
}
