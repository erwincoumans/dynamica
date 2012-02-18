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
plugin.  It is based on rigidBodyNode.h
*/

#ifndef DYN_SOFT_BODY_NODE_H
#define DYN_SOFT_BODY_NODE_H

#include <maya/MString.h>
#include <maya/MTypeId.h>
#include <maya/MPxLocatorNode.h>
#include <maya/MStatus.h>
#include <maya/MFnMesh.h>
#include <vector>

#include "solver.h"
#include "soft_body_t.h"
#include "mathUtils.h"

/** \brief Maya node representing a soft body.

	These nodes store Dynamica soft bodies.  They are connected to the main solver via the ia_solver attribute, and are connected to an
	input mesh (the original mesh used to create the object) and an output mesh (a duplicate of the input mesh which will be manipulated
	by the soft body object inside the solver)

*/
class SoftBodyNode : public MPxLocatorNode
{
public:
	SoftBodyNode(void);
	virtual ~SoftBodyNode(void);
	
	virtual void        draw( M3dView & view, const MDagPath & path,
                            M3dView::DisplayStyle style,
                            M3dView::DisplayStatus status );
	
	virtual bool        excludeAsLocator() const { return false; }
    virtual bool        isTransparent() const { return false; }

	virtual MStatus compute(const MPlug &plug, MDataBlock& data);
	virtual void update();
	

	soft_body_t::pointer soft_body();
	
	void set_soft_body(soft_body_t::pointer);
	
	/** \brief Extracts the mesh data from a given mesh object and stores it in a "raw" format. 
			
			This function is meant to be used for creating soft body objects within a solver based on a mesh.  As such the information is
			extracted in the needed format - a vector of triangle vertex indices that reference vertex coordinate data, and the coordinate data
			itself in "unrolled" form.

		\param mayaMesh Reference to a MFnMesh object which "points" to the mesh where information will be extracted
		\param triIndices reference to a std::vector of integers in which the triangle vertex indices will be stored
		\param triVertices reference to a std::vector of floats in which triangle vertex coordinates will be stored		
	*/
	static void createHelperMesh( MFnMesh &mayaMesh, std::vector<int> &triIndices, std::vector<float> &triVertices, MSpace::Space space = MSpace::kTransform );
	static void nodeRemoved(MObject& node, void *clientData);
	virtual bool        setInternalValueInContext ( const  MPlug & plug,
                                                    const  MDataHandle & dataHandle,
                                                    MDGContext & ctx);

	static  void *      creator();
	
	static	MStatus		initialize();
	static  MTypeId     typeId;
    static  MString     typeName;

	// static public members (node attributes)
	//! connection to the Bullet solver node
	static	MObject		ia_solver;
	static  MObject     ca_solver;
	static	MObject		ca_softBody;
	//! connection to soft body input mesh
	static  MObject		inputMesh;
	//! connection to outuput mesh
	static  MObject		outputMesh;
	//! Soft body total mass
	static  MObject     ia_mass;
	//! Convenience attribute - all local soft body parameters are connected to this for easier updating
	static	 MObject	ca_softBodyParam;
	//! currently unused
	static  MObject     ia_initialPosition;
    //! currently unused
	static  MObject     ia_initialRotation;
	//! currently unused
	static  MObject     ia_initialVelocity;
    //! currently unused
	static  MObject     ia_initialSpin;
	//! Dynamic friction coefficient
	static MObject	ia_dynamicFrictionCoeff;
	//! Collision margin
	static MObject	ia_collisionMargin;
	//! number of collision clusters
	static MObject ia_numClusters;

private:
	//! Pointer to the soft body object contained in the solver
	soft_body_t::pointer m_soft_body;
	//! triangle vertex indices from the initial mesh
	std::vector<int> initVertIndices;
	//! triangle vertex coordinates (unrolled triplets) from the initial mesh
	std::vector<float> initVertCoords;
	void computeWorldMatrix(const MPlug& plug, MDataBlock& data);
	/** \brief Initializes the soft body upon creation.

		This function will extract the mesh information from the input mesh node and will create soft body within the solver
		object based on this mesh.
		\param plug MPlug reference 
		\param data MDataBlock reference
	
	*/
public:
	void destroySoftBody();

	void computeSoftBody(const MPlug& plug, MDataBlock &data);
	/** \brief This function will update the soft body when one of its attributes (such as mass or friction coefficient) is modified.

		\param plug MPlug reference 
		\param data MDataBlock reference
	*/
	void computeSoftBodyParam(const MPlug& plug, MDataBlock& data);
};

#endif
