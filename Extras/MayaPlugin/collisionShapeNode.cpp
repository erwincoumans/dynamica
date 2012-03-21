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
*/
//collisionShapeNode.cpp

#include <maya/MFnMessageAttribute.h>
#include <maya/MFnNumericAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MPlugArray.h>
#include <maya/MFloatPointArray.h>
#include <maya/MFloatVectorArray.h>
#include <maya/MIntArray.h>
#include <maya/MFnMesh.h>
#include <maya/MFnTransform.h>
#include <maya/MQuaternion.h>
#include <maya/MDagPath.h>

#include <iterator>
#include <vector>
#include <assert.h>

#include "collisionShapeNode.h"
#include "mayaUtils.h"
#include "solver.h"
#include "LinearMath/btGeometryUtil.h"

#define UPDATE_SHAPE 0 //future collision margin adjust

MTypeId     collisionShapeNode::typeId(0x100332);
MString     collisionShapeNode::typeName("dCollisionShape");

MObject     collisionShapeNode::ia_shape;
MObject     collisionShapeNode::ia_type;
MObject     collisionShapeNode::ia_scale;
MObject     collisionShapeNode::ca_collisionShape;
MObject     collisionShapeNode::ca_collisionShapeParam;
MObject     collisionShapeNode::oa_collisionShape;

MStatus collisionShapeNode::initialize()
{
	MStatus status;
	MFnMessageAttribute fnMsgAttr;
	MFnNumericAttribute fnNumericAttr;
	MFnEnumAttribute fnEnumAttr;

	ia_type = fnEnumAttr.create("type", "tp", 7, &status);
	MCHECKSTATUS(status, "creating type attribute")
		fnEnumAttr.addField("Convex Hull", 0);
	fnEnumAttr.addField("Mesh", 1);
	fnEnumAttr.addField("Cylinder", 2);
	fnEnumAttr.addField("Capsule", 3);
	fnEnumAttr.addField("Box", 4);
	fnEnumAttr.addField("Sphere", 5);
	fnEnumAttr.addField("Plane", 6);
	fnEnumAttr.addField("BvhMesh", 7);
	fnEnumAttr.addField("HACD", 8);
	fnEnumAttr.setKeyable(true);
	status = addAttribute(ia_type);
	MCHECKSTATUS(status, "adding type attribute")

		ia_scale = fnNumericAttr.createPoint("scale", "sc", &status);
	MCHECKSTATUS(status, "creating ia_scale attribute")
		fnNumericAttr.setDefault(1.0, 1.0, 1.0);
	fnNumericAttr.setKeyable(true);
	status = addAttribute(ia_scale);
	MCHECKSTATUS(status, "adding ia_scale attribute")

		oa_collisionShape = fnMsgAttr.create("outCollisionShape", "oucs", &status);
	MCHECKSTATUS(status, "creating outCollisionShape attribute")
		status = addAttribute(oa_collisionShape);
	MCHECKSTATUS(status, "adding outCollisionShape attribute")

		ia_shape = fnMsgAttr.create("inShape", "insh", &status);
	MCHECKSTATUS(status, "creating inShape attribute")
		status = addAttribute(ia_shape);
	MCHECKSTATUS(status, "adding inShape attribute")

		ca_collisionShape = fnNumericAttr.create("ca_collisionShape", "ccs", MFnNumericData::kBoolean, 0, &status);
	MCHECKSTATUS(status, "creating ca_collisionShape attribute")
		fnNumericAttr.setWorldSpace(true);
	fnNumericAttr.setConnectable(false);
	fnNumericAttr.setHidden(true);
	fnNumericAttr.setStorable(false);
	fnNumericAttr.setKeyable(false);
	status = addAttribute(ca_collisionShape);
	MCHECKSTATUS(status, "adding ca_collisionShape attribute")

		ca_collisionShapeParam = fnNumericAttr.create("collisionShapeParam", "cspm", MFnNumericData::kBoolean, 0, &status);
	MCHECKSTATUS(status, "creating ca_collisionShapeParam attribute")
		fnNumericAttr.setConnectable(false);
	fnNumericAttr.setHidden(true);
	fnNumericAttr.setStorable(false);
	fnNumericAttr.setKeyable(false);
	status = addAttribute(ca_collisionShapeParam);
	MCHECKSTATUS(status, "adding ca_collisionShapeParam attribute")

		//
		status = attributeAffects(ia_shape, oa_collisionShape);
	MCHECKSTATUS(status, "adding attributeAffects(ia_shape, oa_collisionShape)")

		status = attributeAffects(ia_type, oa_collisionShape);
	MCHECKSTATUS(status, "adding attributeAffects(ia_type, oa_collisionShape)")

		status = attributeAffects(ia_scale, oa_collisionShape);
	MCHECKSTATUS(status, "adding attributeAffects(ia_scale, oa_collisionShape)")

		//
		status = attributeAffects(ia_shape, ca_collisionShape);
	MCHECKSTATUS(status, "adding attributeAffects(ia_shape, ca_collisionShape)")

		status = attributeAffects(ia_type, ca_collisionShape);
	MCHECKSTATUS(status, "adding attributeAffects(ia_shape, ca_collisionShape)")

		//
		status = attributeAffects(ia_shape, ca_collisionShapeParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_shape, oa_collisionShapeParam)")

		status = attributeAffects(ia_scale, ca_collisionShapeParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_scale, oa_collisionShapeParam)")

		status = attributeAffects(ia_type, ca_collisionShapeParam);
	MCHECKSTATUS(status, "adding attributeAffects(ia_type, oa_collisionShapeParam)")

		return MS::kSuccess;
}

collisionShapeNode::collisionShapeNode()
{
	// std::cout << "collisionShapeNode::collisionShapeNode" << std::endl;
	collisionShapeNode::collisionMarginOffset = 0.04;
}

collisionShapeNode::~collisionShapeNode()
{
	// std::cout << "collisionShapeNode::~collisionShapeNode" << std::endl;
}

void* collisionShapeNode::creator()
{
	return new collisionShapeNode();
}

void collisionShapeNode::setCollisionMarginOffset(float offset)
{
	collisionShapeNode::collisionMarginOffset = offset;
}

bool collisionShapeNode::setInternalValueInContext ( const  MPlug & plug,
	const  MDataHandle & dataHandle,
	MDGContext & ctx)
{
	return false; //setInternalValueInContext(plug,dataHandle,ctx);
}

MStatus collisionShapeNode::compute( const MPlug& plug, MDataBlock& data )
{
	//std::cout << "compute collision shape node" << std::endl;
	if(plug == oa_collisionShape) {
		computeOutputShape(plug, data);
	} else if(plug == ca_collisionShape) {
		computeCollisionShape(plug, data);
	} else  if(plug == ca_collisionShapeParam) {
		computeCollisionShapeParam(plug, data);
	} else {
		return MStatus::kUnknownParameter;
	}
	return MStatus::kSuccess;
}

collision_shape_t::pointer collisionShapeNode::collisionShape()
{
	MObject thisObject(thisMObject());
	MObject update;
	MPlug(thisObject, oa_collisionShape).getValue(update);
	MPlug(thisObject, ca_collisionShapeParam).getValue(update);
	return m_collision_shape;
}

static bool isCompound(const MPlug& plgInShape)
{
	MPlugArray plgaConnectedTo;
	plgInShape.connectedTo(plgaConnectedTo, true, true);
	int numSelectedShapes = plgaConnectedTo.length();

	if(numSelectedShapes > 0) {

		MObject node = plgaConnectedTo[0].node();

		MDagPath dagPath;
		MDagPath::getAPathTo(node, dagPath);
		int numChildren = dagPath.childCount();

		if (node.hasFn(MFn::kTransform))
		{
			MFnTransform trNode(dagPath);
			const char* name = trNode.name().asChar();
			printf("name = %s\n", name);

			for (int i=0;i<numChildren;i++)
			{
				MObject child = dagPath.child(i);
				if(child.hasFn(MFn::kMesh))
				{
					return false;
				}

				if(child.hasFn(MFn::kTransform))
				{
					MDagPath dagPath;
					MDagPath::getAPathTo(child, dagPath);
					MFnTransform trNode(dagPath);
					const char* name = trNode.name().asChar();
					printf("name = %s\n", name);

					int numGrandChildren = dagPath.childCount();
					{
						for (int i=0;i<numGrandChildren;i++)
						{
							MObject grandchild = dagPath.child(i);
							if(grandchild.hasFn(MFn::kMesh))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}
	return false;
}

collision_shape_t::pointer collisionShapeNode::createCompositeShape(const MPlug& plgInShape)
{

	std::vector<collision_shape_t::pointer> childCollisionShapes;
	std::vector< vec3f> childPositions;
	std::vector< quatf> childOrientations;

	MPlugArray plgaConnectedTo;
	plgInShape.connectedTo(plgaConnectedTo, true, true);
	int numSelectedShapes = plgaConnectedTo.length();

	if(numSelectedShapes > 0) {

		MObject node = plgaConnectedTo[0].node();

		MDagPath dagPath;
		MDagPath::getAPathTo(node, dagPath);
		int numChildren = dagPath.childCount();

		if (node.hasFn(MFn::kTransform))
		{
			MFnTransform trNode(dagPath);
			MVector mtranslation = trNode.getTranslation(MSpace::kTransform);
			MObject thisObject(thisMObject());

			MFnDagNode fnDagNode(thisObject);
			MStatus status;

			MFnTransform fnParentTransform(fnDagNode.parent(0, &status));

			mtranslation = trNode.getTranslation(MSpace::kPostTransform);
			mtranslation = fnParentTransform.getTranslation(MSpace::kTransform);

			const char* name = trNode.name().asChar();
			printf("name = %s\n", name);

			for (int i=0;i<numChildren;i++)
			{
				MObject child = dagPath.child(i);
				if(child.hasFn(MFn::kMesh))
				{
					return false;
				}

				if(child.hasFn(MFn::kTransform))
				{
					MDagPath dagPath;
					MDagPath::getAPathTo(child, dagPath);
					MFnTransform trNode(dagPath);

					MVector mtranslation = trNode.getTranslation(MSpace::kTransform);
					mtranslation = trNode.getTranslation(MSpace::kPostTransform);

					MQuaternion mrotation;
					trNode.getRotation(mrotation, MSpace::kTransform);
					double mscale[3];
					trNode.getScale(mscale);

					vec3f childPos((float)mtranslation.x, (float)mtranslation.y, (float)mtranslation.z);
					quatf childOrn((float)mrotation.w, (float)mrotation.x, (float)mrotation.y, (float)mrotation.z);

					const char* name = trNode.name().asChar();
					printf("name = %s\n", name);

					int numGrandChildren = dagPath.childCount();
					{
						for (int i=0;i<numGrandChildren;i++)
						{
							MObject grandchild = dagPath.child(i);
							if(grandchild.hasFn(MFn::kMesh))
							{

								collision_shape_t::pointer childShape = createCollisionShape(grandchild);
								if (childShape)
								{
									childCollisionShapes.push_back(childShape);
									childPositions.push_back(childPos);
									childOrientations.push_back(childOrn);
								}
							}
						}
					}
				}
			}
		}
	}

	if (childCollisionShapes.size()>0)
	{

		composite_shape_t::pointer composite = solver_t::create_composite_shape(
			&childCollisionShapes[0],
			&childPositions[0],
			&childOrientations[0],
			childCollisionShapes.size()
			);
		return composite;
	}
	return 0;
}



collision_shape_t::pointer collisionShapeNode::createCollisionShape(const MObject& node)
{

	collision_shape_t::pointer collision_shape = 0;

	MObject thisObject(thisMObject());
	MPlug plgType(thisObject, ia_type);
	int type;
	plgType.getValue(type);

	switch(type) {
	case 0:  
		{
			//convex hull
			{
				if(node.hasFn(MFn::kMesh)) {
					MDagPath dagPath;
					MDagPath::getAPathTo(node, dagPath);
					MFnMesh fnMesh(dagPath);
					MFloatPointArray mpoints;
					MFloatVectorArray mnormals;
					MIntArray mtrianglecounts;
					MIntArray mtrianglevertices;
					fnMesh.getPoints(mpoints, MSpace::kObject);
					fnMesh.getNormals(mnormals, MSpace::kObject);
					fnMesh.getTriangles(mtrianglecounts, mtrianglevertices);

					std::vector<vec3f> vertices(mpoints.length());
					std::vector<vec3f> normals(mpoints.length());
					std::vector<unsigned int> indices(mtrianglevertices.length());

					btAlignedObjectArray<btVector3> btVerts; //mb

					for(size_t i = 0; i < vertices.size(); ++i) {
						vertices[i] = vec3f(mpoints[i].x, mpoints[i].y, mpoints[i].z);
						normals[i] = vec3f(mnormals[i].x, mnormals[i].y, mnormals[i].z);

#if UPDATE_SHAPE //future collision margin adjust
						btVerts.push_back(btVector3(mpoints[i].x, mpoints[i].y, mpoints[i].z)); //mb
#endif
					}
					for(size_t i = 0; i < indices.size(); ++i) {
						indices[i] = mtrianglevertices[i];
					}

#if UPDATE_SHAPE //future collision margin adjust
					btAlignedObjectArray<btVector3> planeEquations;
					btGeometryUtil::getPlaneEquationsFromVertices(btVerts, planeEquations);

					btAlignedObjectArray<btVector3> shiftedPlaneEquations;
					for (int p=0;p<planeEquations.size();p++)
					{
						btVector3 plane = planeEquations[p];
						plane[3] += collisionShapeNode::collisionMarginOffset;
						shiftedPlaneEquations.push_back(plane);
					}

					btAlignedObjectArray<btVector3> shiftedVertices;
					btGeometryUtil::getVerticesFromPlaneEquations(shiftedPlaneEquations, shiftedVertices);

					std::vector<vec3f> shiftedVerticesVec3f(shiftedVertices.size());
					for(size_t i = 0; i < shiftedVertices.size(); ++i) 
					{
						shiftedVerticesVec3f[i] = vec3f(shiftedVertices[i].getX(), shiftedVertices[i].getY(), shiftedVertices[i].getZ());
						//std::cout << "orig verts: " << vertices[i][0] << " " << vertices[i][1] << " " << vertices[i][2] << std::endl;
						//std::cout << "shft verts: " << shiftedVertices[i].getX() << " " << shiftedVertices[i].getY() << " " << shiftedVertices[i].getZ() << std::endl;
						//std::cout << std::endl;
					}

					collision_shape = solver_t::create_convex_hull_shape(&(shiftedVerticesVec3f[0]), shiftedVerticesVec3f.size(), &(normals[0]), &(indices[0]), indices.size());

#endif

#if UPDATE_SHAPE == 0
					collision_shape = solver_t::create_convex_hull_shape(&(vertices[0]), vertices.size(), &(normals[0]), &(indices[0]), indices.size()); //original
#endif
				}
			}
		}

		break;
	case 1:
		{
			//mesh
			{
				if(node.hasFn(MFn::kMesh)) {
					MDagPath dagPath;
					MDagPath::getAPathTo(node, dagPath);
					MFnMesh fnMesh(dagPath);
					MFloatPointArray mpoints;
					MFloatVectorArray mnormals;
					MIntArray mtrianglecounts;
					MIntArray mtrianglevertices;
					fnMesh.getPoints(mpoints, MSpace::kObject);
					fnMesh.getNormals(mnormals, MSpace::kObject);
					fnMesh.getTriangles(mtrianglecounts, mtrianglevertices);

					std::vector<vec3f> vertices(mpoints.length());
					std::vector<vec3f> normals(mpoints.length());
					std::vector<unsigned int> indices(mtrianglevertices.length());

					for(size_t i = 0; i < vertices.size(); ++i) {
						vertices[i] = vec3f(mpoints[i].x, mpoints[i].y, mpoints[i].z);
						normals[i] = vec3f(mnormals[i].x, mnormals[i].y, mnormals[i].z);
					}
					for(size_t i = 0; i < indices.size(); ++i) {
						indices[i] = mtrianglevertices[i];
					}
					bool dynamicMesh = true;
					collision_shape = solver_t::create_mesh_shape(&(vertices[0]), vertices.size(), &(normals[0]),
						&(indices[0]), indices.size(),dynamicMesh);
				}
			}
		}
		break;
	case 2:
		//cylinder
		break;
	case 3:
		//capsule
		break;

	case 7:
		//btBvhTriangleMeshShape
		{

			{
				if(node.hasFn(MFn::kMesh)) {
					MDagPath dagPath;
					MDagPath::getAPathTo(node, dagPath);
					MFnMesh fnMesh(dagPath);
					MFloatPointArray mpoints;
					MFloatVectorArray mnormals;
					MIntArray mtrianglecounts;
					MIntArray mtrianglevertices;
					fnMesh.getPoints(mpoints, MSpace::kObject);
					fnMesh.getNormals(mnormals, MSpace::kObject);
					fnMesh.getTriangles(mtrianglecounts, mtrianglevertices);

					std::vector<vec3f> vertices(mpoints.length());
					std::vector<vec3f> normals(mpoints.length());
					std::vector<unsigned int> indices(mtrianglevertices.length());

					for(size_t i = 0; i < vertices.size(); ++i) {
						vertices[i] = vec3f(mpoints[i].x, mpoints[i].y, mpoints[i].z);
						normals[i] = vec3f(mnormals[i].x, mnormals[i].y, mnormals[i].z);
					}
					for(size_t i = 0; i < indices.size(); ++i) {
						indices[i] = mtrianglevertices[i];
					}
					bool dynamicMesh = false;
					collision_shape = solver_t::create_mesh_shape(&(vertices[0]), vertices.size(), &(normals[0]),
						&(indices[0]), indices.size(),dynamicMesh);
				}
			}
		}
		break;
	case 8:
		//btBvhTriangleMeshShape
		{

			{
				if(node.hasFn(MFn::kMesh)) {
					MDagPath dagPath;
					MDagPath::getAPathTo(node, dagPath);
					MFnMesh fnMesh(dagPath);
					MFloatPointArray mpoints;
					MFloatVectorArray mnormals;
					MIntArray mtrianglecounts;
					MIntArray mtrianglevertices;
					fnMesh.getPoints(mpoints, MSpace::kObject);
					fnMesh.getNormals(mnormals, MSpace::kObject);
					fnMesh.getTriangles(mtrianglecounts, mtrianglevertices);

					std::vector<vec3f> vertices(mpoints.length());
					std::vector<vec3f> normals(mpoints.length());
					std::vector<unsigned int> indices(mtrianglevertices.length());

					for(size_t i = 0; i < vertices.size(); ++i) {
						vertices[i] = vec3f(mpoints[i].x, mpoints[i].y, mpoints[i].z);
						normals[i] = vec3f(mnormals[i].x, mnormals[i].y, mnormals[i].z);
					}
					for(size_t i = 0; i < indices.size(); ++i) {
						indices[i] = mtrianglevertices[i];
					}
					bool dynamicMesh = false;
					collision_shape = solver_t::create_hacd_shape(&(vertices[0]), vertices.size(), &(normals[0]),
						&(indices[0]), indices.size(),dynamicMesh);
				}
			}
		}
		break;
	default:
		{
		}
	}

	return collision_shape;
}

void collisionShapeNode::computeCollisionShape(const MPlug& plug, MDataBlock& data)
{
	//std::cout << collisionShapeNode::collisionMarginOffset << std::endl;

	MObject thisObject(thisMObject());

	MPlug plgInShape(thisObject, ia_shape);


	if (isCompound(plgInShape))
	{
		m_collision_shape = createCompositeShape(plgInShape);
	} else
	{

		MObject thisObject(thisMObject());
		MPlug plgType(thisObject, ia_type);
		int type;
		plgType.getValue(type);

		switch(type)
		{
		case 4:
			//box
			m_collision_shape = solver_t::create_box_shape();
			break;
		case 5:
			//sphere
			m_collision_shape = solver_t::create_sphere_shape();
			break;
		case 6:
			//plane
			m_collision_shape = solver_t::create_plane_shape();
			break;

		default:
			{
				MPlugArray plgaConnectedTo;
				plgInShape.connectedTo(plgaConnectedTo, true, true);
				int numSelectedShapes = plgaConnectedTo.length();

				if(numSelectedShapes > 0) 
				{
					MObject node = plgaConnectedTo[0].node();
					m_collision_shape = createCollisionShape(node);
				}
			}
		}
	}



	//btAssert(m_collision_shape);

	data.setClean(plug);
}


void collisionShapeNode::computeCollisionShapeParam(const MPlug& plug, MDataBlock& data)
{
	// std::cout << "collisionShapeNode::computeCollisionShapeParam" << std::endl;

	MObject thisObject(thisMObject());

	MObject update;
	MPlug(thisObject, ia_shape).getValue(update);
	MPlug(thisObject, ia_type).getValue(update);

	float3& scale = data.inputValue(ia_scale).asFloat3();

	if (m_collision_shape)
		m_collision_shape->set_scale(vec3f(scale[0], scale[1], scale[2]));

	data.setClean(plug);
}

void collisionShapeNode::computeOutputShape(const MPlug& plug, MDataBlock& data)
{
	// std::cout << "collisionShapeNode::computeOutputShape" << std::endl;

	MObject thisObject(thisMObject());

	MObject update;
	MPlug(thisObject, ca_collisionShape).getValue(update);
	MPlug(thisObject, ca_collisionShapeParam).getValue(update);

	data.setClean(plug);
}
