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
plugin.  It is based on rigid_body.h
*/

#ifndef DYN_SOFT_BODY_T_H
#define DYN_SOFT_BODY_T_H

#include "shared_ptr.h"
#include "mathUtils.h"
#include "soft_body_impl_t.h"

// note : due to the need for a btSoftBodyWorldInfo object in the construction of a soft body
// from a mesh, we did not fully abstract away the dependence on Bullet objects in this particular case
// TODO: fix this
#include "BulletSoftBody/btSoftBody.h"
#include "bt_soft_body.h"
#include <vector>

class soft_body_t
{
public:
	// typedefs
	
	// shared pointer type
	typedef shared_ptr<soft_body_t> pointer;
	// TODO: fix incorrect level of abstraction here
	soft_body_t(btSoftBodyWorldInfo &worldInfo, const std::vector<float> &triVertCoords , const std::vector<int> &triVertIndices);
	virtual ~soft_body_t(void);
	void set_mass(float mass, bool fromFaces = false) ;
	float get_total_mass();
	void apply_central_force(const vec3f& force);

	void set_transform(vec3f const& position, quatf const& rotation);
    void get_transform(vec3f& position, quatf& rotation) const;
	void get_transform(mat4x4f& xform) const;
	void gl_draw(std::size_t draw_type);
	void set_dynamic_friction_coeff(float dfc = 0.2f);
	void set_gravity(const vec3f & grav);
	void get_mesh(std::vector<int> &triIndices, std::vector<vec3f> &triCoords);
	/** \brief set the soft body's collision margin
		\param margin collision margin. The function does nothing if margin < 0
	*/
	void set_collision_margin(float margin = 0.04);
	vec3f get_center(void);
	/** \brief Get the average velocity of each node (vertex)
	*/
	vec3f get_average_velocity(void);

	/** \brief Set number of collision clusters.
		
		\param num Number of clusters.  0 generates a cluster for each triangle
	*/
	void set_num_clusters(size_t num = 0); 

protected:
	friend class solver_t;
	soft_body_impl_t* impl(void)
	{
		return(m_impl.get());
	}
private:
	
	soft_body_t(void);
	shared_ptr<bt_soft_body> m_impl;	


};

#endif
