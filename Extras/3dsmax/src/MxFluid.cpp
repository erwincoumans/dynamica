
#include <NxPhysics.h>

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>
#include <SimpObj.h>

#include "PxPlugin.h"

#include "MxUtils.h"
#include "MxPluginData.h"
#include "MxObjects.h"
#include "MxFluid.h"

extern CharStream *gCurrentstream;

MxFluid::MxFluid(const char* name, INode* node) : MxObject(name, node)
{
	m_ObjectType = MX_OBJECTTYPE_FLUID;
	m_fluid         = NULL;
	m_compartment	= NULL;
	m_fluidData		= NULL;
	m_numParticles	= 0;
	m_updateNr		= 0;
	m_initParticlesNum = 0;
	m_initParticles = NULL;

	NxFluidDesc fluidDesc;
	InitFluidDescProps(fluidDesc);
	CreateFluid(fluidDesc);

	invalidateMesh();
}

void MxFluid::CreateFluid(NxFluidDesc& desc) 
{
	if (!desc.isValid()) {
		gCurrentstream->printf("Fluid Error: Fluid descriptor is not valid\n");
		return;
	}

	m_fluid = NULL;
	if (gPluginData != NULL)
	{
		NxScene* scene = gPluginData->getScene();
		if (scene != NULL)
		{
			m_fluidData = new ParticleSDK[desc.maxParticles];
			//Setup particle write data.
			NxParticleData particleData;
			particleData.numParticlesPtr = &m_numParticles;
			particleData.bufferPos = &m_fluidData[0].position.x;
			particleData.bufferPosByteStride = sizeof(ParticleSDK);
			particleData.bufferVel = &m_fluidData[0].velocity.x;
			particleData.bufferVelByteStride = sizeof(ParticleSDK);
			particleData.bufferLife = &m_fluidData[0].lifetime;
			particleData.bufferLifeByteStride = sizeof(ParticleSDK);
			particleData.bufferId = &m_fluidData[0].id;
			particleData.bufferIdByteStride = sizeof(ParticleSDK);
			desc.particlesWriteData = particleData;
			if(!MxPluginData::HwAvailable())
				desc.flags &= ~NX_FF_HARDWARE;

			m_fluid = scene->createFluid(desc);
			if (m_fluid) {
				m_fluid->userData = this;
			}
		}
	}

}

MxFluid::~MxFluid()
{
	Free();
}

void MxFluid::Free(void)
{
	NxScene* scene = NULL;
	if (gPluginData != NULL)
	{
		scene = gPluginData->getScene();
	}

	if(m_fluid)
	{
		scene->releaseFluid(*m_fluid);
		m_fluid = 0;
	}

	if (m_compartment != NULL)
	{
		gPluginData->releaseCompartment(m_compartment);
		m_compartment = NULL;
	}

	if (m_fluidData != NULL)
	{
		delete[] m_fluidData;
		m_fluidData = NULL;
	}
	if (m_initParticles != NULL)
	{
		delete[] m_initParticles;
		m_initParticles = NULL;
	}
}

void MxFluid::InitFluidDescProps(NxFluidDesc &desc)
{
	TimeValue t     = GetCOREInterface()->GetTime();
	ObjectState os  = m_node->EvalWorldState(t); 

	if (os.obj == NULL) return;

	//os.obj->GetParamBlockByID(0) is the first parameter block, and (1) is the one with the drains
	IParamBlock2* pb = os.obj->GetParamBlockByID(0);

	desc.setToDefault();
	desc.initialParticleData.setToDefault(); //clear this for now, we don't support initial data yet
	desc.maxParticles = MxParamUtils::GetIntParam(pb, t, "MaxParticles", 32767);
	desc.restParticlesPerMeter = MxParamUtils::GetFloatParam(pb, t, "RestParticlesMeter", 50.0f);
	desc.restDensity = MxParamUtils::GetFloatParam(pb, t, "RestDensity", 1000.0f);
	desc.kernelRadiusMultiplier = MxParamUtils::GetFloatParam(pb, t, "KernelRadiusMultiplier", 1.2f);
	desc.motionLimitMultiplier = MxParamUtils::GetFloatParam(pb, t, "MotionLimitMultiplier", 3.6f); //3.0*kernelRadiusMultiplier
	desc.collisionDistanceMultiplier = MxParamUtils::GetFloatParam(pb, t, "CollisionDistanceMultiplier", 0.12f); //0.1*kernelRadiusMultiplier
	desc.packetSizeMultiplier = MxParamUtils::GetFloatParam(pb, t, "PacketSizeMultiplier", 16.0f);
	desc.stiffness = MxParamUtils::GetFloatParam(pb, t, "Stiffness", 20.0f);
	desc.viscosity = MxParamUtils::GetFloatParam(pb, t, "Viscosity", 6.0f);
	desc.damping = MxParamUtils::GetFloatParam(pb, t, "Damping", 0.0f);
	desc.externalAcceleration = MxParamUtils::GetVectorParam(pb, t, "ExternalAcceleration", NxVec3(0, 0, 0));
#if NX_SDK_VERSION_NUMBER >= 280
	// desc.surfaceTension = MxParamUtils::GetFloatParam(pb, t, "surfaceTension", 20.0f); // not implemented yet
	desc.restitutionForStaticShapes = MxParamUtils::GetFloatParam(pb, t, "restitutionForStaticShapes", 0.5f);
	desc.dynamicFrictionForStaticShapes = MxParamUtils::GetFloatParam(pb, t, "dynamicFrictionForStaticShapes", 0.05f);
	desc.staticFrictionForStaticShapes = MxParamUtils::GetFloatParam(pb, t, "staticFrictionForStaticShapes", 0.05f);
	desc.attractionForStaticShapes = MxParamUtils::GetFloatParam(pb, t, "attractionForStaticShapes", 0.0f);

	desc.restitutionForDynamicShapes = MxParamUtils::GetFloatParam(pb, t, "restitutionForDynamicShapes", 0.5f);
	desc.dynamicFrictionForDynamicShapes = MxParamUtils::GetFloatParam(pb, t, "dynamicFrictionForDynamicShapes", 0.5f);
	desc.staticFrictionForDynamicShapes = MxParamUtils::GetFloatParam(pb, t, "staticFrictionForDynamicShapes", 0.5f);
	desc.attractionForDynamicShapes = MxParamUtils::GetFloatParam(pb, t, "attractionForDynamicShapes", 0.0f);
#else
	desc.staticCollisionRestitution = MxParamUtils::GetFloatParam(pb, t, "StaticRestitution", 0.5f);
	desc.staticCollisionAdhesion = MxParamUtils::GetFloatParam(pb, t, "StaticAdhesion", 0.05f);
	desc.dynamicCollisionRestitution = MxParamUtils::GetFloatParam(pb, t, "DynamicRestitution", 0.5f);
	desc.dynamicCollisionAdhesion = MxParamUtils::GetFloatParam(pb, t, "DynamicAdhesion", 0.5f);
#endif

#if NX_SDK_VERSION_NUMBER >= 280
	desc.numReserveParticles = MxParamUtils::GetIntParam(pb, t, "ReservedParticles", 0);
	desc.fadeInTime = MxParamUtils::GetFloatParam(pb, t, "FadeInTime", 0.0f);
#elif NX_SDK_VERSION_NUMBER >= 270
	desc.numReserveParticles = MxParamUtils::GetIntParam(pb, t, "ReservedParticles", 0);
	desc.fadeInTime = MxParamUtils::GetFloatParam(pb, t, "FadeInTime", 0.0f);
	desc.staticCollisionAttraction = MxParamUtils::GetFloatParam(pb, t, "StaticAttraction", 0.0f);
	desc.dynamicCollisionAttraction = MxParamUtils::GetFloatParam(pb, t, "DynamicAttraction", 0.0f);
#endif

	desc.collisionResponseCoefficient = MxParamUtils::GetFloatParam(pb, t, "CollisionResponse", 0.2f);
	desc.collisionGroup = MxParamUtils::GetIntParam(pb, t, "CollisionGroup", 0);

	desc.simulationMethod = NX_F_SPH;
	int tempSimMethod = MxParamUtils::GetIntParam(pb, t, "SimulationMethod", -1);
	if (tempSimMethod == 0) {
		desc.simulationMethod = NX_F_SPH;
	} else if (tempSimMethod == 1) {
		desc.simulationMethod = NX_F_NO_PARTICLE_INTERACTION;
	} else if (tempSimMethod == 2) {
		desc.simulationMethod = NX_F_MIXED_MODE;
	} else if (tempSimMethod == -1) {
		desc.simulationMethod = NX_F_SPH;
	} else {
		//Unknown mode
		gCurrentstream->printf("Fluid Setting Error: Unknown fluid simulation method: %d\n", tempSimMethod);
		desc.simulationMethod = NX_F_SPH;
	}

	desc.collisionMethod = 0;
	if (MxParamUtils::GetBoolParam(pb, t, "CollisionMethodStatic", true)) 
		desc.collisionMethod |= NX_F_STATIC;
	if (MxParamUtils::GetBoolParam(pb, t, "CollisionMethodDynamic", true)) 
		desc.collisionMethod |= NX_F_DYNAMIC;

	desc.flags = 0; //should be initialized to 0, since we provide the default values below
	if (MxParamUtils::GetBoolParam(pb, t, "FluidFlagsEnabled", true))
		desc.flags |= NX_FF_ENABLED;
	if (MxParamUtils::GetBoolParam(pb, t, "FluidFlagsHardware", true) && MxPluginData::HwAvailable())
		desc.flags |= NX_FF_HARDWARE;
	if (MxParamUtils::GetBoolParam(pb, t, "FluidFlagsTwoway", false))
		desc.flags |= NX_FF_COLLISION_TWOWAY;
#if NX_SDK_VERSION_NUMBER >= 270
	if (MxParamUtils::GetBoolParam(pb, t, "FluidFlagsPriorityMode", false))
		desc.flags |= NX_FF_PRIORITY_MODE;
#endif
	if (MxParamUtils::GetBoolParam(pb, t, "FluidFlagsDisableGravity", false))
		desc.flags |= NX_FF_DISABLE_GRAVITY;
	if (MxParamUtils::GetBoolParam(pb, t, "FluidFlagsVisualization", true))
		desc.flags |= NX_FF_VISUALIZATION;

	//Compartment
	desc.compartment = NULL;
	int compartmentID = MxParamUtils::GetIntParam(pb, t, "FluidCompartment", 0);
	if (compartmentID != 0)
	{
		m_compartment = gPluginData->createCompartment(NX_SCT_FLUID, compartmentID);
		if (m_compartment != NULL)
		{
			desc.compartment = m_compartment->getNxCompartment();
		}
	}

	//Initial particles
	INode* initParticlesNode = MxParamUtils::GetINodeParam(pb, t, "InitialParticles", NULL);
	if (initParticlesNode != NULL)
	{
		BOOL needDel = FALSE;
		TriObject* tri = MxUtils::GetTriObjectFromNode(initParticlesNode, t, needDel);
		if (tri != NULL) {
			Mesh& mesh = tri->GetMesh();

			m_initParticlesNum = mesh.getNumVerts();
			m_initParticles = new ParticleSDK[m_initParticlesNum];

			NxMat34 fluidFrame = MxMathUtils::MaxMatrixToNx(m_node->GetObjectTM(t));
			NxMat34 particlesFrame = MxMathUtils::MaxMatrixToNx(initParticlesNode->GetObjectTM(t));
			NxMat34 diffFrame = MxMathUtils::transformToLocal(fluidFrame, particlesFrame);

			for (NxU32 i = 0; i < m_initParticlesNum; i++) 
			{
				m_initParticles[i].position = MxMathUtils::transformToParent(particlesFrame, MxMathUtils::Point3ToNxVec3(mesh.verts[i]));
			}

			NxParticleData initParticleData;			
			initParticleData.numParticlesPtr = &m_initParticlesNum;
			initParticleData.bufferPos = &m_initParticles[0].position.x;
			initParticleData.bufferPosByteStride = sizeof(ParticleSDK);
			initParticleData.bufferVel = &m_initParticles[0].velocity.x;
			initParticleData.bufferVelByteStride = sizeof(ParticleSDK);

			desc.initialParticleData = initParticleData;
		}
	}
}

float* MxFluid::getParticles(int& nrParticles)
{
	NxU32 numParts = m_numParticles;
#if NX_SDK_VERSION_NUMBER < 272
	if (m_updateNr == 1) numParts += m_initParticlesNum;
#endif
	nrParticles = numParts;
	if (numParts == 0) return NULL;
	if (m_fluidData == NULL) return NULL;

	//transform to local
	Matrix3 maxMat = m_node->GetObjectTM(0);
	NxMat34 ref = MxMathUtils::MaxMatrixToNx(maxMat);
	NxMat34 inv;
	ref.getInverse(inv);

	float* buf = new float[numParts*3];
	for (NxU32 i = 0; i < m_numParticles; i++)
	{
		NxVec3 p = (inv * m_fluidData[i].position);
		buf[i*3] = p.x;
		buf[i*3+1] = p.y;
		buf[i*3+2] = p.z;
	}
#if NX_SDK_VERSION_NUMBER < 272
	if (m_updateNr == 1 && m_initParticles != NULL)
	{
		for (NxU32 i = 0; i < m_initParticlesNum; i++)
		{
			NxVec3 p = (inv * m_initParticles[i].position);
			buf[(m_numParticles+i)*3] = p.x;
			buf[(m_numParticles+i)*3+1] = p.y;
			buf[(m_numParticles+i)*3+2] = p.z;
		}
	}
#endif
	return buf;
}

void MxFluid::getParticles(Tab<float>& dest)
{
	int numParts = 0;
	float* buf = getParticles(numParts);
	if (buf != NULL && numParts > 0) 
	{
		dest.Append(numParts*3, buf);
		delete[] buf;
	}
}

void MxFluid::resetObject()
{
	m_fluid->removeAllParticles();

	if (m_initParticlesNum > 0)
	{
		NxParticleData initParticleData;
		initParticleData.numParticlesPtr = &m_initParticlesNum;
		initParticleData.bufferPos = &m_initParticles[0].position.x;
		initParticleData.bufferPosByteStride = sizeof(ParticleSDK);
		initParticleData.bufferVel = &m_initParticles[0].velocity.x;
		initParticleData.bufferVelByteStride = sizeof(ParticleSDK);
		m_fluid->addParticles(initParticleData);
	}
	

	m_updateNr = 0;

	invalidateMesh();
}

Mesh* MxFluid::createMesh(Mesh* currentMesh)
{
	//first get the particles
	int numParts = 0;
	float* buf = getParticles(numParts);

	//then create the mesh, represent each particle with a small tetrahedron
	Mesh* mesh = NULL;
	if (currentMesh != NULL)
		mesh = currentMesh;
	if (mesh == NULL)
		mesh = new Mesh();
	NxU32 numFaces = 12 + numParts*4;
	NxU32 numVerts = 8 + numParts*4;
	mesh->setNumVerts(numVerts);
	mesh->setNumFaces(numFaces);
	mesh->setSmoothFlags(0);
	mesh->setNumTVerts(0);
	mesh->setNumTVFaces(0);
	mesh->displayAllEdges(TRUE);

	const float vertices[] = {
		-1, -1,  1 ,
		1, -1,  1 ,
		-1, -1, -1 ,
		1, -1, -1 ,
		-1,  1,  1 ,
		1,  1,  1 ,
		-1,  1, -1 ,
		1,  1, -1 
	};

	const int triangles[] = {
		0,2,3,
		3,1,0,
		4,5,7,
		7,6,4,
		0,1,5,
		5,4,0,
		1,3,7,
		7,5,1,
		3,2,6,
		6,7,3,
		2,0,4,
		4,6,2
	};

	float xSize = 1.0f;
	float ySize = 1.0f;
	float zSize = 1.0f;

	for (unsigned int i = 0; i < 8; i++) {
		mesh->setVert(i, vertices[i*3+0]*xSize, vertices[i*3+1]*ySize, vertices[i*3+2]*zSize);
	}

	for (unsigned int i = 0; i < 12; i++) {
		mesh->faces[i].setVerts(triangles[i*3+0], triangles[i*3+1], triangles[i*3+2]);
		mesh->faces[i].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_INVIS);
	}

	if (buf == NULL)
		return mesh;

	int faceNr = 12;
	int vertNr = 8;
	for (NxU32 i = 0; i < numParts; i++)
	{
		Point3 p = Point3(buf[i*3+0], buf[i*3+1], buf[i*3+2]);
		mesh->setVert(vertNr+0, p + Point3(0.0f, 0.1f, 0.0f));
		mesh->setVert(vertNr+1, p + Point3(0.0f, -0.1f, 0.1f));
		mesh->setVert(vertNr+2, p + Point3(-0.1f, -0.1f, -0.05f));
		mesh->setVert(vertNr+3, p + Point3(0.1f, -0.1f, -0.05f));
		mesh->faces[faceNr+0].setVerts(vertNr, vertNr+3, vertNr+1);
		mesh->faces[faceNr+1].setVerts(vertNr+3, vertNr+2, vertNr+1);
		mesh->faces[faceNr+2].setVerts(vertNr+2, vertNr+3, vertNr+0);
		mesh->faces[faceNr+3].setVerts(vertNr+1, vertNr+2, vertNr+0);
		mesh->faces[faceNr+0].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
		mesh->faces[faceNr+1].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
		mesh->faces[faceNr+2].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
		mesh->faces[faceNr+3].setEdgeVisFlags(EDGE_VIS, EDGE_VIS, EDGE_VIS);
		faceNr += 4;
		vertNr += 4;
	}

	mesh->InvalidateGeomCache(); //check nr faces
	mesh->InvalidateTopologyCache();
	mesh->buildNormals();
	mesh->buildRenderNormals();
	mesh->buildBoundingBox();

	delete[] buf;
	return mesh;
}
