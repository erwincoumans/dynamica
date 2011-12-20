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
 
This file was written by Francisco Gochez <fjgochez@gmail.com>, and was not part of the original
plugin.  It is based on bt_rigid_body.h
*/


#ifndef DYN_BT_SOFT_BODY_H
#define DYN_BT_SOFT_BODY_H

#include <iterator>
#include "soft_body_impl_t.h"
#include "shared_ptr.h"
#include "LinearMath/btAlignedObjectArray.h"
#include "BulletSoftBody/btSoftBody.h"
#include "mathUtils.h"
#include <vector>

/** \brief Implementation of the soft_body_impl_t interface based on Bullet soft bodies
*/
class bt_soft_body : public soft_body_impl_t
{
public:
	//! Smart pointer type
	typedef shared_ptr<bt_soft_body> pointer;
	
	/** \brief A constructor which builds the soft body from a mesh using btSoftBodyHelpers::CreateFromTriMesh
		\see soft_body_t::soft_body_t
		\see soft_body_t::add_soft_body

		\param worldInfo Reference to a btSoftBodyWorldInfo structure, to be used with CreateFromTriMesh
		\param triVertexCoords "Unrolled" set of triangle vertex coordinates
		\param triVertIndices Triangle integer indices into the vertex coordinates
		\param collisionMargin collision margin
	*/
	bt_soft_body(btSoftBodyWorldInfo &worldInfo,const std::vector<float> &triVertexCoords,
		const std::vector<int> &triVertexIndices,
		float collisionMargin = 0.04f);
	virtual ~bt_soft_body(void);
	btSoftBody* body();

	virtual void get_transform(mat4x4f &xForm) const;
	
	/** \brief Set total mass
		
		\param mass Float with total mass
		\param fromFaces boolean flag, passed directly to btSoftBody::setTotalMass function
	*/
	virtual void set_mass(float mass, bool fromFaces = false);
	/** \brief set the gravity level for this soft body.

		Soft bodies each seem to have their own gravity level independent of the solver's 
		global setting.

		\param grav reference to vec3f with the gravity level

	*/

	virtual void set_gravity(const vec3f & grav);
	/** \brief Set number of collision clusters
		
		\param num Number of clusters.  0 generates a cluster for each triangle
		\see btSoftBody::generateClusters
		
	*/
	virtual void set_num_clusters(size_t num = 0);

	virtual float get_total_mass(void);
    virtual void set_inertia(vec3f const& I);
    virtual void set_restitution(float r);
    virtual void set_dynamic_friction_coeff(float f);
	virtual void apply_central_force(const vec3f& force);
    virtual void set_transform(vec3f const& position, quatf const& rotation);
    virtual void get_transform(vec3f& position, quatf& rotation) const;
	virtual void gl_draw(std::size_t draw_style);
	virtual vec3f get_center();
	virtual vec3f get_average_velocity();
	virtual void set_collision_margin(float margin = 0.04);

	friend class bt_solver_t;
	/** \brief Extracts the current mesh configuration of the soft body into "generic" std::vector objects

		\param triIndices Reference to a std::vector<int> object which will be overwritten by triangle vertex indices
		\param triCoords Reference to a std::vector<vec3f> object which will be overwritten by triangle vertex coordinates

	*/
	virtual void get_mesh(std::vector<int> &triIndices, std::vector<vec3f> &triCoords);
private:
	bt_soft_body(void);
	shared_ptr<btSoftBody> m_body;

	// btSoftBodyWorldInfo m_SoftWorldInfo;
	// WARNING: naked pointers!
	btScalar *triVertCoords;
	int *triVertIndices;
	
	int numTriangles;
	int numVertices;
};

#endif
