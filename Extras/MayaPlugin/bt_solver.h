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
 
Written by: Nicola Candussi <nicola@fluidinteractive.com>

Modified by Roman Ponomarev <rponom@gmail.com>
01/22/2010 : Constraints reworked
01/27/2010 : Replaced COLLADA export with Bullet binary export

Modified by Francisco Gochez <fjgochez@gmail.com>
Nov 2011 - Dec 2011 : Added soft body logic, and changed solvers used to
btSoftRigidBodyWorld.
*/

//bt_solver.h

#ifndef DYN_BT_SOLVER_H
#define DYN_BT_SOLVER_H

#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletSoftBody/btSoftRigidDynamicsWorld.h"
#include "BulletCollision/Gimpact/btGImpactShape.h"
#include "BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h"
#include "LinearMath/btHashMap.h"

#include "solver_impl.h"
#include "bt_rigid_body.h"
#include "soft_body_t.h"
#include "bt_sphere_shape.h"
#include "bt_plane_shape.h"
#include "bt_box_shape.h"
#include "bt_convex_hull_shape.h"
#include "bt_mesh_shape.h"
#include "bt_hacd_shape.h"
#include "bt_composite_shape.h"
#include <vector>

#include "constraint/bt_nail_constraint.h"
#include "constraint/bt_hinge_constraint.h"
#include "constraint/bt_slider_constraint.h"
#include "constraint/bt_sixdof_constraint.h"

/** \brief	This class is an implementation of the pure virtual class solver_impl_t.
			It is based on the Bullet library solver for rigid and soft bodies.

*/

class bt_solver_t: public solver_impl_t
{
public:
    virtual rigid_body_impl_t* create_rigid_body(collision_shape_impl_t* cs) {
        return new bt_rigid_body_t(cs);
    }

	/** \brief Generate a soft body object from a given mesh.

		This function will construct a soft_body_t object from a mesh using the btSoftBodyHelpers::CreateFromTriMesh
		from Bullet.  The mesh should be passed in as a vector of (integer) indices and and a float vector of coordinates in the form
		{x1, y1, z1, x2, y2, ..., x_n, y_n, z_n}, where n is the number of vertices (so this array will have 3 * num_vertices elements)
		The indices will thus index these coordinates to form triangles, so the index vector should have length equal to the number of triangles
		in the mesh * 3.

		\param triVertexCoords constant std::vector<float> reference to the "unrolled" vertex coordinates as described above
		\param triVertexIndices constant std::vector<int> reference to the triangle vertex indices

		\return soft_body_t object with the new soft body

	*/
	virtual soft_body_t::pointer create_soft_body(const std::vector<float> &triVertexCoords, const std::vector<int> &triVertexIndices );

    virtual collision_shape_impl_t* create_sphere_shape(float radius) {
        return new bt_sphere_shape_t(radius);
    }

    virtual collision_shape_impl_t* create_plane_shape(vec3f const& normal, float d) {
        return new bt_plane_shape_t(normal, d);
    }

    virtual collision_shape_impl_t* create_box_shape(vec3f const& halfExtents) {
        return new bt_box_shape_t(halfExtents);
    }

    virtual collision_shape_impl_t* create_convex_hull_shape(vec3f const* vertices, size_t num_vertices,
                                                             vec3f const* normals,
                                                             unsigned int const *indices, size_t num_indices)
    {
        return new bt_convex_hull_shape_t(vertices, num_vertices, normals, indices, num_indices);
    }

    virtual collision_shape_impl_t* create_dynamic_mesh_shape(vec3f const* vertices, size_t num_vertices,
                                                      vec3f const* normals,
                                                      unsigned int const *indices, size_t num_indices)
    {
		bool dynamicMesh = true;
        return new bt_mesh_shape_t(vertices, num_vertices, normals, indices, num_indices, dynamicMesh);
    }

	

	virtual collision_shape_impl_t* create_hacd_shape(vec3f const* vertices, size_t num_vertices,
                                                      vec3f const* normals,
                                                      unsigned int const *indices, size_t num_indices)
    {
		bool dynamicMesh = true;
		bt_hacd_shape_t* tmpShape = new bt_hacd_shape_t(vertices, num_vertices, normals, indices, num_indices, dynamicMesh);
		btCollisionShape* shape = tmpShape->shape();
		if (shape)
		{
			return new bt_hacd_shape_t(vertices, num_vertices, normals, indices, num_indices, dynamicMesh);
		}

		return 0;
    }

	virtual collision_shape_impl_t* create_composite_shape(
				collision_shape_t::pointer* childShapes, 
				vec3f* childPositions,
				quatf* childOrientations,
				int numChildren) 
	{
		bt_composite_shape_t* tmp = new bt_composite_shape_t(childShapes, childPositions, childOrientations, numChildren);
		return tmp;
	}

	virtual collision_shape_impl_t* create_static_mesh_shape(vec3f const* vertices, size_t num_vertices,
                                                      vec3f const* normals,
                                                      unsigned int const *indices, size_t num_indices)
    {
		bool dynamicMesh = false;
        return new bt_mesh_shape_t(vertices, num_vertices, normals, indices, num_indices, dynamicMesh);
    }


    virtual nail_constraint_impl_t* create_nail_constraint(rigid_body_impl_t* rb, vec3f const& pivot)
    {
        return new bt_nail_constraint_t(rb, pivot);
    }
    virtual nail_constraint_impl_t* create_nail_constraint(rigid_body_impl_t* rbA, rigid_body_impl_t* rbB, vec3f const& pivotInA, vec3f const& pivotInB)
    {
        return new bt_nail_constraint_t(rbA, rbB, pivotInA, pivotInB);
    }
    virtual hinge_constraint_impl_t* create_hinge_constraint(rigid_body_impl_t* rb, vec3f const& pivot, quatf const& rot)
    {
        return new bt_hinge_constraint_t(rb, pivot, rot);
    }
    virtual hinge_constraint_impl_t* create_hinge_constraint(rigid_body_impl_t* rbA, vec3f const& pivotA, quatf const& rotA, rigid_body_impl_t* rbB, vec3f const& pivotB, quatf const& rotB)
    {
        return new bt_hinge_constraint_t(rbA, pivotA, rotA, rbB, pivotB, rotB);
    }
    virtual slider_constraint_impl_t* create_slider_constraint(rigid_body_impl_t* rb, vec3f const& pivot, quatf const& rot)
    {
        return new bt_slider_constraint_t(rb, pivot, rot);
    }
    virtual slider_constraint_impl_t* create_slider_constraint(rigid_body_impl_t* rbA, vec3f const& pivotA, quatf const& rotA, rigid_body_impl_t* rbB, vec3f const& pivotB, quatf const& rotB)
    {
        return new bt_slider_constraint_t(rbA, pivotA, rotA, rbB, pivotB, rotB);
    }
    virtual sixdof_constraint_impl_t* create_sixdof_constraint(rigid_body_impl_t* rb, vec3f const& pivot, quatf const& rot)
    {
        return new bt_sixdof_constraint_t(rb, pivot, rot);
    }
    virtual sixdof_constraint_impl_t* create_sixdof_constraint(rigid_body_impl_t* rbA, vec3f const& pivotA, quatf const& rotA, rigid_body_impl_t* rbB, vec3f const& pivotB, quatf const& rotB)
    {
        return new bt_sixdof_constraint_t(rbA, pivotA, rotA, rbB, pivotB, rotB);
    }

    virtual void add_rigid_body(rigid_body_impl_t* rb, const char* name)
    {
        bt_rigid_body_t* bt_body = static_cast<bt_rigid_body_t*>(rb);
		bt_body->body()->setActivationState(DISABLE_DEACTIVATION);
        m_dynamicsWorld->addRigidBody(bt_body->body());

		
    }

	/** \brief	Add a soft body to the existing "physics world" of this solver.
		
		\param sb Pointer to an object of class soft_body_impl_t, which must also be
				of class bt_soft_body_t.  If sb is 0, nothing will happen.
		\para name Character pointer.  Does nothing at the moment.
	*/

	virtual void add_soft_body(soft_body_impl_t* sb, const char* name)
	{
		if(sb == 0)
		{
			return;
		}
		bt_soft_body* bt_body = static_cast<bt_soft_body*>(sb);
		bt_body->body()->setActivationState(DISABLE_DEACTIVATION);
		m_dynamicsWorld->addSoftBody(bt_body->body());		

	}
	
	/** \brief Remove a soft body from the physics world.
		
		\param sb Pointer to a soft_body_impl_t object.  Must be of class bt_soft_body.

	*/
	virtual void remove_soft_body(soft_body_impl_t* sb)
	{
		bt_soft_body* bt_body = static_cast<bt_soft_body*>(sb);
		m_dynamicsWorld->removeSoftBody(bt_body->body());
	}

    virtual void remove_rigid_body(rigid_body_impl_t* rb)
    {
        bt_rigid_body_t* bt_body = static_cast<bt_rigid_body_t*>(rb);
		if (m_dynamicsWorld->getCollisionObjectArray().findLinearSearch(bt_body->body())!=m_dynamicsWorld->getCollisionObjectArray().size())
		{
	        m_dynamicsWorld->removeRigidBody(bt_body->body());
		}
    }

    virtual void add_constraint(constraint_impl_t* c, bool disableCollide)
    {
        bt_constraint_t* btc = dynamic_cast<bt_constraint_t*>(c);
        m_dynamicsWorld->addConstraint(btc->constraint(), disableCollide); //MB
    }

    virtual void remove_constraint(constraint_impl_t* c)
    {
        bt_constraint_t* btc = dynamic_cast<bt_constraint_t*>(c);
        m_dynamicsWorld->removeConstraint(btc->constraint());
    }
	
	/** \brief Modify the solver's global gravity level.

		\param g 3 element vector with the gravitational force in the 3 spacial
			directions.
	*/
    virtual void set_gravity(vec3f const& g)
    {
		btVector3 btG(g[0], g[1], g[2]);
		this->m_worldInfo->m_gravity = btG;
		
		// we must modify the m_dynamicsWorld descriptor for soft-bodies as well
		m_dynamicsWorld->setGravity(btG);
		
    }

    virtual void set_split_impulse(bool enabled)
    {
        m_dynamicsWorld->getSolverInfo().m_splitImpulse = enabled;
    }

    virtual void step_simulation(float dt, float fixedPhysicsFrameRate) 
    {
        m_dynamicsWorld->stepSimulation(dt, 10000, fixedPhysicsFrameRate);
    }

	virtual void debug_draw(int dbgMode);

    virtual void export_bullet_file(const char* fileName);

	virtual void register_name(const void* pointer, const char* objectName);

    virtual void import_bullet_file(const char* filename);

	virtual void export_collada_file(const char* fileName);

    virtual void import_collada_file(const char* filename);

	virtual void destroyWorld();

	virtual void createWorld();

protected:
    friend class solver_t;
    bt_solver_t();


private:
	btSoftBodyWorldInfo* m_worldInfo;
    btBroadphaseInterface*            m_broadphase;
    btConstraintSolver*               m_solver;
    btDefaultCollisionConfiguration*  m_collisionConfiguration;
    btCollisionDispatcher*            m_dispatcher;
    btSoftRigidDynamicsWorld*          m_dynamicsWorld;
public:
	btHashMap<btHashPtr,const char*>	m_nameMap;
};

#endif
