#include <NxPhysics.h>

#include <max.h>
#include <MAXScrpt\MAXScrpt.h>
#include <SimpObj.h>

#include "PxPlugin.h"

#include "MxUtils.h"
#include "MxPluginData.h"
#include "MxObjects.h"
//#include "MxShape.h"
#include "MxFluid.h"
#include "MxFluidEmitter.h"

extern CharStream *gCurrentstream;

#define DEG2RAD (3.1415926535897932384626433832795f/180.0f)

MxFluidEmitter::MxFluidEmitter(const char* name, INode *node, MxFluid* fluid) : MxObject(name, node)
{
	m_ObjectType = MX_OBJECTTYPE_FLUIDEMITTER;
	m_fluidEmitter         = NULL;
	m_fluid			= fluid;

	NxFluidEmitterDesc fluidEmitterDesc;
	InitFluidEmitterDescProps(fluidEmitterDesc);
	CreateFluidEmitter(fluidEmitterDesc);
}

void MxFluidEmitter::CreateFluidEmitter(NxFluidEmitterDesc& desc) 
{
	if (!desc.isValid()) {
		gCurrentstream->printf("FluidEmitter Error: FluidEmitter descriptor is not valid\n");
		return;
	}

	NxFluid* fluid = m_fluid->getNxFluid();
	if (fluid)
	{
		m_fluidEmitter = fluid->createEmitter(desc);
		if (m_fluidEmitter) {
			m_fluidEmitter->userData = this;
		}
	}
}

MxFluidEmitter::~MxFluidEmitter()
{
	Free();
}

void MxFluidEmitter::Free(void)
{
	if(m_fluidEmitter)
	{
		NxFluid* fluid = m_fluid->getNxFluid();
		if (fluid != NULL) {
			fluid->releaseEmitter(*m_fluidEmitter);
		}
		m_fluidEmitter = NULL;
	}
}

void MxFluidEmitter::InitFluidEmitterDescProps(NxFluidEmitterDesc &desc)
{
	TimeValue t     = GetCOREInterface()->GetTime();
	ObjectState os  = m_node->EvalWorldState(t); 

	if (os.obj == NULL) return;

	//os.obj->GetParamBlockByID(0) is the first parameter block
	IParamBlock2* pb = os.obj->GetParamBlockByID(0);

	desc.dimensionX = MxParamUtils::GetFloatParam(pb, t, "DimensionX", 0.25f);
	desc.dimensionY = MxParamUtils::GetFloatParam(pb, t, "DimensionY", 0.25f);

	desc.flags = 0; //should be initialized to 0, since we provide the default values below
	if (MxParamUtils::GetBoolParam(pb, t, "EmitterFlagsEnabled", true))
		desc.flags |= NX_FEF_ENABLED;
	if (MxParamUtils::GetBoolParam(pb, t, "EmitterFlagsForceOnBody", false))
		desc.flags |= NX_FEF_FORCE_ON_BODY;
	if (MxParamUtils::GetBoolParam(pb, t, "EmitterFlagsAddBodyVelocity", true))
		desc.flags |= NX_FEF_ADD_BODY_VELOCITY;
	if (MxParamUtils::GetBoolParam(pb, t, "EmitterFlagsVisualization", true))
		desc.flags |= NX_FEF_VISUALIZATION;

	desc.fluidVelocityMagnitude = MxParamUtils::GetFloatParam(pb, t, "FluidVelocityMagnitude", 1.0f);
	
	desc.maxParticles = MxParamUtils::GetIntParam(pb, t, "MaxParticles", 0);
	desc.particleLifetime = MxParamUtils::GetFloatParam(pb, t, "ParticleLifetime", 0.0f);
	desc.randomAngle = MxParamUtils::GetFloatParam(pb, t, "RandomAngle", 0.0f) * DEG2RAD;
	desc.randomPos = MxParamUtils::GetVectorParam(pb, t, "RandomPos", NxVec3(0, 0, 0));
	desc.rate = MxParamUtils::GetFloatParam(pb, t, "Rate", 100.0f);

	NxMat34 emitterPose(NxMat33(NX_IDENTITY_MATRIX), NxVec3(0, 0, 0));
	if (m_node) emitterPose = MxMathUtils::MaxMatrixToNx(m_node->GetObjectTM(t)); //is it correct to use GetObjectTM()? I think so, that is the one that also takes into account the offset transform in Max?

	INode* shapeReference = MxParamUtils::GetINodeParam(pb, t, "FrameShape", NULL);
	if (shapeReference) {
		//first find out if it is a valid NxShape
		NxShape* nxShape = MxUtils::GetNxShapeFromName(shapeReference->GetName());
		
		if (nxShape != NULL) {
			NxMat34 referenceFrame = MxMathUtils::MaxMatrixToNx(shapeReference->GetObjectTM(t));

			//have to adjust the reference frame depending on the geometry type to which it is attached (because of the different pivot point settings between 3ds max and PhysX)
			NxVec3 offset = NxVec3(0, 0, 0);
			ObjectState refOS = shapeReference->EvalWorldState(t);
			if (refOS.obj != NULL) {
				Class_ID geomID = refOS.obj->ClassID();
				if(geomID == Class_ID(BOXOBJ_CLASS_ID, 0)) {
					SimpleObject* so = (SimpleObject*) refOS.obj;
					float z;
					so->pblock->GetValue(BOXOBJ_HEIGHT, 0, z, FOREVER);
					offset = -NxVec3(0, 0, z * 0.5f);
				}
				if(geomID == CAPS_CLASS_ID) {
					SimpleObject* so = (SimpleObject*) refOS.obj;
					int centersflag;
					float radius, height;
					so->pblock->GetValue(CAPS_RADIUS , 0, radius, FOREVER);
					so->pblock->GetValue(CAPS_HEIGHT , 0, height, FOREVER);
					so->pblock->GetValue(CAPS_CENTERS, 0, centersflag, FOREVER);
					if(!centersflag) height -= radius*2.0f;
					offset = -NxVec3(0, 0, height/2 + radius);
				}
			}
			NxMat34 tempPose = emitterPose;
			emitterPose = MxMathUtils::transformToLocal(referenceFrame, tempPose);
			emitterPose.t += offset;
			desc.frameShape = nxShape;
		} else {
			//nothing to do here except report to the user?
			gCurrentstream->printf("The fluid emitter \"%s\" has a reference object \"%s\" which is not in the scene, unable to attach the emitter to the shape. Please always add actors before fluids. The emitter will be created without binding to the shape.\n", m_node->GetName(), shapeReference->GetName());
		}
	}
	desc.relPose = emitterPose;

#if NX_SDK_VERSION_NUMBER >= 270
	desc.repulsionCoefficient = MxParamUtils::GetFloatParam(pb, t, "RepulsionCoefficient", 1.0f);
#endif

	desc.shape = NX_FE_RECTANGULAR;
	int tempShapeType = MxParamUtils::GetIntParam(pb, t, "EmitterShape", -1);
	if (tempShapeType == 0) {
		desc.shape = NX_FE_RECTANGULAR;
	} else if (tempShapeType == 1) {
		desc.shape = NX_FE_ELLIPSE;
	} else if (tempShapeType == -1) {
		desc.shape = NX_FE_RECTANGULAR;
	} else {
		//Unknown shape type
		gCurrentstream->printf("FluidEmitter Setting Error: Unknown fluid emitter shape type: %d\n", tempShapeType);
		desc.shape = NX_FE_RECTANGULAR;
	}

	desc.type = NX_FE_CONSTANT_PRESSURE;
	int tempEmitterType = MxParamUtils::GetIntParam(pb, t, "EmitterType", -1);
	if (tempEmitterType == 0) {
		desc.type = NX_FE_CONSTANT_PRESSURE;
	} else if (tempEmitterType == 1) {
		desc.type = NX_FE_CONSTANT_FLOW_RATE;
	} else if (tempEmitterType == -1) {
		desc.type = NX_FE_CONSTANT_PRESSURE;
	} else {
		//Unknown emitter type
		gCurrentstream->printf("FluidEmitter Setting Error: Unknown fluid emitter type: %d\n", tempEmitterType);
		desc.type = NX_FE_CONSTANT_PRESSURE;
	}
}
