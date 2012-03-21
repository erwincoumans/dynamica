/*
Bullet Continuous Collision Detection and Physics Library Maya Plugin
Copyright (c) 2008 Walt Disney Studios
 
This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising
from the use of this software.
Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it freely,
subject to the following restrictions:
 
1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must
not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
 
Written by: Erwin Coumans <erwin.coumans@gmail.com>
*/
 
//bt_compound_shape.h

#ifndef DYN_BT_COMPOUND_SHAPE_H
#define DYN_BT_COMPOUND_SHAPE_H

#include "composite_shape_impl.h"
#include "drawUtils.h"
#include "BulletCollision/CollisionShapes/btCompoundShape.h"

class bt_composite_shape_t: public bt_collision_shape_t 
{
public:
    virtual void gl_draw(size_t draw_style) 
    {
			std::cout << "bt_box_shape_t::draw" << std::endl;

    }

    virtual void set_scale(vec3f const& s) {
        const btVector3& scale = shape()->getLocalScaling();
        if(scale.x() != s[0] || scale.y() != s[1] || scale.z() != s[2]) {
            shape()->setLocalScaling(btVector3(s[0], s[1], s[2]));
            update();
        }
    }

    virtual void get_scale(vec3f& s) {
        const btVector3& scale = shape()->getLocalScaling();
        s = vec3f(scale.x(), scale.y(), scale.z());
    }

    virtual float volume()                  { return m_volume;  }
    virtual vec3f const& local_inertia()    { return m_local_inertia;  }
    virtual vec3f const& center()           { return m_center; }
    virtual quatf const& rotation()         { return m_rotation;  }

	virtual void getCenterOfMassTransformShift(class btTransform& shiftTransform)
	{
		shiftTransform.setIdentity();
		shiftTransform.setOrigin(btVector3(center()[0],center()[1],center()[2]));
		shiftTransform.setRotation(btQuaternion(rotation()[1],rotation()[2],rotation()[3],rotation()[0]));
	}

protected:
    friend class bt_solver_t;

	std::vector<collision_shape_t::pointer> m_childCollisionShapes;
	std::vector< vec3f> m_childPosition;
	std::vector< quatf> m_childOrientations;

    bt_composite_shape_t(
				collision_shape_t::pointer* childShapes, 
				vec3f* childPositions,
				quatf* childOrientations,
				int numChildren): 
        bt_collision_shape_t()
    { 

		for (int i=0;i<numChildren;i++)
		{
			m_childCollisionShapes.push_back(childShapes[i]);
			m_childPosition.push_back(childPositions[i]);
			m_childOrientations.push_back(childOrientations[i]);
		}

		btCompoundShape* compound = new btCompoundShape();
		for (int i=0;i<numChildren;i++)
		{
			btTransform childTrans;
			childTrans.setIdentity();
			childTrans.setOrigin(btVector3(childPositions[i][0],childPositions[i][1],childPositions[i][2]));
			childTrans.setRotation(btQuaternion(childOrientations[i][1],childOrientations[i][2],childOrientations[i][3],childOrientations[i][0]));
			btCollisionShape* childShape = childShapes[i]->getBulletCollisionShape();
			if (childShape)
			{
				compound->addChildShape(childTrans, childShape);
			}
		}

		btScalar* masses = new btScalar[numChildren];
		for (int i=0;i<numChildren;i++)
			masses[i] = 1.f;

		btTransform principal;
		btVector3 inertia;
		compound->calculatePrincipalAxisTransform(masses, principal, inertia);
		
		delete[] masses;
		m_center = vec3f(principal.getOrigin().x(),principal.getOrigin().y(),principal.getOrigin().z());
        m_rotation = quatf(principal.getRotation().w(),principal.getRotation().x(),principal.getRotation().y(),principal.getRotation().z());
		
		for (int i=0;i<numChildren;i++)
		{
			btTransform newChildTrans = principal.inverse() * compound->getChildTransform(i);
			compound->updateChildTransform(i,newChildTrans,false);
		}
		compound->recalculateLocalAabb();
		set_shape(compound);
		
		update();
    }

	virtual ~bt_composite_shape_t()
	{
		int i;
		i=0;
	}

    void update()
    {
        btCompoundShape* compound_shape = static_cast<btCompoundShape*>(shape());
        //btVector3 e = 2 * compound_shape->getHalfExtentsWithoutMargin();
		btTransform tr;
		tr.setIdentity();
		btVector3 aabbMin,aabbMax,localInertia;
		compound_shape->getAabb(tr, aabbMin,aabbMax);
		compound_shape->calculateLocalInertia(1.f,localInertia);
		
		btVector3 e = aabbMax-aabbMin;
		m_volume = e.x() * e.y() * e.z();
        //m_center = vec3f(0,0,0);
        //m_rotation = qidentity<float>();
		m_local_inertia = vec3f(localInertia.x(),localInertia.y(),localInertia.z());
           
    }

private:
    float m_volume;
    vec3f m_center;
    quatf m_rotation;
    vec3f m_local_inertia;
};

#endif //DYN_BT_COMPOUND_SHAPE_H

