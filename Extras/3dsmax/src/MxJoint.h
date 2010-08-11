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

#ifndef MX_JOINT_H
#define MX_JOINT_H

#include "MxObjects.h"

class MxJoint : public MxObject {
public:
	virtual MxJoint* isJoint() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_JOINT) return this; return NULL; }

	virtual INode* getDriverNode() { return m_driverNode; }
	virtual void setDriverNode(INode* node) { m_driverNode = node; }

	virtual MxActor* getActor0() { return m_actor0; }
	virtual MxActor* getActor1() { return m_actor1; }
	virtual bool setActor0(MxActor* actor);
	virtual bool setActor1(MxActor* actor);

	virtual btTypedConstraint* getBulletConstraint();
	virtual NxD6JointDesc* getD6JointDesc() { return &m_desc; }

	//TODO: Are these needed anymore?
	int fromstartframe;
	void setFromStartFrame(int frame) { fromstartframe = frame; }
	NxVec3 translimitmin;
	NxVec3 translimitmax;

protected:
	friend class MxPluginData;
	MxJoint(const char* name, INode* node, MxActor* actor0, MxActor* actor1);
	virtual ~MxJoint();

	INode* m_driverNode;
	MxActor* m_actor0;
	MxActor* m_actor1;

	NxD6JointDesc m_desc;
	
	class btTypedConstraint* m_bulletConstraint;
};


#endif //MX_JOINT_H