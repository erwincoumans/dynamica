#include "MxContactReport.h"
#include "MxActor.h"
//#include <iterator>

// 
// to make contact report active, please call 
//     m_scene->setActorGroupPairFlags(0,0,NX_NOTIFY_ON_START_TOUCH|NX_NOTIFY_ON_TOUCH|NX_NOTIFY_ON_END_TOUCH);
// to make the contact force enabled, please call for each shape
//     nxShape->setFlag(NX_SF_POINT_CONTACT_FORCE, true);
//
void MxContactReport::init()
{
	m_contacts.clear();
	m_index = -1;
}

int MxContactReport::getNumContacts()
{
	return m_contacts.size();
}

bool MxContactReport::getNextContact()
{
	++m_index;
	while(m_index < m_contacts.size())
	{
		MxContact& c = m_contacts[m_index];
		if(c.forceScale > m_filter)
		{
			return true;
		}
		++m_index;
	}
	return false;
}

Point3 MxContactReport::getContactForce()
{
	MxContact& c = m_contacts[m_index];
	return Point3(c.force[0], c.force[1], c.force[2]);
}

Point3 MxContactReport::getContactPoint()
{
	MxContact& c = m_contacts[m_index];
	return Point3(c.point[0], c.point[1], c.point[2]);
}

INode* MxContactReport::getContactNode0()
{
	MxContact& c = m_contacts[m_index];
	MxActor* a = (MxActor*) c.actors[0]->userData;
	return a->getNode();
}

INode* MxContactReport::getContactNode1()
{
	MxContact& c = m_contacts[m_index];
	MxActor* a = (MxActor*) c.actors[1]->userData;
	return a->getNode();
}

void  MxContactReport::onContactNotify(NxContactPair& pair, NxU32 events)
{
#if 0
	// most collision events are on touch events
	if( ! (events & NX_NOTIFY_ON_START_TOUCH))
	{
		return;
	}

#if NX_SDK_VERSION_NUMBER >= 280
	if(pair.isDeletedActor[0] || pair.isDeletedActor[1])
	{
		return;
	}
#endif

	if((pair.actors[0]->userData == NULL) || (pair.actors[1] == NULL))
	{
		return;
	}

	NxReal maxForceValue = 0;
	MxContact c;
	c.actors[0] = pair.actors[0];
	c.actors[1] = pair.actors[1];
	// Iterate through contact points
	NxContactStreamIterator i(pair.stream);
	//user can call getNumPairs() here
	while(i.goNextPair())
	{
		//user can also call getShape() and getNumPatches() here
		while(i.goNextPatch())
		{
			const NxVec3& contactNormal = i.getPatchNormal();
			//user can also call getPatchNormal() and getNumPoints() here
			while(i.goNextPoint())
			{
				const NxVec3& contactPoint = i.getPoint();
				NxReal fv = i.getPointNormalForce();
				if(maxForceValue < fv)
				{
					maxForceValue = fv;
					c.forceScale = fv;
					c.force = contactNormal * fv;
					c.point = contactPoint;
				}
			}
		}
	}
	//
	if(maxForceValue > 0.0001f)
	{
		m_contacts.push_back(c);
	}
#endif

}

MxContactReport& MxContactReport::getMyContactReport()
{
	static MxContactReport report;
	return report;
}

float MxContactReport::setFilter(float force)
{
	m_filter = force;
	return m_filter;
}

float MxContactReport::getFilter()
{
	return m_filter;
}

MxContactReport::MxContactReport()   : m_filter(0.0f)
{
	init();
}