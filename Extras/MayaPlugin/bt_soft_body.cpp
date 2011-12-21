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
plugin.  It is based on bt_rigid_body.cpp
*/

#include "bt_soft_body.h"
#include "BulletSoftBody/btSoftBodyHelpers.h"
// #include "bt_mesh_shape.h"
#include "collision_shape.h"
#include <iostream>

#ifdef WIN32//for glut.h
#include <windows.h>
#endif

//think different
#if defined(__APPLE__) && !defined (VMDMESA)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#endif
// #include <GL.h>



bt_soft_body::bt_soft_body(void):
	triVertCoords(0),
	triVertIndices(0),
	numTriangles(0),
	m_body(0)
{
}

	
bt_soft_body::bt_soft_body(btSoftBodyWorldInfo &worldInfo, const std::vector<float> &triVertexCoords,
						   const std::vector<int> &triVertexIndices, float collisionMargin  )
{
	this->numTriangles = triVertexIndices.size() / 3;
	this->numVertices = triVertexCoords.size() / 3;

	// allocate ordinary arrays to hold data passed via vector objects
	// these arrays are deallocated by the destructor
	this->triVertCoords = new btScalar[triVertexCoords.size()];
	this->triVertIndices = new int[triVertexIndices.size()];
	
	// copy the data from vectors to arrays
	for(int i = 0; i < triVertexCoords.size(); i ++)
	{
		this->triVertCoords[i] = static_cast<btScalar>(triVertexCoords[i]);
	}
	
	for(int i = 0; i < triVertexIndices.size(); i ++)
	{
		this->triVertIndices[i] = triVertexIndices[i];
	}

	this->m_body.reset( btSoftBodyHelpers::CreateFromTriMesh(worldInfo, this->triVertCoords, this->triVertIndices, this->numTriangles));
		
	btSoftBody::Material*	pm=this->m_body->appendMaterial();
	pm->m_kLST				=	0.5;
	pm->m_flags				-=	btSoftBody::fMaterial::DebugDraw;
	this->m_body->generateBendingConstraints(2,pm);
	this->m_body->m_cfg.piterations	=	3;
	this->m_body->m_cfg.kDF			=	0.5;
	
	// enable cluster collisions with other soft bodies and self
	
	this->m_body->m_cfg.collisions += (btSoftBody::fCollision::CL_SS + btSoftBody::fCollision::CL_SELF);
	// this->m_body->m_cfg.collisions += btSoftBody::fCollision::VF_SS;
	
	this->m_body->generateClusters(100);
	this->m_body->scale(btVector3(1,1,1));	
	// default mass is 100
	// TODO: make adjustable via parameter
	this->m_body->setTotalMass(100, true);
	this->m_body->getCollisionShape()->setMargin(collisionMargin);
	
	this->m_body->randomizeConstraints();
	
}

bt_soft_body::~bt_soft_body(void)
{
	if(triVertCoords != 0)
	{
		delete[] triVertCoords;
	}
	if(triVertIndices != 0)
	{
		delete[] triVertIndices;
	}
}	

void bt_soft_body::get_transform(mat4x4f &xform) const
{
	float m[16];
    m_body->getWorldTransform().getOpenGLMatrix(m);
    xform = trans(cmat<float, 4, 4>(m));
}

void bt_soft_body::set_num_clusters(std::size_t num)
{
	
	this->m_body->generateClusters(num);
}

void bt_soft_body::get_transform(vec3f &position, quatf &rotation) const
    {
        const btTransform& btxform = m_body->getWorldTransform();
        btQuaternion q = btxform.getRotation();
        btVector3 p = btxform.getOrigin();
        position = vec3f(p.x(), p.y(), p.z());
        rotation = quatf(q.w(), q.x(), q.y(), q.z());
    }

void bt_soft_body::set_transform(vec3f const &position, quatf const &rotation)
    {
        vec3f tp = position;
        quatf tr = rotation;
        btTransform xform(btQuaternion(tr[1], tr[2], tr[3], tr[0]),
                          btVector3(tp[0], tp[1], tp[2])); 
        // m_body->setWorldTransform(xform);
		m_body->transform(xform);

    }

 
void bt_soft_body::set_mass(float mass, bool fromFaces)
{
	this->m_body->setTotalMass(mass, fromFaces);	
}

 void bt_soft_body::set_inertia(vec3f const& I)
{
}

 void bt_soft_body::set_restitution(float r)
{
}

 void bt_soft_body::set_dynamic_friction_coeff(float f)
{
	if(f >= 0.0f && f <= 1.0f)
	{
		this->m_body->m_cfg.kDF = f;
	}
}


 btSoftBody* bt_soft_body::body()
 {
	return this->m_body.get();
 }

void bt_soft_body::get_mesh(std::vector<int> &triIndices, std::vector<vec3f> &triCoords)
{

	triIndices.resize(this->numTriangles * 3);
	for(int i = 0; i < this->numTriangles * 3; i++)
	{
		triIndices[i] = this->triVertIndices[i];
	}
	
	btSoftBody::tNodeArray &nodes = this->m_body->m_nodes;
	triCoords.resize(nodes.size());
	for(int i = 0; i < nodes.size(); i ++)
	{
		vec3f pt(nodes[i].m_x[0], nodes[i].m_x[1], nodes[i].m_x[2] );
		triCoords[i] = pt;		
	}
}

vec3f bt_soft_body::get_center()
{
	vec3f total(0,0,0);
	btSoftBody::tNodeArray &nodes = this->m_body->m_nodes;
	
	// sum and average over current node coordinates
	for(int i = 0; i < nodes.size(); i++)
	{
		total[0] += nodes[i].m_x[0];
		total[1] += nodes[i].m_x[1];
		total[2] += nodes[i].m_x[2];
	}
	return(total / nodes.size());
}

vec3f bt_soft_body::get_average_velocity()
{
	vec3f total(0,0,0);
		btSoftBody::tNodeArray &nodes = this->m_body->m_nodes;
	
	for(int i = 0; i < nodes.size(); i++)
	{		
		total[0] += nodes[i].m_v[0];
		total[1] += nodes[i].m_v[1];
		total[2] += nodes[i].m_v[2];
	}
	return(total / nodes.size());

}

float bt_soft_body::get_total_mass(void)
{
	
	return this->m_body->getTotalMass();
}

void bt_soft_body::apply_central_force(const vec3f &force)
{
	btVector3 btForce(force[0], force[1], force[2]);
	this->m_body->addForce(btForce);
}

void bt_soft_body::set_gravity(const vec3f& grav)
{
	btVector3 btGrav(grav[0], grav[1], grav[2]);
	btSoftBodyWorldInfo* wInfo = this->m_body->getWorldInfo();
	wInfo->m_gravity = btGrav;
	
}

 void bt_soft_body::gl_draw(std::size_t draw_style) {

        glPushMatrix();             
		
		if(draw_style & collision_shape_t::kDSSolid) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
			
        glBegin(GL_TRIANGLES);
		for(int i = 0; i < this->numTriangles; i ++)
		{
			float x[3];
			float y[3];
			float z[3];
			// current index into the vertex list (i * 3 since each triangle has 3 indices)
			int j = i * 3;
			
			// retrieve coordinates from current soft body node set
			x[0] = this->m_body->m_nodes[this->triVertIndices[j]].m_x[0];
			y[0] = this->m_body->m_nodes[ this->triVertIndices[j]].m_x[1];
			z[0] = this->m_body->m_nodes[ this->triVertIndices[j]].m_x[2];

			x[1] = this->m_body->m_nodes[ this->triVertIndices[j + 1]].m_x[0];
			y[1] = this->m_body->m_nodes[ this->triVertIndices[j + 1]].m_x[1];
			z[1] = this->m_body->m_nodes[ this->triVertIndices[j + 1]].m_x[2];

			x[2] = this->m_body->m_nodes[ this->triVertIndices[j + 2]].m_x[0];
			y[2] = this->m_body->m_nodes[ this->triVertIndices[j + 2]].m_x[1];
			z[2] = this->m_body->m_nodes[ this->triVertIndices[j + 2]].m_x[2];
			
			glVertex3f(x[0], y[0], z[0]);
			glVertex3f(x[1], y[1], z[1]);
			glVertex3f(x[2], y[2], z[2]);

		}
		glEnd();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glPopMatrix();
		
    }


 void bt_soft_body::set_collision_margin(float margin)
 {
	if(margin >= 0.0f)
	{
		this->m_body->getCollisionShape()->setMargin(margin);
	}

 }
