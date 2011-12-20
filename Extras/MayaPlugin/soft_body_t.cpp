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
plugin.  It is based on rigid_body.cpp
*/

#include "soft_body_t.h"
#include "BulletSoftBody/btSoftBody.h"

soft_body_t::soft_body_t(void):
	m_impl(0)
{
}

	
soft_body_t::soft_body_t(btSoftBodyWorldInfo &worldInfo, const std::vector<float> &triVertCoords, const std::vector<int> &triVertIndices)
{
	m_impl.reset(new bt_soft_body(worldInfo, triVertCoords, triVertIndices));	
}

void soft_body_t::gl_draw(std::size_t draw_type)
{
	this->m_impl->gl_draw(draw_type);
}

void soft_body_t::set_mass(float mass, bool fromFaces)
{
	if(mass >= 0)
	{
		this->m_impl->set_mass(mass, fromFaces);
	}
}

void soft_body_t::set_transform(vec3f const& position, quatf const& rotation) 
{ 
	 m_impl->set_transform(position, rotation); 
}

void soft_body_t::get_transform(vec3f& position, quatf& rotation) const
{ 
	m_impl->get_transform(position, rotation);  
}
void soft_body_t::get_transform(mat4x4f& xform) const                          
{ 
	m_impl->get_transform(xform); 
}

void soft_body_t::get_mesh(std::vector<int> &triIndices, std::vector<vec3f> &triCoords)
{
	this->m_impl->get_mesh(triIndices, triCoords);
}

void soft_body_t::set_dynamic_friction_coeff(float dfc)
{
	this->m_impl->set_dynamic_friction_coeff(dfc);
}

vec3f soft_body_t::get_center(void)
{
	return this->m_impl->get_center();
}

vec3f soft_body_t::get_average_velocity(void)
{
	return this->m_impl->get_average_velocity();
}

void soft_body_t::apply_central_force(const vec3f& force)
{
	this->m_impl->apply_central_force(force);
}


float soft_body_t::get_total_mass(void)
{
	return this->m_impl->get_total_mass();
}

void soft_body_t::set_gravity(const vec3f & grav)
{
	this->m_impl->set_gravity(grav);
}

void soft_body_t::set_num_clusters(std::size_t num)
{
	this->m_impl->set_num_clusters(num);
}

void soft_body_t::set_collision_margin(float margin)
{
	this->m_impl->set_collision_margin(margin);
}

soft_body_t::~soft_body_t(void)
{
}
