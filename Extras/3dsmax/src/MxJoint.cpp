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
extern btDiscreteDynamicsWorld* gDynamicsWorld;

MxJoint::MxJoint(const char* name, INode* node, MxActor* actor0, MxActor* actor1) : MxObject(name, node), m_driverNode(NULL), m_actor0(actor0), m_actor1(actor1), fromstartframe(0), m_bulletConstraint(NULL) 
{
	m_ObjectType = MX_OBJECTTYPE_JOINT;
	translimitmin = NxVec3(0, 0, 0);
	translimitmax = NxVec3(0, 0, 0);
    m_desc.name = m_name.data(); //safe, since MxJoint lives longer than the NxJoint created by it
	m_desc.bulletBodies[0] = (actor0 != NULL)?actor0->getBulletRigidBody():NULL;
	m_desc.bulletBodies[1] = (actor1 != NULL)?actor1->getBulletRigidBody():NULL;
}

MxJoint::~MxJoint() 
{
	//reference counting is done by MxPluginData
	if (gDynamicsWorld && m_bulletConstraint)
	{
		gDynamicsWorld ->removeConstraint(m_bulletConstraint);
	}
}

btTypedConstraint* MxJoint::getBulletConstraint()
{
	if (m_bulletConstraint == NULL)
	{
		
		if (m_desc.isValid())
		{

			//we could add specific constraint here (btPoint2PointConstrains, btHingeConstraint, btSliderConstraint)
			//or btGeneric6DofSpringConstraint for most generic case
		
			bool disableCollision = (m_desc.jointFlags & NX_JF_COLLISION_ENABLED) == 0;

		
				
			btVector3 pivotInA(m_desc.localAnchor[0].getX(),m_desc.localAnchor[0].getY(),m_desc.localAnchor[0].getZ());
			btRigidBody* rbA = m_desc.bulletBodies[0];
			if (rbA)
			{
				MxActor* actorA = (MxActor*)rbA->getUserPointer();
				if (actorA)
				{
					btTransform localTrans;
					max2Bullet(actorA->getActorDesc()->localPose , localTrans);
					pivotInA = localTrans.inverse() * pivotInA;
				}
			}
			btVector3 pivotInB(m_desc.localAnchor[1].getX(),m_desc.localAnchor[1].getY(),m_desc.localAnchor[1].getZ());
			btRigidBody* rbB = m_desc.bulletBodies[1];
			if (rbB)
			{
				MxActor* actorB = (MxActor*)rbB->getUserPointer();
				if (actorB)
				{
					btTransform localTrans;
					max2Bullet(actorB->getActorDesc()->localPose , localTrans);
					pivotInB = localTrans.inverse() * pivotInB;
				}
			}

			if ((m_desc.xMotion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.yMotion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.zMotion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.swing1Motion == NX_D6JOINT_MOTION_FREE) &&
				(m_desc.swing2Motion == NX_D6JOINT_MOTION_FREE))
			{

				if (rbA)
				{
					if (rbB)
					{
						m_bulletConstraint = new btPoint2PointConstraint(*rbA,*rbB,pivotInA,pivotInB);
						gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
					} else
					{
						m_bulletConstraint = new btPoint2PointConstraint(*rbA,pivotInA);
						gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
					}
				}
//				MaxMsgBox(NULL, _T("btPoint2PointConstraint"), _T("Error"), MB_OK);
			} else
			{
				if ((m_desc.xMotion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.yMotion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.zMotion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.swing1Motion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.swing2Motion == NX_D6JOINT_MOTION_LOCKED) &&
				(m_desc.twistMotion == NX_D6JOINT_MOTION_LIMITED))
				{
					if (rbA)
					{
						if (rbB)
						{
							m_bulletConstraint = new btHingeConstraint(*rbA,*rbB,pivotInA,pivotInB,m_desc.localAxis[0],m_desc.localAxis[1]);
							gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
						} else
						{
							m_bulletConstraint = new btHingeConstraint(*rbA,pivotInA,m_desc.localAxis[0]);
							gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
						}
					}
//					MaxMsgBox(NULL, _T("btHingeConstraint"), _T("Error"), MB_OK);

				} else
				{

					if ((m_desc.xMotion == NX_D6JOINT_MOTION_LOCKED) &&
					(m_desc.yMotion == NX_D6JOINT_MOTION_LOCKED) &&
					(m_desc.zMotion == NX_D6JOINT_MOTION_LOCKED) &&
					(m_desc.swing1Motion == NX_D6JOINT_MOTION_LOCKED) &&
					(m_desc.swing2Motion == NX_D6JOINT_MOTION_LOCKED) &&
					(m_desc.twistMotion == NX_D6JOINT_MOTION_LOCKED))
					{
						if (rbA)
						{
							if (rbB)
							{
								btHingeConstraint* hinge = new btHingeConstraint(*rbA,*rbB,pivotInA,pivotInB,m_desc.localAxis[0],m_desc.localAxis[1]);
								m_bulletConstraint = hinge;
								hinge->setLimit(0,0);
								gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
							} else
							{
								btHingeConstraint* hinge = new btHingeConstraint(*rbA,pivotInA,m_desc.localAxis[0]);
								m_bulletConstraint = hinge;
								hinge->setLimit(0,0);
								gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
							}
						}
						MaxMsgBox(NULL, _T("Fully fixed btGeneric6DofConstraint"), _T("Error"), MB_OK);
					} else
					{
						if (rbA)
						{

							btTransform frameInA;
							frameInA.setIdentity();
							frameInA.setOrigin(pivotInA);
							btVector3 otherAxisA = m_desc.localAxis[0].cross(m_desc.localNormal[0]);
							btMatrix3x3 orn(m_desc.localAxis[0].getX(),m_desc.localAxis[0].getY(),m_desc.localAxis[0].getZ(),
								m_desc.localNormal[0].getX(),m_desc.localNormal[0].getY(),m_desc.localNormal[0].getZ(),
								otherAxisA.getX(),otherAxisA.getY(),otherAxisA.getZ());
							
							bool userLinearReferenceFrameA = true;

							if (rbB)
							{
								btTransform frameInB;
								frameInB.setIdentity();
								frameInB.setOrigin(pivotInB);
								
								btVector3 otherAxisB = m_desc.localAxis[1].cross(m_desc.localNormal[1]);


								m_bulletConstraint = new btGeneric6DofConstraint(*rbA,*rbB,frameInA,frameInB,userLinearReferenceFrameA);
								gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
							} else
							{

							
								m_bulletConstraint = new btGeneric6DofConstraint(*rbA,frameInA,userLinearReferenceFrameA);
								gDynamicsWorld->addConstraint(m_bulletConstraint,disableCollision);
							}
						}

						MaxMsgBox(NULL, _T("btGeneric6DofConstraint"), _T("Error"), MB_OK);
					}
				}
			}


			if (m_bulletConstraint == NULL)
			{
				if (gCurrentstream) gCurrentstream->printf("Unable to create the btTypedConstraint object for \"%s\".\n", getName());
				return NULL;
			}
			m_bulletConstraint->setUserConstraintPtr(this);
		}


	}
	return m_bulletConstraint;
}

bool MxJoint::setActor0(MxActor* actor)
{
	if (m_actor0 != NULL)
	{
		m_actor0->decRef();
	}
	m_actor0 = actor;
	m_desc.bulletBodies[0] = NULL;
	if (m_actor0)
	{
		m_desc.bulletBodies[0] = m_actor0->getBulletRigidBody();
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
	m_desc.bulletBodies[1] = NULL;
	if (m_actor1)
	{
		m_desc.bulletBodies[1] = m_actor1->getBulletRigidBody();
		m_actor1->incRef();
	}
	return true;
}
