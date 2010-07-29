#ifndef MX_OBJECTS_H
#define MX_OBJECTS_H

#include <vector>
#include <list>
#include <map>
#include <iterator>

class MxActor;
//class MxShape;
class MxJoint;
class MxMaterial;
class MxFluid;
class MxFluidEmitter;
class MxCloth;
class MxSoftBody;
class MxForceField;
class MxPluginData;
class MxMesh;
class MxCompartment;

enum MxObjectType {
	MX_OBJECTTYPE_BASEOBJECT = 0,
	MX_OBJECTTYPE_ACTOR = 1,
	MX_OBJECTTYPE_SHAPE = 2,
	MX_OBJECTTYPE_JOINT = 3,
	MX_OBJECTTYPE_MATERIAL = 4,
	MX_OBJECTTYPE_FLUID = 5,
	MX_OBJECTTYPE_FLUIDEMITTER = 6,
	MX_OBJECTTYPE_CLOTH = 7,
	MX_OBJECTTYPE_SOFTBODY = 8,
	MX_OBJECTTYPE_MESH = 9,
	MX_OBJECTTYPE_COMPARTMENT = 10,
	MX_OBJECTTYPE_FORCEFIELD = 11
};

class MxObject;
typedef std::map<INode*, MxObject*>              ccPhysXMapContainer;
typedef std::vector<MxObject*>                   ccPhysXNodeContainer;
typedef std::vector<INode*>                      ccMaxNodeContainer;

struct MxPhysXObjectRegsiter
{
	ccPhysXNodeContainer        physXNodes;              // store physics nodes
	ccMaxNodeContainer          maxNodes;                // store all max nodes
	ccPhysXMapContainer         quickTable;

	void loadMaxNodes();
	void registerObject(MxObject *);
	void unregisterObject(MxObject *);
	void arrange();
	void clear();
};


class MxObject {
public:
	MxObject(const char* name, INode* node, NxU32 id=0xFFFFFFFF) 
	{
		m_refcount = 0;
		m_name = TSTR(name);
		m_node = node;
		if (id == 0xFFFFFFFF) {
			m_currentID++;
			m_id = m_currentID;
		} else {
			m_id = id;
		}
		//
		m_ObjectType = MX_OBJECTTYPE_BASEOBJECT;
		//
		PhysXObjects.registerObject(this);
	}

	virtual ~MxObject() 
	{
		assert(m_refcount == 0);
		PhysXObjects.unregisterObject(this);
	}

	NxU32 getRefCount() 
	{
		return m_refcount;
	}

	NxU32 getID() 
	{
		return m_id;
	}

	TSTR getName() 
	{
		return m_name;
	}

	INode* getNode()
	{
		return m_node;
	}

	/**
		Resets the object as it was when it was added to the simulation
	*/
	virtual void resetObject() {}

	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_BASEOBJECT) return this; return NULL; }
	virtual MxActor* isActor() { return NULL; }
	//virtual MxShape* isShape(){ return NULL; }
	virtual MxJoint* isJoint() { return NULL; }
	virtual MxMaterial* isMaterial() { return NULL; }
	virtual MxFluid* isFluid() { return NULL; }
	virtual MxFluidEmitter* isFluidEmitter() { return NULL; }
	virtual MxCloth* isCloth() { return NULL; }
	virtual MxSoftBody* isSoftBody() { return NULL; }
	virtual MxForceField* isForceField() { return NULL; }
	virtual MxMesh* isMesh() { return NULL; }
	virtual MxCompartment* isCompartment() { return NULL; }

	MxObjectType getType()  { return m_ObjectType; }
public:
	static MxPhysXObjectRegsiter     PhysXObjects;

protected:
	friend class MxPluginData;
	void decRef() 
	{
		assert(m_refcount > 0);
		m_refcount--;
	}

	void incRef() 
	{
		m_refcount++;
	}

	TSTR m_name;
	INode* m_node;
	NxU32 m_id;
	NxU32 m_refcount;

	MxObjectType m_ObjectType;
private:
	static NxU32 m_currentID;
};

class MxCompartment : public MxObject {
public:
	virtual MxCompartment* isCompartment() { return this; }
	virtual void* isType(MxObjectType type) { if (type == MX_OBJECTTYPE_COMPARTMENT) return this; return NULL; }

	NxCompartmentType getCompartmentType() { return m_compartmentType; }
	NxU32 getCompartmentID() { return m_compartmentID; }
	NxCompartment* getNxCompartment() { return m_compartment; }
protected:
	friend class MxPluginData;
	MxCompartment(const char* name, NxCompartmentType type, NxU32 compartmentID, NxCompartment* compartment) : MxObject(name, NULL), m_compartmentType(type), m_compartmentID(compartmentID), m_compartment(compartment) 
	{
		m_ObjectType = MX_OBJECTTYPE_COMPARTMENT;
	}
	virtual ~MxCompartment() {}

	NxCompartmentType m_compartmentType;
	NxU32 m_compartmentID;
	NxCompartment* m_compartment;
};

#endif //MX_OBJECTS_H