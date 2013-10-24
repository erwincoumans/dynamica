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
02/18/2010 : Re-enabled COLLADA export as option (next to Bullet binary export)

Modified by Francisco Gochez <fjgochez@gmail.com>
Nov 2011 - Dec 2011 : Added soft body logic
*/

//bt_solver.cpp
#include "rigidBodyNode.h"
#include "constraint/nailConstraintNode.h"
#include "constraint/hingeConstraintNode.h"
#include "constraint/sliderConstraintNode.h"
#include "constraint/sixdofConstraintNode.h"

#include "LinearMath/btSerializer.h"
#include "bt_solver.h"
#include "soft_body_t.h"
#include "BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.h"
#include "../BulletColladaConverter/ColladaConverter.h"
#include <string.h>
#include <stdio.h>



#include <string.h> 
#include <sys/types.h>
#include <maya/MStatus.h>
#include <maya/MPxCommand.h>
#include <maya/MString.h>
#include <maya/MStringArray.h>
#include <maya/MArgList.h>
#include <maya/MGlobal.h>
#include <maya/MSelectionList.h>
#include <maya/MItSelectionList.h>
#include <maya/MPoint.h>
#include <maya/MPointArray.h>
#include <maya/MDagPath.h>
#include <maya/MDagPathArray.h>
//#include <maya/MFnPlugin.h>
#include <maya/MFnMesh.h>
#include <maya/MFnSet.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshEdge.h>
#include <maya/MFloatVector.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MFloatArray.h>
#include <maya/MObjectArray.h>
#include <maya/MObject.h>
//#include <maya/MPlug.h>
#include <maya/MPxFileTranslator.h>
#include <maya/MFnDagNode.h>
#include <maya/MItDag.h>
#include <maya/MDistance.h>
#include <maya/MIntArray.h>
#include <maya/MIOStream.h>


void bt_solver_t::export_collada_file(const char* fileName)
{
#ifdef BT_USE_COLLADA
	ColladaConverter tmpConverter(m_dynamicsWorld);


	  MStatus status;

	

	MStatus stat;
//	MItDag dagIterator( MItDag::kBreadthFirst, MFn::kInvalid, &stat);
	MItDag dagIterator( MItDag::kDepthFirst, MFn::kInvalid, &stat);
   	if (stat != MS::kSuccess)
	{
        std::cout << "Failure in DAG iterator setup" << std::endl;
   	}
	for ( ;!dagIterator.isDone(); dagIterator.next()) 
	{
   	    MDagPath dagPath;
   	    stat = dagIterator.getPath( dagPath );
   		if(stat != MS::kSuccess)
		{
			std::cout << "Failure in getting DAG path" << std::endl;
		}
		// skip over intermediate objects
		MFnDagNode dagNode( dagPath, &stat );
		if (dagNode.isIntermediateObject()) 
		{
			continue;
		}
		if(!dagPath.hasFn(MFn::kDependencyNode))
		{
			continue;
		}
		MObject mObj = dagNode.object(&stat);
		if(stat != MS::kSuccess)
		{
			std::cout << "Failure in getting MObject" << std::endl;
		}
        MFnDependencyNode fnNode(mObj, &stat);
		if(stat != MS::kSuccess)
		{
			std::cout << "Failure in getting dependency node" << std::endl;
		}

        if(fnNode.typeId() == rigidBodyNode::typeId) 
		{
			rigidBodyNode *rbNode = static_cast<rigidBodyNode*>(dagNode.userNode()); 
			tmpConverter.registerNameForPointer(rbNode->rigid_body()->impl()->getBulletRigidBodyPointer(),rbNode->name().asChar());

		}
        if(fnNode.typeId() == nailConstraintNode::typeId) 
		{
			nailConstraintNode *ncNode = static_cast<nailConstraintNode*>(dagNode.userNode()); 
		//	ncNode->register_name(solv.get(),ncNode->name().asChar());
		}
        if(fnNode.typeId() == hingeConstraintNode::typeId) 
		{
			hingeConstraintNode *hcNode = static_cast<hingeConstraintNode*>(dagNode.userNode()); 
			//hcNode->register_name(solv.get(),hcNode->name().asChar());
			
		}
        if(fnNode.typeId() == sliderConstraintNode::typeId) 
		{
			sliderConstraintNode *scNode = static_cast<sliderConstraintNode*>(dagNode.userNode()); 
			//scNode->register_name(solv.get(),scNode->name().asChar());

		}
        if(fnNode.typeId() == sixdofConstraintNode::typeId) 
		{
			sixdofConstraintNode *sdNode = static_cast<sixdofConstraintNode*>(dagNode.userNode()); 
			//sdNode->register_name(solv.get(),sdNode->name().asChar());
		}
	}

	tmpConverter.save(fileName);
#endif
}

void bt_solver_t::import_collada_file(const char* filename)
{
	//	ColladaConverter tmpConverter(m_dynamicsWorld.get());
	//	tmpConverter.save(fileName);
}

class MySerializer : public btDefaultSerializer
{
	bt_solver_t* m_solver;

public:
	MySerializer(bt_solver_t* solver)
		:btDefaultSerializer(),
		m_solver(solver)
	{
	}

	virtual	const char*	findNameForPointer(const void* ptr) const
	{
		const char*const * namePtr = m_solver->m_nameMap.find(ptr);
		if (namePtr && *namePtr)
			return *namePtr;
		return 0;

	}

	
};

void bt_solver_t::export_bullet_file(const char* fileName)
{
	FILE* f2 = fopen(fileName,"wb");
	if(f2 == NULL)
	{
	    fprintf(stderr,"Error: Can't open file %s for writing\n", fileName);
		return;
	}

	btDefaultSerializer* serializer = new MySerializer(this);

	m_dynamicsWorld->serialize(serializer);
	fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1,f2);
	fclose(f2);
	delete serializer;
}



void bt_solver_t::register_name(const void* pointer,const char* objectName)
{
	if (objectName)
	{
		int nameLen = strlen(objectName);
		if (nameLen>0)
		{
			char* newName = (char*)malloc(nameLen+1);
			memcpy(newName ,objectName,nameLen);
			newName[nameLen]=0;
			m_nameMap.insert(pointer,newName);
		}
	}
}

void bt_solver_t::import_bullet_file(const char* filename)
{
//todo: need to create actual bodies etc
//	ColladaConverter tmpConverter(m_dynamicsWorld.get());
//	tmpConverter.load(filename);
}


class bt_debug_draw : public btIDebugDraw
{
	int m_debugMode;
public:
	virtual void	drawLine(const btVector3& from,const btVector3& to,const btVector3& color)
	{
		glBegin(GL_LINES);
			glColor3f(color.getX(), color.getY(), color.getZ());
			glVertex3d(from.getX(), from.getY(), from.getZ());
			glVertex3d(to.getX(), to.getY(), to.getZ());
		glEnd();
	}
	virtual void	drawContactPoint(const btVector3& PointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color)
	{
		return;
	}
	virtual void	reportErrorWarning(const char* warningString)
	{
		return;
	}
	virtual void	draw3dText(const btVector3& location,const char* textString) 
	{
		return;
	}
	virtual void	setDebugMode(int debugMode) { m_debugMode = debugMode; }
	virtual int		getDebugMode() const  { return m_debugMode; }
};



bt_solver_t::bt_solver_t():	
	m_broadphase(0),
          m_solver(0),
            m_collisionConfiguration(0),
            m_dispatcher(0),
			m_dynamicsWorld(0),
			m_worldInfo(0)
{

	createWorld();
	
}

void bt_solver_t::destroyWorld()
{
	if (!m_dynamicsWorld)
		return;

	while (m_dynamicsWorld->getNumConstraints())
	{
		m_dynamicsWorld->removeConstraint(m_dynamicsWorld->getConstraint(0));
	}
//fixme: check memory deallocation
	m_dispatcher = 0;
	m_broadphase = 0;
	m_solver = 0;
	m_collisionConfiguration = 0;
	m_dynamicsWorld=0;

}

void bt_solver_t::createWorld()
{
	// use soft-body and rigid body collisions
	m_collisionConfiguration = new btSoftBodyRigidBodyCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	
	
	m_broadphase = new btDbvtBroadphase();
	m_solver =new btSequentialImpulseConstraintSolver();

	m_dynamicsWorld = new btSoftRigidDynamicsWorld(m_dispatcher,
                                                        m_broadphase,
                                                        m_solver,
                                                        m_collisionConfiguration);
	//register algorithm for concave meshes
    btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher);

	// set default gravity
    m_dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	
	m_dynamicsWorld->getDispatchInfo().m_enableSPU = true;
	
	m_worldInfo = new btSoftBodyWorldInfo;
	m_worldInfo->m_broadphase = m_broadphase;
	m_worldInfo->m_dispatcher = m_dispatcher;
	m_worldInfo->m_gravity = m_dynamicsWorld->getGravity();
	m_worldInfo->m_sparsesdf.Initialize();

	bt_debug_draw* dbgDraw = new bt_debug_draw();
	m_dynamicsWorld->setDebugDrawer(dbgDraw);
}

void bt_solver_t::debug_draw(int dbgMode)
{
	m_dynamicsWorld->getDebugDrawer()->setDebugMode(dbgMode);	
	m_dynamicsWorld->debugDrawWorld();
}

soft_body_t::pointer bt_solver_t::create_soft_body(const std::vector<float> &triVertexCoords, const std::vector<int> &triVertexIndices )
{
	// we simply invoke the soft body constructor together with this solver's world info
	soft_body_t::pointer sptr(new soft_body_t( *m_worldInfo, triVertexCoords, triVertexIndices ));
	return sptr;
}
