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
 
This file was added to the original plugin by Francisco Gochez <fjgochez@gmail.com>
It is based in large part on rigidBodyNode.cpp

*/

#include "BulletSoftBody/btSoftBody.h"

#include "softBodyNode.h"
#include <iostream>
#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDependencyNode.h>
#include "mayaUtils.h"
#include <maya/MStatus.h>
#include <maya/MFnMesh.h>
#include <maya/MFloatPointArray.h>
#include <maya/MPlugArray.h>
#include <maya/MQuaternion.h>
#include <vector>

MTypeId     SoftBodyNode::typeId(0x192837) ;
MString     SoftBodyNode::typeName("dSoftBody");

// declare object attributes

MObject     SoftBodyNode::ia_solver;
MObject		SoftBodyNode::ca_solver;
MObject		SoftBodyNode::ca_softBody;
MObject		SoftBodyNode::inputMesh;
MObject		SoftBodyNode::outputMesh;
MObject     SoftBodyNode::ia_mass;

MObject     SoftBodyNode::ia_initialPosition;
MObject     SoftBodyNode::ia_initialRotation;
MObject     SoftBodyNode::ia_initialVelocity;
MObject     SoftBodyNode::ia_initialSpin;
MObject		SoftBodyNode::ca_softBodyParam;
MObject		SoftBodyNode::ia_dynamicFrictionCoeff;
MObject		SoftBodyNode::ia_collisionMargin;
MObject		SoftBodyNode::ia_numClusters;

// default mass
const double DEFAULT_MASS = 100.0f;
// default dynamic friction coefficient
const double DEFAULT_DFC = 0.2f;
// default collision margin.  1.75 is an adjustment for Maya-Bullet discrepancy
const float DEFAULT_CMARGIN = 1.75 * 0.04;
// default number of collision clusters
const int DEFAULT_CLUSTERS = 100;

SoftBodyNode::SoftBodyNode(void):
	m_soft_body(0)
{
}

SoftBodyNode::~SoftBodyNode(void)
{
}

void* SoftBodyNode::creator()
{
	return new SoftBodyNode();
}


void SoftBodyNode::draw( M3dView & view, const MDagPath &path,
                             M3dView::DisplayStyle style,
                             M3dView::DisplayStatus status )
{
	
	MObject thisObject(thisMObject());
    update();

	MFnDagNode fnDagNode(thisObject);    
	MFnTransform fnTransform(fnDagNode.parent(0));
	MVector mtranslation = fnTransform.getTranslation(MSpace::kTransform);

    view.beginGL();
    glPushAttrib( GL_ALL_ATTRIB_BITS );
	// glTranslatef(-mtranslation.x, -mtranslation.y, -mtranslation.z);
    // size of the soft-body "icon"
	const float LINE_OFFSET = 0.25f;
	if(m_soft_body) {
	   // draw a simple cross
		glBegin(GL_LINES);
		glVertex3f( - LINE_OFFSET, 0.0, 0.0);
		glVertex3f( LINE_OFFSET, 0.0,0.0);
		
		glVertex3f(0.0, - LINE_OFFSET, 0.0);
		glVertex3f(0.0, LINE_OFFSET, 0.0);

		glVertex3f(0.0, 0.0, - LINE_OFFSET);
		glVertex3f(0.0, 0.0, LINE_OFFSET);
		
		// glTranslatef(-mtranslation.x, -mtranslation.y, -mtranslation.z);
		// this->m_soft_body->gl_draw(collision_shape_t::kDSWireframe);
		glEnd();
		glPopMatrix();
    } // end if m_soft_body
    glPopAttrib();
    view.endGL();
}



MStatus SoftBodyNode::initialize()
{
	MFnMessageAttribute fnMsgAttr;
	MStatus status = MStatus::kSuccess;

	ia_solver = fnMsgAttr.create("solver", "solv", &status);
    MCHECKSTATUS(status, "creating solver attribute")
    status = addAttribute(ia_solver);
    MCHECKSTATUS(status, "adding solver attribute")
	
	MFnNumericAttribute fnNumericAttr;
	ca_solver = fnNumericAttr.create("ca_solver", "caso", MFnNumericData::kBoolean, 0, &status);
    MCHECKSTATUS(status, "creating ca_solver attribute")
    fnNumericAttr.setConnectable(false);
    fnNumericAttr.setHidden(true);
    fnNumericAttr.setStorable(false);
    fnNumericAttr.setKeyable(false);
    status = addAttribute(ca_solver);
    MCHECKSTATUS(status, "adding ca_solver attribute")

    ca_softBody = fnNumericAttr.create("ca_softBody", "casb", MFnNumericData::kBoolean, 0, &status);   
	MCHECKSTATUS(status, "creating ca_softBody attribute")
	fnNumericAttr.setConnectable(false);
    fnNumericAttr.setHidden(true);
    fnNumericAttr.setStorable(false);
    fnNumericAttr.setKeyable(false);
    status = addAttribute(ca_softBody);
    MCHECKSTATUS(status, "adding ca_softBody attribute")

	inputMesh = fnMsgAttr.create("inMesh", "inmsh", &status);
	MCHECKSTATUS(status, "creating inMesh attribute")
	status = addAttribute(inputMesh);
	MCHECKSTATUS(status, "adding inMesh attribute")

	outputMesh = fnMsgAttr.create("outMesh", "outmsh", &status);
	MCHECKSTATUS(status, "creating outMesh attribute")
	status = addAttribute(outputMesh);
	MCHECKSTATUS(status, "adding outMesh attribute")
	/*
		Note that these initial* attributes are not in use for soft bodies at the moment
	*/
	ia_initialPosition = fnNumericAttr.createPoint("initialPosition", "inpo", &status);
    MCHECKSTATUS(status, "creating initialPosition attribute")
    status = addAttribute(ia_initialPosition);
    MCHECKSTATUS(status, "adding initialPosition attribute")

    ia_initialRotation = fnNumericAttr.createPoint("initialRotation", "inro", &status);
    MCHECKSTATUS(status, "creating initialRotation attribute")
    status = addAttribute(ia_initialRotation);
    MCHECKSTATUS(status, "adding initialRotation attribute")

    ia_initialVelocity = fnNumericAttr.createPoint("initialVelocity", "inve", &status);
    MCHECKSTATUS(status, "creating initialVelocity attribute")
    status = addAttribute(ia_initialVelocity);
    MCHECKSTATUS(status, "adding initialVelocity attribute")

    ia_initialSpin = fnNumericAttr.createPoint("initialSpin", "insp", &status);
    MCHECKSTATUS(status, "creating initialSpin attribute")
    status = addAttribute(ia_initialSpin);
    MCHECKSTATUS(status, "adding initialSpin attribute")

	// total soft body mass
	ia_mass = fnNumericAttr.create("mass", "ma", MFnNumericData::kDouble, DEFAULT_MASS, &status);
    MCHECKSTATUS(status, "creating mass attribute")
    fnNumericAttr.setKeyable(true);
    status = addAttribute(ia_mass);
    MCHECKSTATUS(status, "adding mass attribute")
	
	// create dynamic friction coefficient attribute
	ia_dynamicFrictionCoeff = fnNumericAttr.create("dynamicfrictioncoeff", "dfc", MFnNumericData::kDouble, DEFAULT_DFC, &status);
    MCHECKSTATUS(status, "creating dynamicfrictioncoeff attribute")
    fnNumericAttr.setKeyable(true);
    status = addAttribute(ia_dynamicFrictionCoeff);
    MCHECKSTATUS(status, "adding dynamicfrictioncoeff attribute")

	// create collision margin attribute
	ia_collisionMargin = fnNumericAttr.create("collisionmargin", "cmargin", MFnNumericData::kFloat, DEFAULT_CMARGIN, &status);
    MCHECKSTATUS(status, "creating collisionmargin attribute")
    fnNumericAttr.setKeyable(false);
    status = addAttribute(ia_collisionMargin);
    MCHECKSTATUS(status, "adding collision attribute")

	// collision clusters attribute
	ia_numClusters = fnNumericAttr.create("numclusters", "nclust", MFnNumericData::kInt, DEFAULT_CLUSTERS, &status);
    MCHECKSTATUS(status, "creating numclusters attribute")
    fnNumericAttr.setKeyable(false);
    status = addAttribute(ia_numClusters);
    MCHECKSTATUS(status, "adding numclusters attribute")

	ca_softBodyParam = fnNumericAttr.create("ca_softBodyParam", "casbp", MFnNumericData::kBoolean, 0, &status);
    MCHECKSTATUS(status, "creating ca_softBodyParam attribute")
    fnNumericAttr.setConnectable(false);
    fnNumericAttr.setHidden(true);
    fnNumericAttr.setStorable(false);
    fnNumericAttr.setKeyable(false);
    status = addAttribute(ca_softBodyParam);
    MCHECKSTATUS(status, "adding ca_softBodyParam attribute")

	status = attributeAffects(inputMesh, ca_softBody);
    MCHECKSTATUS(status, "adding attributeAffects(inputMesh, ca_softBody)")
			
	status = attributeAffects(ia_solver, ca_solver);
    MCHECKSTATUS(status, "adding attributeAffects(ia_solver, ca_solver)")

	// connect soft body parameters to ca_softBodyParam connection
	status = attributeAffects(ia_mass, ca_softBodyParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_mass, ca_softBodyParam)")

	status = attributeAffects(ia_dynamicFrictionCoeff, ca_softBodyParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_dynamicFrictionCoeff, ca_softBodyParam)")

	status = attributeAffects(ia_collisionMargin, ca_softBodyParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_collisionMargin, ca_softBodyParam)")
	
	status = attributeAffects(ia_numClusters, ca_softBodyParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_numClusters, ca_softBodyParam)")
	return status;
}



bool SoftBodyNode::setInternalValueInContext ( const  MPlug & plug,
                                                   const  MDataHandle & dataHandle,
                                                   MDGContext & ctx)
{
    return false; 
}

MStatus SoftBodyNode::compute(const MPlug &plug, MDataBlock& data)
{
	if(plug == ca_softBody) 
	{
        this->computeSoftBody(plug, data);
    } 
	else if(plug == ca_softBodyParam) 
	{
        computeSoftBodyParam(plug, data); 
	}
    else if(plug == ca_solver) 
	{
		data.inputValue(ia_solver).asBool();
    } else 
	{
        return MStatus::kUnknownParameter;
    }
    return MStatus::kSuccess;
}

void SoftBodyNode::computeSoftBody(const MPlug &plug, MDataBlock &data)
{
//	std::cout << "(SoftBodyNode::computeSoftBody) | ";

    MObject thisObject(thisMObject());
	
	MPlug plgInputMesh(thisObject, inputMesh);

    MObject update;
    //force evaluation of the shape
    plgInputMesh.getValue(update);
	
	assert(plgInputMesh.isConnected());
	MPlugArray connections;
	plgInputMesh.connectedTo(connections, true, false);
	
	// MFnDependencyNode fnNode(connections[0].node());
	assert( connections.length() != 0);

	// std::cout << "(SoftBodyNode::computeSoftBody) Dependency node fn name: | " << fnNode.name() << std::endl;
	
	MFnMesh meshFn(connections[0].node());
	
	MFnDagNode fnDagNode(thisObject);    
	MFnTransform fnTransform(fnDagNode.parent(0));
	MVector mtranslation = fnTransform.getTranslation(MSpace::kTransform);
    MQuaternion mrotation;
    fnTransform.getRotation(mrotation, MSpace::kObject);
	
	std::cout << "(SoftBodyNode::computeSoftBody) fnTranform: | " << fnTransform.name().asChar() << std::endl;
	std::cout << "(SoftBodyNode::computeSoftBody) fnTranform: | " << mtranslation.x << ", " << mtranslation.y << ", " 
		<< mtranslation.z << std::endl;
	solver_t::remove_soft_body(this->m_soft_body);
	std::vector<int> triVertIndices;
	std::vector<float> triVertCoords;
	
	createHelperMesh(meshFn, triVertIndices, triVertCoords, MSpace::kTransform);
	this->m_soft_body = solver_t::create_soft_body(triVertCoords, triVertIndices);

	solver_t::add_soft_body(this->m_soft_body, this->name().asChar());
//	this->m_soft_body->set_transform(vec3f((float)mtranslation.x, (float)mtranslation.y, (float)mtranslation.z),
//								quatf((float)mrotation.w, (float)mrotation.x, (float)mrotation.y, (float)mrotation.z));

	data.outputValue(ca_softBody).set(true);
    data.setClean(plug);
	
}

soft_body_t::pointer SoftBodyNode::soft_body()
{
	MObject thisObject(thisMObject());
    MObject update;
    MPlug(thisObject, ca_softBody).getValue(update);
	MPlug(thisObject, ca_softBodyParam).getValue(update);

	return this->m_soft_body;
}

void SoftBodyNode::createHelperMesh(MFnMesh &mayaMesh, std::vector<int> &triIndices, std::vector<float> &triVertices, MSpace::Space space)
{
	MFloatPointArray ptArray;
	mayaMesh.getPoints(ptArray, space);	
	
	// append vertex locations (x, y, z) into "flattened array"
	for(int i = 0; i < ptArray.length(); i++)
	{
		MFloatPoint pt;		
		pt = ptArray[i];
		pt.cartesianize();		
		triVertices.push_back(pt.x);
		triVertices.push_back(pt.y);
		triVertices.push_back(pt.z);
		
	}
	std::cout << std::endl;
	
	// create vector of triangle indices
	MIntArray tCounts;
	MIntArray tVerts;
	mayaMesh.getTriangles(tCounts, tVerts);
	triIndices.resize(tVerts.length());
	for(int i = 0; i < tVerts.length(); i ++)
	{
		triIndices[i] = tVerts[i];	
	} 	
}

void SoftBodyNode::update()
{
	MObject thisObject(this->thisMObject());
    MObject update;
	MPlug(thisObject, inputMesh).getValue(update);
    MPlug(thisObject, ca_softBody).getValue(update);
    MPlug(thisObject, ca_softBodyParam).getValue(update);
    MPlug(thisObject, ca_solver).getValue(update);
    // MPlug(thisObject, worldMatrix).elementByLogicalIndex(0).getValue(update);
	
}


void SoftBodyNode::nodeRemoved(MObject& node, void *clientData)
{   
    MFnDependencyNode fnNode(node);
    solver_t::remove_soft_body(static_cast<SoftBodyNode*>(fnNode.userNode())->m_soft_body);
}

void SoftBodyNode::computeSoftBodyParam(const MPlug &plug, MDataBlock &data)
{

	MObject thisObject(thisMObject());
    MObject update;
    
	// update mass
    MPlug(thisObject, ca_softBody).getValue(update);
	float mass = static_cast<float>( data.inputValue(ia_mass).asDouble() );
	this->m_soft_body->set_mass(mass);
	
	// update dynamic friction coefficient
	float dynFrictionCoeff = static_cast<float>( data.inputValue(ia_dynamicFrictionCoeff).asDouble() );	
	this->m_soft_body->set_dynamic_friction_coeff(dynFrictionCoeff);
	data.outputValue(ca_softBodyParam).set(true);

	// update collision margin
	float collisionMargin = data.inputValue(ia_collisionMargin).asFloat();
	this->m_soft_body->set_collision_margin(collisionMargin);
	data.setClean(plug);

	// number of collision clusters
	int numClust = data.inputValue(ia_numClusters).asInt();
	// this->m_soft_body->set_
}

void SoftBodyNode::computeWorldMatrix(const MPlug& plug, MDataBlock& data)
{
  
}

// kludge to reinitialize soft body
// TODO: do this by forcing call of computeSoftBody ?
void SoftBodyNode::set_soft_body(soft_body_t::pointer sb)
{
	this->m_soft_body = sb;
}
