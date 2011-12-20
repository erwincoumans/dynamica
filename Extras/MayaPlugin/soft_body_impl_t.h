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
plugin.  It is based on rigid_body_impl.h
*/

#ifndef DYN_SOFT_BODY_IMPL_T_H
#define DYN_SOFT_BODY_IMPL_T_H

#include <vector>
#include "mathUtils.h"

class soft_body_impl_t
{
public:	
	virtual ~soft_body_impl_t(void) 
	{
	}
	
    //
    virtual void set_mass(float mass, bool fromFaces = false) = 0;  
	virtual float get_total_mass(void) = 0;
	virtual void set_gravity(const vec3f & grav) = 0;
    virtual void set_inertia(vec3f const& I) = 0;      
    virtual void set_restitution(float r) = 0;          
    virtual void set_dynamic_friction_coeff(float f) = 0;
	virtual void apply_central_force(const vec3f& force) = 0;  
    virtual void set_transform(vec3f const& position, quatf const& rotation) = 0;
    virtual void get_transform(vec3f& position, quatf& rotation) const = 0;
    virtual void get_transform(mat4x4f& xform) const = 0;
	virtual void gl_draw(size_t draw_style) = 0;
	virtual void get_mesh(std::vector<int> &triIndices, std::vector<vec3f> &triCoords) = 0;
	virtual vec3f get_center(void) = 0;
	virtual vec3f get_average_velocity(void) = 0;
	/** \brief set the soft body's collision margin
		\param margin collision margin. The function does nothing if margin < 0
		\see soft_body_t::set_collision_margin
	*/
	virtual void set_collision_margin(float margin = 0.04) = 0;
	
	/** \brief Set number of collision clusters
		
		\param num Number of clusters.  0 generates a cluster for each triangle
	*/
	virtual void set_num_clusters(size_t num = 0) = 0;
};

#endif
