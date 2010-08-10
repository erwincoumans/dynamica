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