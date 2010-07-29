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

	virtual NxJoint* getNxJoint();
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
	NxJoint* m_joint;
};


#endif //MX_JOINT_H