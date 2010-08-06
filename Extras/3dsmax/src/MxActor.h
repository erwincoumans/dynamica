#ifndef MX_ACTOR_H
#define MX_ACTOR_H

#include "MxObjects.h"
#include "PxPlugin.h"

class ccMaxNode;



class MxActor : public MxObject {
public:
	virtual MxActor* isActor() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_ACTOR) return this; return NULL; }

	//virtual NxActor* getNxActor();
	//virtual bool addShape(MxShape* shape); //add a shape to the actor (you can only add one shape to one actor)
	//virtual bool removeShape(MxShape* shape); //doesn't release the shape, just removes it from the actor

	void	syncGlobalPose();

	virtual NxActorDesc* getActorDesc() { return &m_desc; }
	virtual NxBodyDesc* getBodyDesc() { return &m_bodyDesc; }

	virtual void releaseAllShapes();
	virtual void releaseNxActor();

	//void setProxyNode(INode* node) { m_proxyNode = node; }
	//void setProxyNode(INode* node, NxMat34 transform)
	//{
	//	m_proxyNode = node;
	//	m_proxyMat = transform;
	//}
	INode* getProxyNode() { return m_proxyNode; }
	//NxMat34 getProxyTransform() { return m_proxyMat; }

	bool isKenematic() { return m_isKenematic; }
	void setAsKenematic(bool flag) { m_isKenematic = flag; }

	NxReal getMass() { return m_mass; }

	NxActor* createNxActor();
	PxRBInteractivity& getInteractivity() { return Interactivity; };
	void resetObject();
	void ActionBeforeSimulation();
	void ActionAfterSimulation();

	void SaveLastPose(const Matrix3& pose);
	const Matrix3& GetLastPose() { return LastPose; }

//	NxActor* getPhysXActor() { return m_actor; }

protected:
	class btCollisionShape* createShape(NxActorDesc& actorDesc, ccMaxNode* node, ccMaxNode* actorNode);

	friend class MxJoint; //hack to be able to set the refcount
	friend class MxPluginData;
	MxActor(const char* name, INode* node);
	virtual ~MxActor();

	//NxActor*           m_actor;

	class	btRigidBody* m_bulletBody; // might want to use CollisionObject* m_bulletObject if we add soft body support

	NxActorDesc        m_desc;
	NxBodyDesc         m_bodyDesc;
	//NxArray<MxShape*>  m_shapes;
	INode*             m_proxyNode;
	//NxMat34            m_proxyMat;
	bool               m_isKenematic;
	NxReal             m_mass;

	ccMaxNode*      maxNodeActor;
	ccMaxNode*      maxNodeProxy;

	Point3          ProxyDistance;             // the distance vector from proxy to actor

	Matrix3         LastPose;

	PxRBInteractivity   Interactivity;

public:
	Matrix3 m_orgPose;
	bool m_sleep;
};

#endif //MX_ACTOR_H
