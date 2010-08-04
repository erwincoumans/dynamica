#ifndef MX_CONTACTREPORT_H
#define MX_CONTACTREPORT_H

//#include <NxPhysics.h>
#include "MxUtils.h"
#include "LinearMath/btAlignedObjectArray.h"
#include <vector>
#include <max.h>

struct MxContact
{
	NxReal forceScale;
	NxVec3 force;
	NxVec3 point;
	NxActor* actors[2];
};

class MxContactReport : public NxUserContactReport
{
public:
	MxContactReport();

	virtual void  onContactNotify(NxContactPair& pair, NxU32 events);

	int getNumContacts();

	bool getNextContact();
	Point3 getContactPoint();
	Point3 getContactForce();
	INode* getContactNode0();
	INode* getContactNode1();

	float setFilter(float force);
	float getFilter();

	void init();

	static MxContactReport& getMyContactReport();

private:
//	std::vector<MxContact> m_contacts;
	btAlignedObjectArray<MxContact> m_contacts;
	int m_index;
	float m_filter;
};

#endif