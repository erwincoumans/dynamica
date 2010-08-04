#include <max.h>
#undef min
#undef max
#include "notify.h"

#include <MAXScrpt\MAXScrpt.h>
#include "iparamb2.h"
#include <iiksys.h> 
#include "btBulletDynamicsCommon.h"

//#include <NxCooking.h>

//#include "NXU_Helper.h"
//#include "NXU_Schema.h"
//#include "NXU_Streaming.h"

#include "PxPlugin.h"
#include "PxFunctions.h"
#include "MxUtils.h"
//#include "MxFluid.h"
#include "MxFluidEmitter.h"
#include "MxActor.h"
#include "MxJoint.h"
#include "MxUtils.h"
//#include "MxShape.h"
//#include "MxCloth.h"
//#include "MxSoftBody.h"
//#include "MxForceField.h"
#include "MxPluginData.h"
#include "MxDebugVisualizer.h"
#include "MxExport.h"
#include "MaxWorld.h"
#include "MaxNode.h"


#ifdef PXPLUGIN_SOFTBODY
#include "NxTetra.h"
#endif

#ifndef DEG2RAD
#define DEG2RAD (3.1415926535897932384626433832795f/180.0f)
#endif
#ifndef RAD2DEG
#define RAD2DEG (180.0f/3.1415926535897932384626433832795f)
#endif

class PXPluginClassDesc : public ClassDesc2 {
	public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE);
	const TCHAR *	ClassName() { return "PX"; }				//returns fixed parsable name (scripter-visible name)
	SClass_ID		SuperClassID() { return REF_TARGET_CLASS_ID; }
	Class_ID		ClassID() { return PX_CLASS_ID; }
	const TCHAR* 	Category() { return "PhysX functions"; }
	const TCHAR*	InternalName() { return _T("pxplugin"); }	//returns fixed parsable name (scripter-visible name)
	HINSTANCE		HInstance() { return hInstance; }

	~PXPluginClassDesc() 
	{
		MxUtils::ReleasePlugin(true);
	}
};

/*
static void pxMaxPreOpenNewFile(void *param, NotifyInfo *info);
static void pxMaxPostOpenNewFile(void *param, NotifyInfo *info);

void pxMaxPreOpenNewFile(void *param, NotifyInfo *info)
{
	DebugPrint("max pre-opens a new file!\n");
}

void pxMaxPostOpenNewFile(void *param, NotifyInfo *info)
{
	DebugPrint("max post-opens a new file!\n");
}

void pxMaxSystemPreNew(void *param, NotifyInfo *info)
{
	DebugPrint("max post-opens a new file!\n");
}

void GeneralNotification(void *param, NotifyInfo *info)
{
	switch(info->intcode)
	{
	case 0x6E:
	case 0x02:
	case 0x03:
	case 0x12:
	case 0x01:
	case 0x0A:
	case 0x70:
	case 0x48:
	case 0x8F:
	case 0x8E:
		return;
		break;
	case NOTIFY_FILE_PRE_OPEN:
		DebugPrint("NOTIFY_FILE_PRE_OPEN!\n");
		break;
	case NOTIFY_SYSTEM_PRE_NEW:
		DebugPrint("NOTIFY_SYSTEM_PRE_NEW!\n");
		break;
	case NOTIFY_FILE_POST_OPEN_PROCESS:
		DebugPrint("NOTIFY_FILE_POST_OPEN_PROCESS!\n");
		break;
	//case NOTIFY_SYSTEM_PRE_NEW:
	//	DebugPrint("NOTIFY_SYSTEM_PRE_NEW!\n");
	//	break;
	//case NOTIFY_SYSTEM_PRE_NEW:
	//	DebugPrint("NOTIFY_SYSTEM_PRE_NEW!\n");
	//	break;
	//case NOTIFY_SYSTEM_PRE_NEW:
	//	DebugPrint("NOTIFY_SYSTEM_PRE_NEW!\n");
	//	break;
	//case NOTIFY_SYSTEM_PRE_NEW:
	//	DebugPrint("NOTIFY_SYSTEM_PRE_NEW!\n");
	//	break;
	}
	DebugPrint("GeneralNotification: %Xh!\n", info->intcode);
}
*/

static PXPluginClassDesc pxPluginDescriptor;
ClassDesc2* GetPxPluginDesc()
{
	/*
	Try to use RegisterNotification. It works.
	int result = 0;
	//result = RegisterNotification(pxMaxPreOpenNewFile, 0, NOTIFY_FILE_PRE_OPEN);
	//result = RegisterNotification(pxMaxPostOpenNewFile, 0, NOTIFY_FILE_POST_OPEN);
	//result = RegisterNotification(pxMaxPostOpenNewFile, 0, NOTIFY_SYSTEM_PRE_NEW);
	//for(int i=1; i<164; i++)
	//	result = RegisterNotification(GeneralNotification, 0, i);
	*/
	return &pxPluginDescriptor;
}

static PxFunctions PxInterfaceDescription
(
	PX_INTERFACE_ID, _T("pxplugin"), 0, &pxPluginDescriptor, 0,
	em_setstream , _T("pxSetOutputStream"), 0, TYPE_INT, 0, 1,  
		_T("stream"), 0, TYPE_VALUE,
	em_convexdecomposition  ,    _T("convexDecomposition"), 0, TYPE_INT, 0, 1,
		_T("object"), 0, TYPE_OBJECT,
	em_pxadd       , _T("pxadd")  , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_pxcreateD6JointDesc  , _T("pxcreateD6JointDesc")  , 0, TYPE_INT, 0, 1,
		_T("node")        , 0, TYPE_INODE,
	em_pxaddD6Joint  , _T("pxaddD6Joint")  , 0, TYPE_INT, 0, 1,
		_T("jointDesc")  , 0, TYPE_INT,
	em_pxsetD6JointSwing  , _T("pxsetD6JointSwing")  , 0, TYPE_INT, 0, 5,
		_T("jointDesc")        , 0, TYPE_INT,
		_T("index")		      , 0, TYPE_INT,
		_T("limited")		  , 0, TYPE_BOOL,
		_T("locked")			  , 0, TYPE_BOOL,
		_T("values")			  , 0, TYPE_POINT4_BR,

	em_pxsetD6JointTwist  , _T("pxsetD6JointTwist")  , 0, TYPE_INT, 0, 5,
		_T("jointDesc")        , 0, TYPE_INT,
		_T("twistEnable")      , 0, TYPE_BOOL,
		_T("twistLow")		  , 0, TYPE_FLOAT,
		_T("twistHigh")		  , 0, TYPE_FLOAT,
		_T("values")			  , 0, TYPE_POINT3_BR,

	em_pxsetD6JointLinear  , _T("pxsetD6JointLinear")  , 0, TYPE_INT, 0, 5,
		_T("jointDesc")    , 0, TYPE_INT,
		_T("modeX")        , 0, TYPE_INT,
		_T("modeY")		  , 0, TYPE_INT,
		_T("modeZ")		  , 0, TYPE_INT,
		_T("radius")		  , 0, TYPE_FLOAT,

	em_pxsetD6JointLocalAxis , _T("pxsetD6JointLocalAxis"), 0, TYPE_INT, 0, 5,
		_T("jointDesc")    , 0, TYPE_INT,
		_T("index")        , 0, TYPE_INT,
		_T("axis")		  , 0, TYPE_POINT3_BR,
		_T("normal")		  , 0, TYPE_POINT3_BR,
		_T("anchor")		  , 0, TYPE_POINT3_BR,

	em_pxsetD6JointBreakable , _T("pxsetD6JointBreakable"), 0, TYPE_INT, 0, 4,
		_T("joint")		  , 0, TYPE_INT,
		_T("breakable")	  , 0, TYPE_BOOL,
		_T("maxForce")	  , 0, TYPE_FLOAT,
		_T("maxTorque")	  , 0, TYPE_FLOAT,
		
	em_pxsetD6JointProjection , _T("pxsetD6JointProjection"), 0, TYPE_INT, 0, 4,
		_T("jointDesc")    , 0, TYPE_INT,
		_T("mode")		  , 0, TYPE_INT,
		_T("distance")	  , 0, TYPE_FLOAT,
		_T("angle")		  , 0, TYPE_FLOAT,

	em_pxsetD6JointCollision , _T("pxsetD6JointCollision"), 0, TYPE_INT, 0, 2,
		_T("jointDesc")	  , 0, TYPE_INT,
		_T("value")		  , 0, TYPE_BOOL,

	em_pxsetD6JointGear , _T("pxsetD6JointGear"), 0, TYPE_INT, 0, 3,
		_T("jointDesc")	  , 0, TYPE_INT,
		_T("enabled")	  , 0, TYPE_BOOL,
		_T("ratio")		  , 0, TYPE_FLOAT,

	em_pxaddjoint  , _T("pxaddjoint")  , 0, TYPE_INT, 0, 6,
		_T("inode")        , 0, TYPE_INODE,
		_T("axis")         , 0, TYPE_POINT3_BR,
		_T("normal")       , 0, TYPE_POINT3_BR,
		_T("limits")       , 0, TYPE_POINT4_BR,
		_T("translimitmin"), 0, TYPE_POINT3_BR,
		_T("translimitmax"), 0, TYPE_POINT3_BR,
	em_pxaddcloth     , _T("pxaddcloth")  , 0, TYPE_INT, 0, 2,
		_T("inode")           , 0, TYPE_INODE,
		_T("value")			  , 0, TYPE_BOOL,
	em_pxaddfluid     , _T("pxaddfluid")  , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_pxaddfluidemitter     , _T("pxaddfluidemitter")  , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_pxsetshapeflag  , _T("pxsetshapeflag")  , 0, TYPE_INT, 0, 3,
		_T("inode")          , 0, TYPE_INODE,
		_T("flagname")		, 0, TYPE_TSTR_BV,
		_T("value")			, 0, TYPE_BOOL,
	em_pxremove       , _T("pxremove")  , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_pxremoveall       , _T("pxremoveall")  , 0, TYPE_INT, 0, 0,
	em_pxcalcD6jointfromIK       , _T("pxcalcD6jointfromIK")  , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_setdynamic     , _T("setdynamic")  , 0, TYPE_INT, 0, 2,
		_T("inode")           , 0, TYPE_INODE,
		_T("dynamic")         , 0, TYPE_INT,
	em_isdynamic      , _T("isdynamic")  , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_isactor        , _T("isactor")    , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_isjoint        , _T("isjoint")    , 0, TYPE_INT, 0, 1,
		_T("inode")           , 0, TYPE_INODE,
	em_pxsim       , _T("pxsim")  , 0, TYPE_INT, 0, 1,
		_T("deltat")           , 0, TYPE_FLOAT,
	em_setsubsimsteps       , _T("setSubSimSteps")  , 0, TYPE_INT, 0, 1,
		_T("numSteps")           , 0, TYPE_INT,
	
	em_pxnxuexport    , _T("pxnxuexport")  , 0, TYPE_INT, 0, 2,
		_T("filename")        , 0, TYPE_STRING,
		_T("extension")       , 0, TYPE_STRING,
	em_pxsync       , _T("pxsync")   , 0, TYPE_INT, 0, 0,
	em_pxrestart    , _T("pxrestart"), 0, TYPE_INT, 0, 0,
	em_pxvisualizephysics, _T("pxvisualizephysics"), 0, TYPE_INT, 0, 1,
		_T("enable")			  , 0, TYPE_BOOL,
	em_pxprep       , _T("pxprep")   , 0, TYPE_INT, 0, 0,
	em_pxsnap       , _T("pxsnap")   , 0, TYPE_INT, 0, 1,
		_T("unused")           , 0, TYPE_FLOAT,

	em_updateInitialVelocityAndSpin, _T("updateInitialVelocityAndSpin"), 0, TYPE_INT  ,0,1,
		_T("inode")          ,  0, TYPE_INODE,
	em_getLinearVelocity         , _T("getLinearVelocity")        ,0, TYPE_POINT3_BR  ,0,1,
		_T("inode")           , 0, TYPE_INODE,
	em_setLinearVelocity         , _T("setLinearVelocity")        ,0, TYPE_INT        ,0,2, 
		_T("inode")           , 0, TYPE_INODE,
		_T("linearvelocity"), 0, TYPE_POINT3_BR,
	em_getAngularVelocity        , _T("getAngularVelocity")       ,0, TYPE_POINT3_BR  ,0,1, 		
		_T("inode")           , 0, TYPE_INODE,
	em_setAngularVelocity        , _T("setAngularVelocity")       ,0, TYPE_INT        ,0,2, 
		_T("inode")           , 0, TYPE_INODE,
		_T("angularvelocity"), 0, TYPE_POINT3_BR,
	em_getGlobalPosition         , _T("getGlobalPosition")        ,0, TYPE_POINT3_BR ,0,1, 
		_T("inode")           , 0, TYPE_INODE,
	em_setGlobalPosition         , _T("setGlobalPosition")        ,0, TYPE_INT ,0,2, 
		_T("inode")           , 0, TYPE_INODE,
		_T("position")        , 0, TYPE_POINT3_BR,
	em_getGlobalPose             , _T("getGlobalPose")            ,0, TYPE_MATRIX3_BR ,0,1, 
		_T("inode")           , 0, TYPE_INODE,
	em_setGlobalPose             , _T("setGlobalPose")            ,0, TYPE_INT ,0,2, 
		_T("inode")           , 0, TYPE_INODE,
		_T("pose")            , 0, TYPE_MATRIX3_BR,
	em_getMass                   , _T("getMass")                  ,0, TYPE_FLOAT      ,0,1, 
		_T("inode")           , 0, TYPE_INODE,
	em_getMassSpaceInertiaTensor , _T("getMassSpaceInertiaTensor"),0, TYPE_POINT3_BR  ,0,1, 
		_T("inode")           , 0, TYPE_INODE,
	em_getCMassLocalPose         , _T("getCMassLocalPose")        ,0, TYPE_MATRIX3_BR ,0,1, 
		_T("inode")           , 0, TYPE_INODE,
	em_updateMassFromShapes  , _T("updateMassFromShapes")         ,0, TYPE_INT        ,0,3,
		_T("inode")           , 0, TYPE_INODE,
		_T("density")         , 0, TYPE_FLOAT,
		_T("totalMass")       , 0, TYPE_FLOAT,
	em_setjointlinearlimits  , _T("setjointlinearlimits")         ,0, TYPE_INT        ,0,3,
		_T("inode")           , 0, TYPE_INODE,
		_T("translimitmin"), 0, TYPE_POINT3_BR,
		_T("translimitmax"), 0, TYPE_POINT3_BR,
	em_setjointangularlimits , _T("setjointangularlimits")        ,0, TYPE_INT        ,0,3,
		_T("inode")           , 0, TYPE_INODE,
		_T("rotlimitmin")     , 0, TYPE_POINT3_BR,
		_T("rotlimitmax")     , 0, TYPE_POINT3_BR,
	em_setjointslerpdrive    , _T("setjointslerpdrive")           ,0, TYPE_INT        ,0,3,
		_T("inode")           , 0, TYPE_INODE,
		_T("spring")          , 0, TYPE_FLOAT,
		_T("damping")         , 0, TYPE_FLOAT,
	em_setjointtwistswingdrive , _T("setjointtwistswingdrive")    ,0, TYPE_INT        ,0,4,
		_T("inode")           , 0, TYPE_INODE,
		_T("twistspring")     , 0, TYPE_FLOAT,
		_T("swingspring")     , 0, TYPE_FLOAT,
		_T("damping")         , 0, TYPE_FLOAT,
	em_setjointdriver        ,  _T("setjointdriver")              ,0, TYPE_INT        ,0,3,
		_T("d6jointinode")    , 0, TYPE_INODE,
		_T("driver")          , 0, TYPE_INODE,
		_T("fromstartframe")  , 0, TYPE_INT,

	em_getdynamicfriction    , _T("getDynamicFriction")              ,0, TYPE_FLOAT      ,0,1, 
	    _T("inode"),         0, TYPE_INODE,
	em_getstaticfriction     , _T("getStaticFriction")               ,0, TYPE_FLOAT      ,0,1, 
	    _T("inode"),         0, TYPE_INODE,
	em_getrestitution        , _T("getRestitution")                  ,0, TYPE_FLOAT      ,0,1, 
	    _T("inode"),         0, TYPE_INODE,

	em_setdynamicfriction    , _T("setDynamicFriction"),      0,      TYPE_INT,      0, 2,
	    _T("inode"),         0, TYPE_INODE,
		_T("value"),         0, TYPE_FLOAT,
	em_setstaticfriction     , _T("setStaticFriction"),       0,      TYPE_INT,      0, 2,
	    _T("inode"),         0, TYPE_INODE,
		_T("value"),         0, TYPE_FLOAT,
	em_setrestitution        , _T("setRestitution"),          0,      TYPE_INT,      0, 2,
	    _T("inode"),         0, TYPE_INODE,
		_T("value"),         0, TYPE_FLOAT,
	em_getfps            , _T("getFPS")                  ,0, TYPE_FLOAT      ,0,0, 
	em_printfps          , _T("printFPS")                  ,0, TYPE_BOOL      ,0, 1, 
	    _T("onoff"),         0, TYPE_BOOL,


	em_debugprint     , _T("debugprint")  , 0, TYPE_INT, 0, 0,
	em_describe		,_T("describe"), 0, TYPE_VOID, 0, 0,
	em_getfluidupdatenr, _T("getfluidupdatenr"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE,
	em_getfluidparticlecount, _T("getFluidParticleCount"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE,
	em_getfluidparticles, _T("getfluidparticles"), 0, TYPE_FLOAT_TAB_BV, 0, 1,
		_T("node"), 0, TYPE_INODE,
	em_setsdkparameter, _T("setSDKParameter"), 0, TYPE_INT, 0, 2,
		_T("paramName"), 0, TYPE_TSTR_BV,
		_T("value"), 0, TYPE_FLOAT,
	em_getsdkparameter, _T("getSDKParameter"), 0, TYPE_FLOAT, 0, 1,
		_T("paramName"), 0, TYPE_TSTR_BV,
	em_resetsdkparameter, _T("resetSDKParameter"), 0, TYPE_INT, 0, 1,
		_T("paramName"), 0, TYPE_TSTR_BV,

	em_createconvexhull  ,    _T("createConvexHull"), 0, TYPE_MESH, 0, 3,
		_T("mesh"), 0, TYPE_MESH,
		_T("vertlimit"), 0, TYPE_INT,
		_T("inflation"), 0, TYPE_FLOAT,
	em_createconvexfrompoints  ,    _T("createConvexFromPoints"), 0, TYPE_MESH, 0, 3,
		_T("points"), 0, TYPE_FLOAT_TAB_BR,
		_T("vertlimit"), 0, TYPE_INT,
		_T("inflation"), 0, TYPE_FLOAT,

	em_createinode		,    _T("createINode"), 0, TYPE_INODE, 0, 1,
		_T("object"), 0, TYPE_OBJECT,

	em_getfluidmesh, _T("pxCreateFluidMesh"), 0, TYPE_MESH, 0, 2,
		_T("fluidNode"), 0, TYPE_INODE,
		_T("currentMesh"), 0, TYPE_MESH,

	em_createsoftbodymesh, _T("createSoftBodyMesh"), 0, TYPE_MESH, 0, 6,
		_T("node"), 0, TYPE_INODE,
		_T("createeditablemesh"), 0, TYPE_BOOL,
		_T("subdivision"), 0, TYPE_INT,
		_T("simplification"), 0, TYPE_FLOAT,
		_T("createISOsurface"), 0, TYPE_BOOL,
		_T("singleISO"), 0, TYPE_BOOL,

	em_addsoftbody, _T("pxAddSoftBody"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE,

	em_addforcefield, _T("pxAddForceField"), 0, TYPE_INT, 0, 1,
		_T("node"), 0, TYPE_INODE,

	em_getgravity, _T("getGravity"), 0, TYPE_POINT3_BV, 0, 0,

	em_setgravity, _T("setGravity"), 0, TYPE_INT, 0, 1,
		_T("gravity"), 0, TYPE_POINT3_BV,

	em_getnumcontacts, _T("getNumContacts"), 0, TYPE_INT, 0, 0,
	em_getnextcontact, _T("getNextContact"), 0, TYPE_BOOL, 0, 0, 
	em_getcontactforce, _T("getContactForce"), 0, TYPE_POINT3_BV, 0, 0,
	em_getcontactpoint, _T("getContactPoint"), 0, TYPE_POINT3_BV, 0, 0,
	em_getcontactnode0, _T("getContactNode0"), 0, TYPE_INODE, 0, 0,
	em_getcontactnode1, _T("getContactNode1"), 0, TYPE_INODE, 0, 0,
	em_setcontactfilter, _T("setContactFilter"), 0, TYPE_FLOAT, 0, 1,
		_T("force"), 0, TYPE_FLOAT,
	em_getcontactfilter, _T("getContactFilter"), 0, TYPE_FLOAT, 0, 0,
	em_findsleepingactors, _T("findSleepingActors"), 0, TYPE_INT, 0, 0,
	em_getsleepingactor, _T("getSleepingActor"), 0, TYPE_INODE, 0, 1, 
		_T("index"), 0, TYPE_INT,
	em_mimicstaticrb, _T("mimicStaticRB"), 0, TYPE_BOOL, 0, 1,
		_T("flag"), 0, TYPE_BOOL,

	em_getversion, _T("getVersion"), 0, TYPE_BOOL, 0, 0,
	em_hwavailable, _T("hwAvailable"), 0, TYPE_BOOL, 0, 0,

	em_printtm             , _T("PrintTM")            ,0, TYPE_INT ,0,1, 
		_T("inode")           , 0, TYPE_INODE,

	em_setunitchange      , _T("setUnitChange")      , 0, TYPE_FLOAT,0,1,
		_T("maxToPhysics")           , 0, TYPE_FLOAT,
	em_getunitchange      , _T("getUnitChange")      , 0, TYPE_FLOAT,0,0,

	em_setPivotScale         , _T("setPivotScale")        ,0, TYPE_INT ,0,2, 
		_T("inode")           , 0, TYPE_INODE,
		_T("scale")        , 0, TYPE_POINT3_BR,

	NXIFACE(INT   ,solveriterationcount)
	NXIFACE(INT   ,kinematicwriteback)
	NXIFACE(INT   ,exploitinstances)
	NXIFACE(BOOL  ,debugphysics)
	NXIFACE(BOOL  ,useHardware)
	NXIFACE(BOOL  ,savedefaults)

	// ****** Add another macro function definition thing here (4th of 4 places)  

	end
);

void* PXPluginClassDesc::Create(BOOL loading)
{
	AddInterface(&PxInterfaceDescription);
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//
// PxFunctions implementation
//
//////////////////////////////////////////////////////////////////////////

MxPluginData*		gPluginData		= NULL;
MxDebugVisualizer*	gDebugVisualizer= NULL;
//NxScene*			gScene			= NULL;
MxSDKParam*			gSDKParamSettings=NULL;
//NxActor**			permutation		= NULL;

//following the existing coding style/conventions of the PhysX plugin for now, adding some globals:
btDiscreteDynamicsWorld* gDynamicsWorld = NULL;
btCollisionConfiguration* gCollisionConfiguration= NULL;
btCollisionDispatcher* gDispatcher = NULL;
btBroadphaseInterface* gBroadphase = NULL;
btConstraintSolver* gConstraintSolver = NULL;

//Settings that are available through the interface
Point3	PxFunctions::mSetting_gravity				 (0.0f,0.0f,-9.81f);
int		PxFunctions::mSetting_solveriterationcount	= 4;
int		gSetting_solveriterationcount_old			= 4;
int		PxFunctions::mSetting_kinematicwriteback	= 1;
int		PxFunctions::mSetting_exploitinstances		= 1;
BOOL	PxFunctions::mSetting_debugphysics			= FALSE;
BOOL	PxFunctions::mSetting_useHardware			= TRUE; //Use hardware if available
BOOL	PxFunctions::mSetting_savedefaults			= TRUE; //Save defaults in the XML stream



//////////////////////////////////////////////////////////////////////////
//
// Implementation
//
//////////////////////////////////////////////////////////////////////////

//this is called by the maxscript part of the plugin to set the stream, so if that is not successful, then the stream will continue to be null
int PxFunctions::pxSetOutputStream(Value* val)
{
//	MaxMsgBox(NULL, _T("pxSetOutputStream"), _T("Error"), MB_OK);
	if (!val->_is_charstream()) return 0;
	CharStream* stream = (CharStream*)val;
	gCurrentstream = stream;
	if (gPluginData != NULL) gPluginData->setOutputStream(stream);
	return 1;
}
//
//MxShape* nxaddshape(INode *node, bool needCCD)
//{
//	TimeValue t = GetCOREInterface()->GetTime();
//	Object* obj = node->EvalWorldState(t).obj;
//	if (obj == NULL) return NULL;
//
//	MxShape* shape = NULL;
//	Class_ID id = obj->ClassID();
//	if (id == Class_ID(SPHERE_CLASS_ID, 0)) {
//		shape = MxPluginData::createShape(node, NX_SHAPE_SPHERE, needCCD);
//	} else if (id == Class_ID(BOXOBJ_CLASS_ID, 0)) 
//	{
//		shape = MxPluginData::createShape(node, NX_SHAPE_BOX, needCCD);
//	} else if (id == CAPS_CLASS_ID)
//	{
//		shape = MxPluginData::createShape(node, NX_SHAPE_CAPSULE, needCCD);
//	} else
//	{
//		bool isStatic = false;
//		int phystype = 0;
//		if (node->GetUserPropInt("PhysicsType", phystype))
//		{
//			isStatic = (phystype == 3);
//		}
//		//int isConcaveSetting = 0;
//		//bool isConcave = false;
//		//if (node->GetUserPropInt("IsConcave", isConcaveSetting))
//		//{
//		//	isConcave = isConcaveSetting != 0;
//		//}
//		//if (isConcave) 
//		if(isStatic)
//		{
//			shape = MxPluginData::createShape(node, NX_SHAPE_MESH, needCCD);
//		} else 
//		{
//			shape = MxPluginData::createShape(node, NX_SHAPE_CONVEX, needCCD);
//		}
//	}
//	if (shape != NULL)
//	{
//		return shape;
//	}
//	if (gCurrentstream) gCurrentstream->printf("Unable to create a PhysX shape of node \"%s\".\n", node->GetName());
//	return NULL;
//}

//Releases the whole plugin (SDK, Scene, Objects, HW usage)
int PxFunctions::pxremoveall()
{
	ccMaxWorld::FreeNodes();
	MxObject::PhysXObjects.clear();
	return MxUtils::ReleasePlugin()?1:0;
}

int PxFunctions::pxremove(INode *node)
{
	MaxMsgBox(NULL, _T("pxremove"), _T("Error"), MB_OK);

	ccMaxWorld::FreeNode(node);

	MxJoint* mxJoint = MxUtils::GetJointFromNode(node);
	if (mxJoint != NULL)
		if (MxPluginData::releaseJoint(mxJoint))
			return 1;

	MxActor* mxActor = MxUtils::GetActorFromName(node->GetName());
	if (mxActor != NULL)
		if (MxPluginData::releaseActor(mxActor))
			return 1;



	return 0;
}

int PxFunctions::pxadd(INode *node)
{
	MxUtils::PreparePlugin();

	MaxMsgBox(NULL, _T("pxadd"), _T("Error"), MB_OK);


	TimeValue t = GetCOREInterface()->GetTime();

	//remove previous objects with same name
	MxPluginData::removeOldObjects(node, node->GetName());
	
	//DebugPrint("Go to print debug info test: node %p.", node);

	ObjectState os = node->EvalWorldState(0); 
	if (MxUtils::checkNodeValidity(node, os) < 1) return 0;

	MxActor* mxActor = gPluginData->createActor(node, false);
	if (mxActor == NULL) return 0;


	MaxMsgBox(NULL, _T("mxActor created"), _T("Error"), MB_OK);


	NxActorDesc&  actordesc  = *mxActor->getActorDesc();
	NxBodyDesc&   body       = *mxActor->getBodyDesc();
	body.solverIterationCount = (NxU32)mSetting_solveriterationcount;

	bool staticactor = false;
	int phystype = 0;
	mxActor->getInteractivity() = RB_DYNAMIC;
	if (node->GetUserPropInt("PhysicsType", phystype))
	{
		if(phystype == 3) {
			staticactor = true;
			mxActor->getInteractivity() = RB_STATIC;
		}else if(phystype == 2) {
			mxActor->getInteractivity() = RB_KINEMATIC;
		}
	}

	actordesc.body = staticactor ? 0 : &body;
	if (MxUserPropUtils::GetUserPropFloat(node, "Mass", body.mass))
	{
		if (body.mass < FLT_EPSILON)
		{
			body.flags |= NX_BF_KINEMATIC;
			actordesc.density = 1.0f;
			mxActor->getInteractivity() = RB_KINEMATIC;
		}
	} else
	{
		actordesc.density = 1.0f;
	}

	if (!MxUserPropUtils::GetUserPropFloat(node, "InitialVelocityX", body.linearVelocity[0]))
		body.linearVelocity[0] = 0.0f;
	if (!MxUserPropUtils::GetUserPropFloat(node, "InitialVelocityY", body.linearVelocity[1]))
		body.linearVelocity[1] = 0.0f;
	if (!MxUserPropUtils::GetUserPropFloat(node, "InitialVelocityZ", body.linearVelocity[2]))
		body.linearVelocity[2] = 0.0f;
	if (!MxUserPropUtils::GetUserPropFloat(node, "InitialSpinX", body.angularVelocity[0]))
		body.angularVelocity[0] = 0.0f;
	if (!MxUserPropUtils::GetUserPropFloat(node, "InitialSpinY", body.angularVelocity[1]))
		body.angularVelocity[1] = 0.0f;
	if (!MxUserPropUtils::GetUserPropFloat(node, "InitialSpinZ", body.angularVelocity[2]))
		body.angularVelocity[2] = 0.0f;

	BOOL makeItSleep = FALSE;
	if (node->GetUserPropBool("PutToSleep", makeItSleep) && makeItSleep)
	{
		mxActor->m_sleep = true;
	}

	BOOL needCCD = FALSE;
	node->GetUserPropBool("EnableCCD", needCCD);

	mxActor->createNxActor();

	

	//add all the shapes
	//for (int i = 0; i < shapes.size(); i++) 
	//{
	//	INode* current = shapes[i];
	//	if (!current) continue; //should not be needed, we should not be able to add NULL to the stack

	//	Matrix3 piv(1);
	//	piv.SetTrans(current->GetObjOffsetPos());
	//	PreRotateMatrix(piv,current->GetObjOffsetRot());

	//	//have to handle proxy geometries here, can't be done in nxaddshape, since that returns a single shape descriptor
	//	MxShape* mxShape = nxaddshape(current, needCCD);
	//	if (mxShape == NULL) continue;
	//	NxShapeDesc* shapedesc = mxShape->getShapeDesc(mxActor); //binds the shape to the actor

	//	shapedesc->shapeFlags = GetShapeFlags(current);
	//	if(needCCD)
	//		shapedesc->shapeFlags |= NX_SF_DYNAMIC_DYNAMIC_CCD;

	//	if(isProxyMode)
	//		shapedesc->localPose =  MxMathUtils::MaxMatrixToNx( MxMathUtils::NxMatrixToMax(shapedesc->localPose) * piv * current->GetNodeTM(t) * proxyPoseIvt * proxyRelativePose);
	//	else
	//		shapedesc->localPose =  MxMathUtils::MaxMatrixToNx( MxMathUtils::NxMatrixToMax(shapedesc->localPose) * piv * current->GetNodeTM(t) * actorinv);
	//	MxUserPropUtils::GetUserPropFloat(current,"Mass",shapedesc->mass);
	//	MxUserPropUtils::GetUserPropFloat(current,"Density",shapedesc->density);
	//	if(shapedesc->mass==0 && shapedesc->density==0) {shapedesc->density=1.0f;} // its a kinematic

	//	mxActor->addShape(mxShape);
	//}

	//if (!actordesc.isValid()) 
	//{
	//	if (gCurrentstream) gCurrentstream->printf("Unable to add %s as an actor, it is not correctly specified.\n", node->GetName());
	//	MxPluginData::releaseActor(mxActor);
	//	return 0;
	//}

	//Force the NxActor to be created (otherwise some other functions that depend on its
	//existance won't work, e.g. simulate and export)
//	NxActor* nxActor = mxActor->getNxActor();
//	if (nxActor && mxActor->m_sleep)
//	{
//		nxActor->putToSleep();	
//	}

	return (int) mxActor->getID();
}

// Add a node as cloth...
int PxFunctions::pxaddcloth(INode *node, BOOL isMetalCloth)
{
	return 0;
}

float PxFunctions::getSDKParameter(TSTR paramname)
{
#if 0
	NXU::NxParameter paramNXU = NXU::NX_PARAMS_NUM_VALUES;
	if (NXU::StringToEnum(paramname, paramNXU) && paramNXU < NXU::NX_PARAMS_NUM_VALUES)
	{
		//first check to see if the setting is stored in the root node of the scene
		float value = 0.0f;
		if (getFloatRootSetting(paramname, value))
		{
			//make sure that the stored value is used in this scene
			setSDKParameter(paramname, value);
			return value;
		}

		if (gSDKParamSettings != NULL)
		{
			if (gSDKParamSettings[(NxU32)paramNXU].inUse)
			{
				return gSDKParamSettings[(NxU32)paramNXU].value;
			}
		}

		//if it wasn't specified, then read the value from the SDK (if it exists)
		NxPhysicsSDK* sdk = gPluginData!=NULL?gPluginData->getPhysicsSDK():NULL;
		if (sdk != NULL)
		{
			return sdk->getParameter((NxParameter)paramNXU);
		}

		//no SDK, then we need to get the default value
		return -10.0e10f;
	}
#endif
	return -10.0e10f;
}

//Set a SDK parameter (we need to remember these settings if the SDK is not yet created, or if it is recreated later)
//
//Markus, 2007-06-05:	If we want to keep the settings between different scenes, then we
//						still need to keep the extra array of SDK params, otherwise the
//						settings will only be saved in the root node of the scene and be
//						forgotten when a new scene is created. However, loading an existing
//						scene on top of changed settings could make the loaded scene behave
//						differently than when it was created. Possibly need to call the 
//						reset method when loading a new scene.
int PxFunctions::setSDKParameter(TSTR paramname, float value)
{
#if 0
	if (gSDKParamSettings == NULL)
	{
		gSDKParamSettings = new MxSDKParam[NX_PARAMS_NUM_VALUES];
		for (NxU32 i = 0; i < (NxU32)NX_PARAMS_NUM_VALUES; i++)
		{
			gSDKParamSettings[i].inUse = false;
		}
	}
	NXU::NxParameter paramNXU = NXU::NX_PARAMS_NUM_VALUES;
	if (NXU::StringToEnum(paramname, paramNXU) && paramNXU < NXU::NX_PARAMS_NUM_VALUES)
	{
		//store the value in the root node of the scene, so that it is remembered together with the scene
		setFloatRootSetting(paramname, value);

		bool wasInUse = gSDKParamSettings[(NxU32)paramNXU].inUse;
		gSDKParamSettings[(NxU32)paramNXU].inUse = true;
		gSDKParamSettings[(NxU32)paramNXU].value = value;
		gSDKParamSettings[(NxU32)paramNXU].defaultValue = -10.0e10f;
		if (gPluginData != NULL)
		{
			NxPhysicsSDK* sdk = gPluginData->getPhysicsSDK();
			if (sdk != NULL)
			{
				float oldValue = sdk->getParameter((NxParameter)paramNXU);
				sdk->setParameter((NxParameter)paramNXU, value);
				if (!wasInUse)
					gSDKParamSettings[(NxU32)paramNXU].defaultValue = oldValue;

				if (oldValue != value)
				{
					if (stricmp(paramname.data(), "NX_SKIN_WIDTH") == 0)
					{
						updateskinwidth(value);
					}
				}
			}
			return 1;
		}
	} else
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown SDK parameter: \"%s\"\n", paramname);
		return 0;
	}
#endif

	return 0;
}

int PxFunctions::resetSDKParameter(TSTR paramname)
{
	if (gSDKParamSettings == NULL) return 1; //no settings have been made

	return 1;
#if 0
	NxPhysicsSDK* sdk = NULL;
	if (gPluginData != NULL)
	{
		NxPhysicsSDK* sdk = gPluginData->getPhysicsSDK();
	}

	if (stricmp(paramname.data(), "all") == 0)
	{
		if (sdk != NULL)
		{
			for (NxU32 i = 0; i < (NxU32)NX_PARAMS_NUM_VALUES; i++)
			{
				if (gSDKParamSettings[i].inUse && gSDKParamSettings[i].defaultValue > -10.0e10f)
				{
					sdk->setParameter((NxParameter)i, gSDKParamSettings[i].defaultValue);
					const char* name = NXU::EnumToString((NXU::NxParameter)i);
					if (name != NULL)
						setFloatRootSetting(TSTR(name), gSDKParamSettings[i].defaultValue);
				}
			}
		}
		delete[] gSDKParamSettings;
		gSDKParamSettings = NULL;
		return 1;
	}
	NXU::NxParameter paramNXU = NXU::NX_PARAMS_NUM_VALUES;
	if (NXU::StringToEnum(paramname, paramNXU) && paramNXU < NXU::NX_PARAMS_NUM_VALUES)
	{
		if (sdk != NULL)
		{
			sdk->setParameter((::NxParameter)paramNXU, gSDKParamSettings[(NxU32)paramNXU].defaultValue);
			setFloatRootSetting(paramname, gSDKParamSettings[(NxU32)paramNXU].defaultValue);
		}
		gSDKParamSettings[(NxU32)paramNXU].inUse = false;
		return 1;
	}
#endif

	return 0;
}

int PxFunctions::pxsetshapeflag(INode* node, TSTR flagname, BOOL value) 
{
#if 0
	//Find the shape (if it has been created already)
	NxShape* shape = MxUtils::GetNxShapeFromName(node->GetName());
	//check if we know about the flag that should be set
	FlagHolder flagtable[] =
	{
		FlagHolder("NX_TRIGGER_ON_ENTER",				NX_TRIGGER_ON_ENTER),
		FlagHolder("NX_TRIGGER_ON_LEAVE",				NX_TRIGGER_ON_LEAVE),
		FlagHolder("NX_TRIGGER_ON_STAY",				NX_TRIGGER_ON_STAY),
		FlagHolder("NX_TRIGGER_ENABLE",					NX_TRIGGER_ENABLE),
		FlagHolder("NX_SF_VISUALIZATION",				NX_SF_VISUALIZATION),
		FlagHolder("NX_SF_DISABLE_COLLISION",			NX_SF_DISABLE_COLLISION),
		FlagHolder("NX_SF_FEATURE_INDICES",				NX_SF_FEATURE_INDICES),
		FlagHolder("NX_SF_DISABLE_RAYCASTING",			NX_SF_DISABLE_RAYCASTING),
		FlagHolder("NX_SF_POINT_CONTACT_FORCE",			NX_SF_POINT_CONTACT_FORCE),
		FlagHolder("NX_SF_FLUID_DRAIN",					NX_SF_FLUID_DRAIN),
		FlagHolder("NX_SF_FLUID_DISABLE_COLLISION",		NX_SF_FLUID_DISABLE_COLLISION),
		FlagHolder("NX_SF_FLUID_TWOWAY",				NX_SF_FLUID_TWOWAY),
		FlagHolder("NX_SF_DISABLE_RESPONSE",			NX_SF_DISABLE_RESPONSE),
		FlagHolder("NX_SF_DYNAMIC_DYNAMIC_CCD",			NX_SF_DYNAMIC_DYNAMIC_CCD),
		FlagHolder("NX_SF_DISABLE_SCENE_QUERIES",		NX_SF_DISABLE_SCENE_QUERIES),
		FlagHolder("NX_SF_CLOTH_DRAIN",					NX_SF_CLOTH_DRAIN),
		FlagHolder("NX_SF_CLOTH_DISABLE_COLLISION",		NX_SF_CLOTH_DISABLE_COLLISION),
		FlagHolder("NX_SF_CLOTH_TWOWAY",				NX_SF_CLOTH_TWOWAY),
#ifdef PXPLUGIN_SOFTBODY
		FlagHolder("NX_SF_SOFTBODY_DRAIN",				NX_SF_SOFTBODY_DRAIN),
		FlagHolder("NX_SF_SOFTBODY_DISABLE_COLLISION",	NX_SF_SOFTBODY_DISABLE_COLLISION),
		FlagHolder("NX_SF_SOFTBODY_TWOWAY",				NX_SF_SOFTBODY_TWOWAY)
#endif
	};
	//NxU32 oldFlags = GetShapeFlags(node);
	//NxU32 flag = 0;
	const char* name = flagname.data();
	const NxU32 numFlags = sizeof(flagtable)/sizeof(flagtable[0]);
	for(NxU32 i = 0; i < numFlags; i++)
	{
		if (stricmp(name, flagtable[i].name)==0)
		{
			//result |= flagtable[i].flagValue;
			node->SetUserPropBool(flagtable[i].name, value);
			//BOOL val = FALSE;
			//node->GetUserPropBool(flagtable[i].name, val);
			if(shape) {
				shape->setFlag((NxShapeFlag) flagtable[i].flagValue, value);
				pxadd(node);
			}
			break;
		}
	}
#endif

	//NxU32 newFlags = GetShapeFlags(node);
	return 1;
}

// Add a node as fluid...
int PxFunctions::pxaddfluid(INode *node)
{
	if (node == NULL) return 0;
	return 0;
#if 0
	MxUtils::PreparePlugin();
	MxPluginData::removeOldObjects(node, node->GetName());
	if (gCurrentstream) gCurrentstream->printf("Called pxAddFluid with node %s \n", node->GetName());

	MxFluid* fluid = gPluginData->createFluid(node);
	return fluid ? fluid->getID() : 0;
#endif

}

// Add a node as fluid...
int PxFunctions::pxaddfluidemitter(INode *node)
{
	return 0;
#if 0
	if (node == NULL) return 0;
	MxUtils::PreparePlugin();
	MxPluginData::removeOldObjects(node, node->GetName());
	if (gCurrentstream) gCurrentstream->printf("Called pxAddFluidEmitter with node %s \n", node->GetName());

	//create the emitter
	MxFluidEmitter* emitter = gPluginData->createFluidEmitter(node);
	return emitter ? emitter->getID() : 0;
#endif

}


int PxFunctions::pxcalcD6jointfromIK(INode *node)
{
	Control* tempControl = node->GetTMController();
	if(!tempControl) return 0;

	IIKControl* control = (IIKControl*)tempControl->GetInterface(I_IKCONTROL);
	if(!control) return 0;

	int limited = 0;

	//TODO: set the translation settings
	Point2 trX = control->DofLimits(IKSys::TransX);
	if(control->DofLowerLimited(IKSys::TransX))
	{		
	}
	else if(control->DofUpperLimited(IKSys::TransX))
	{
	}

	Point2 trY = control->DofLimits(IKSys::TransY);
	if(control->DofLowerLimited(IKSys::TransY))
	{

	}
	else if(control->DofUpperLimited(IKSys::TransY))
	{
	}

	Point2 trZ = control->DofLimits(IKSys::TransZ);
	if(control->DofLowerLimited(IKSys::TransZ))
	{

	}
	else if(control->DofUpperLimited(IKSys::TransZ))
	{
	}

	//no high/low for swing in PhysX (only one symetrical value). We take the upper value
	//defined in the IK limits for the joint limit.
	Point2 rotX = control->DofLimits(IKSys::RotX);
	if(control->DofActive(IKSys::RotX) && (control->DofLowerLimited(IKSys::RotX) || control->DofUpperLimited(IKSys::RotX)))
	{
		node->SetUserPropFloat("pmljointswing2", rotX.y*RAD2DEG);
		limited = 1;
	}

	Point2 rotY = control->DofLimits(IKSys::RotY);
	if(control->DofActive(IKSys::RotY) && (control->DofLowerLimited(IKSys::RotY) || control->DofUpperLimited(IKSys::RotY)))
	{
		node->SetUserPropFloat("pmljointswing1", rotY.y*RAD2DEG);
		limited = 1;
	}

	Point2 rotZ = control->DofLimits(IKSys::RotZ);
	if(control->DofActive(IKSys::RotZ))
	{
		if(control->DofLowerLimited(IKSys::RotZ))
		{
			node->SetUserPropFloat("pmljointtwistlow", rotZ.x*RAD2DEG);
			limited = 1;
		}
		if(control->DofUpperLimited(IKSys::RotZ))
		{
			node->SetUserPropFloat("pmljointtwisthigh", rotZ.y*RAD2DEG);
			limited = 1;
		}
	}

	node->SetUserPropInt("pmljointtype", 4);
	node->SetUserPropInt("pmljointlimits", limited);

	return 1;
}

int setnxjointlinearlimits(MxJoint* mxJoint,Point3 &translimitmin, Point3 &translimitmax)
{
#if 0
	if (mxJoint == NULL) return 0;
	NxD6Joint* nxjoint = mxJoint->getNxJoint()!=NULL?mxJoint->getNxJoint()->isD6Joint():NULL;
	nxjoint->purgeLimitPlanes();
	NxD6JointDesc jointdesc;
	nxjoint->saveToDesc(jointdesc);
	Point3 t = translimitmax-translimitmin;
	jointdesc.xMotion     = (t.x>0) ? NX_D6JOINT_MOTION_FREE : NX_D6JOINT_MOTION_LOCKED;
	jointdesc.yMotion     = (t.y>0) ? NX_D6JOINT_MOTION_FREE : NX_D6JOINT_MOTION_LOCKED;
	jointdesc.zMotion     = (t.z>0) ? NX_D6JOINT_MOTION_FREE : NX_D6JOINT_MOTION_LOCKED;
	nxjoint->loadFromDesc(jointdesc);
	nxjoint->setLimitPoint(jointdesc.actor[1]->getGlobalPose()* jointdesc.localAnchor[1]);
	NxVec3 tempCrossed = jointdesc.localAxis[0].cross(jointdesc.localNormal[0]);
	NxMat33 tempM(jointdesc.localAxis[0], jointdesc.localNormal[0], tempCrossed);
	tempM.setTransposed();
	NxMat34 tempMat = ((jointdesc.actor[0])?jointdesc.actor[0]->getGlobalPose(): NxMat34(true)) 
		* NxMat34(tempM, jointdesc.localAnchor[0]);

	if(t.x>0) { nxjoint->addLimitPlane(tempMat.M*NxVec3(1,0,0),tempMat*(NxVec3&)translimitmin) ;  nxjoint->addLimitPlane(tempMat.M*NxVec3(-1,0,0),tempMat*(NxVec3&)translimitmax) ;} 
	if(t.y>0) { nxjoint->addLimitPlane(tempMat.M*NxVec3(0,1,0),tempMat*(NxVec3&)translimitmin) ;  nxjoint->addLimitPlane(tempMat.M*NxVec3(0,-1,0),tempMat*(NxVec3&)translimitmax) ;} 
	if(t.z>0) { nxjoint->addLimitPlane(tempMat.M*NxVec3(0,0,1),tempMat*(NxVec3&)translimitmin) ;  nxjoint->addLimitPlane(tempMat.M*NxVec3(0,0,-1),tempMat*(NxVec3&)translimitmax) ;} 
#endif

	return 1;
}

int PxFunctions::setjointlinearlimits(INode *node, Point3& translimitmin, Point3& translimitmax)
{
	return setnxjointlinearlimits(MxUtils::GetJointFromNode(node),translimitmin,translimitmax);
}

void setjointdescangularlimits(NxD6JointDesc &jointdesc,Point4 &limits)
{
#if 0
	jointdesc.twistLimit.low.value  = limits.z*DEG2RAD;   // assume z is the twist axis  
	jointdesc.twistLimit.high.value = limits.w*DEG2RAD;  // preprocessing should set z to be the one with highest limit.
	jointdesc.swing1Limit.value = limits.x*DEG2RAD;  // assuming: rotlimitmin.x == -rotlimitmax.x  
	jointdesc.swing2Limit.value = limits.y*DEG2RAD;
	float trange = limits.w-limits.z;
	jointdesc.twistMotion  = (trange>=359.0f)? NX_D6JOINT_MOTION_FREE : (trange<=1.0f)?NX_D6JOINT_MOTION_LOCKED : NX_D6JOINT_MOTION_LIMITED;
	jointdesc.swing1Motion = (limits.x>=180) ? NX_D6JOINT_MOTION_FREE : (limits.x<=0) ?NX_D6JOINT_MOTION_LOCKED : NX_D6JOINT_MOTION_LIMITED;
	jointdesc.swing2Motion = (limits.y>=180) ? NX_D6JOINT_MOTION_FREE : (limits.y<=0) ?NX_D6JOINT_MOTION_LOCKED : NX_D6JOINT_MOTION_LIMITED;
#endif

}


int PxFunctions::setjointangularlimits(INode *node,Point3 &rotlimitmin, Point3 &rotlimitmax)
{
#if 0
	MxJoint* mxJoint = MxUtils::GetJointFromNode(node);
	if (mxJoint == NULL) return 0;
	NxD6Joint* nxjoint = mxJoint->getNxJoint()!=NULL?mxJoint->getNxJoint()->isD6Joint():NULL;
	if (nxjoint == NULL) return 0;

	NxD6JointDesc jointdesc;
	nxjoint->saveToDesc(jointdesc);
	setjointdescangularlimits(jointdesc,Point4(rotlimitmax.y,rotlimitmax.z,rotlimitmin.x,rotlimitmax.x));
	nxjoint->loadFromDesc(jointdesc);
#endif

	return 1;
}


int PxFunctions::setjointslerpdrive(INode *node,float spring,float damping)
{
#if 0
	MxJoint* mxJoint = MxUtils::GetJointFromNode(node);
	if (mxJoint == NULL) return 0;
	NxD6Joint* nxjoint = mxJoint->getNxJoint()!=NULL?mxJoint->getNxJoint()->isD6Joint():NULL;
	if (nxjoint == NULL) return 0;

	NxD6JointDesc jointdesc;
	nxjoint->saveToDesc(jointdesc);
	jointdesc.slerpDrive.spring    = spring;
	jointdesc.slerpDrive.damping   = damping;
	jointdesc.slerpDrive.driveType = (spring>0)?NX_D6JOINT_DRIVE_POSITION:0;
	jointdesc.twistDrive.spring    = 0.0f;
	jointdesc.twistDrive.damping   = 0.0f;
	jointdesc.twistDrive.driveType = 0;
	jointdesc.swingDrive.spring    = 0.0f;
	jointdesc.swingDrive.damping   = 0.0f;
	jointdesc.swingDrive.driveType = 0;
	jointdesc.flags |= NX_D6JOINT_SLERP_DRIVE;
	nxjoint->loadFromDesc(jointdesc);
#endif

	return 1;
}

int PxFunctions::setjointtwistswingdrive(INode *node,float twistspring,float swingspring,float damping)
{
#if 0
	MxJoint* mxJoint = MxUtils::GetJointFromNode(node);
	if (mxJoint == NULL) return 0;
	NxD6Joint* nxjoint = mxJoint->getNxJoint()!=NULL?mxJoint->getNxJoint()->isD6Joint():NULL;
	if (nxjoint == NULL) return 0;

	NxD6JointDesc jointdesc;
	nxjoint->saveToDesc(jointdesc);
	jointdesc.slerpDrive.spring    = 0.0f;
	jointdesc.slerpDrive.damping   = 0.0f;
	jointdesc.slerpDrive.driveType = 0;
	jointdesc.twistDrive.spring    = twistspring;
	jointdesc.twistDrive.damping   = damping;
	jointdesc.twistDrive.driveType = (twistspring>0)?NX_D6JOINT_DRIVE_POSITION:0;
	jointdesc.swingDrive.spring    = swingspring;
	jointdesc.swingDrive.damping   = damping;
	jointdesc.swingDrive.driveType = (swingspring>0)?NX_D6JOINT_DRIVE_POSITION:0;
	jointdesc.flags &= ~NX_D6JOINT_SLERP_DRIVE;
	nxjoint->loadFromDesc(jointdesc);
#endif

	return 1;
}

int PxFunctions::setjointdriver(INode* node, INode* driver, int fromstartframe)
{
#if 0
	MxJoint* mxJoint = MxUtils::GetJointFromNode(node);
	if (mxJoint == NULL) return 0;
	//assert(mxJoint->getNode() == node);

	mxJoint->setDriverNode(driver!=NULL?driver:node);
	mxJoint->setFromStartFrame(fromstartframe);
#endif
	return 1;
}

int PxFunctions::pxcreateD6JointDesc(INode* node)
{
	if (node == NULL) return 0;

	MxUtils::PreparePlugin();
	MxPluginData::removeOldObjects(node, node->GetName());

	MxJoint* mxJoint = MxPluginData::createJoint(node);
	if (mxJoint != NULL) return (int)mxJoint->getID();
	return 0;
}

//This method is not of much use at the moment, since the D6 joint is created by the MxJoint object
int PxFunctions::pxaddD6Joint(int jointDesc)
{
#if 0
	//TODO: check so that the NxJoint object can be created almost directly after the "desc" has been created
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxaddD6Joint.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxaddD6Joint is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6Joint* nxjoint = mxJoint->getNxJoint()!=NULL?mxJoint->getNxJoint()->isD6Joint():NULL;
	if (nxjoint == NULL) {
		if (gCurrentstream) gCurrentstream->printf("Unable to create the specified D6 joint, the descriptor is not valid.\n");
		return 0;
	}
	return mxJoint->getID();
#endif
	return 1;

}

int PxFunctions::pxsetD6JointSwing(int jointDesc, int index, BOOL limited, BOOL locked, Point4 values)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointSwing.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointSwing is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();
	if(index == 1)
	{
		desc->swing1Motion = NX_D6JOINT_MOTION_FREE;

		if(limited)
			desc->swing1Motion = NX_D6JOINT_MOTION_LIMITED;
		if(locked)
			desc->swing1Motion = NX_D6JOINT_MOTION_LOCKED;

		desc->swing1Limit.value = DegToRad(values[0]);
		desc->swing1Limit.restitution = values[1];
		desc->swing1Limit.spring = values[2];
		desc->swing1Limit.damping = values[3];
	}
	else if(index == 2)
	{
		desc->swing2Motion = NX_D6JOINT_MOTION_FREE;
		if(limited)
			desc->swing2Motion = NX_D6JOINT_MOTION_LIMITED;
		if(locked)
			desc->swing2Motion = NX_D6JOINT_MOTION_LOCKED;

		desc->swing2Limit.value = DegToRad(values[0]);
		desc->swing2Limit.restitution = values[1];
		desc->swing2Limit.spring = values[2];
		desc->swing2Limit.damping = values[3];
	}
#endif
	return 1;
}

int PxFunctions::pxsetD6JointTwist(int jointDesc, BOOL twistEnable, float twistLow, float twistHigh, Point3 values)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointTwist.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointTwist is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();

	if(twistEnable)
	{
		desc->twistMotion = NX_D6JOINT_MOTION_LIMITED;
		desc->twistLimit.low.value = DegToRad(twistLow);
		desc->twistLimit.low.restitution = values[0];
		desc->twistLimit.low.spring = values[1];
		desc->twistLimit.low.damping = values[2];

		desc->twistLimit.high.value = DegToRad(twistHigh);
		desc->twistLimit.high.restitution = values[0];
		desc->twistLimit.high.spring = values[1];
		desc->twistLimit.high.damping = values[2];
	}
	else
	{
		desc->twistMotion = NX_D6JOINT_MOTION_LOCKED;
	}
#endif

	return 1;
}

int	PxFunctions::pxsetD6JointLinear(int jointDesc, int modeX, int modeY, int modeZ, float radius)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointLinear.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointLinear is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();

	if(modeX == 1) desc->xMotion = NX_D6JOINT_MOTION_LOCKED;
	else if(modeX == 2) desc->xMotion = NX_D6JOINT_MOTION_LIMITED;
	else if(modeX == 3) desc->xMotion = NX_D6JOINT_MOTION_FREE;

	if(modeY == 1) desc->yMotion = NX_D6JOINT_MOTION_LOCKED;
	else if(modeY == 2) desc->yMotion = NX_D6JOINT_MOTION_LIMITED;
	else if(modeY == 3) desc->yMotion = NX_D6JOINT_MOTION_FREE;

	if(modeZ == 1) desc->zMotion = NX_D6JOINT_MOTION_LOCKED;
	else if(modeZ == 2) desc->zMotion = NX_D6JOINT_MOTION_LIMITED;
	else if(modeZ == 3) desc->zMotion = NX_D6JOINT_MOTION_FREE;

	desc->linearLimit.value = radius;
#endif

	return 1;
}

int	PxFunctions::pxsetD6JointLocalAxis(int jointDesc, int index, Point3 axis, Point3 normal, Point3 anchor)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointLocalAxis.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointLocalAxis is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();

	if(index == 1)
	{
		desc->localAxis[0] = NxVec3(axis[0], axis[1], axis[2]);
		desc->localNormal[0] = NxVec3(normal[0], normal[1], normal[2]);
		desc->localAnchor[0] = NxVec3(anchor[0], anchor[1], anchor[2]);
	}
	else if(index == 2)
	{
		desc->localAxis[1] = NxVec3(axis[0], axis[1], axis[2]);
		desc->localNormal[1] = NxVec3(normal[0], normal[1], normal[2]);
		desc->localAnchor[1] = NxVec3(anchor[0], anchor[1], anchor[2]);
	}
#endif
	return 1;
}

int PxFunctions::pxsetD6JointBreakable(int joint, BOOL breakable, float maxForce, float maxTorque)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(joint);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointBreakable.\n", joint);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointBreakable is not a joint ID.\n", joint);
		return 0;
	}
	NxJoint* nxJoint = mxJoint->getNxJoint();
	if (nxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Warning: Unable to create the NxJoint for the specified joint (%d - \"%s\"). Please check the joint properties.", joint, mxJoint->getName());
		return 0;
	}

	if(breakable)
		nxJoint->setBreakable(maxForce,maxTorque);
	else
		nxJoint->setBreakable(NX_MAX_REAL, NX_MAX_REAL);

#endif
	return 1;
}

int PxFunctions::pxsetD6JointProjection(int jointDesc, int mode, float dist, float angle)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointProjection.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointProjection is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();

	if(mode == 1)
	{
		desc->projectionMode = NX_JPM_NONE;
		desc->projectionDistance = 0.1f; //default values
		desc->projectionAngle = 0.0872f;
	}
	else if(mode == 2)
	{
		desc->projectionMode = NX_JPM_POINT_MINDIST;
		desc->projectionDistance = dist;
		desc->projectionAngle = angle;
	}
	else if(mode == 3)
	{
		desc->projectionMode = NX_JPM_LINEAR_MINDIST;
		desc->projectionDistance = dist;
		desc->projectionAngle = angle;
	}
#endif
	return 1;
}

int PxFunctions::pxsetD6JointCollision(int jointDesc, BOOL enabled)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointCollision.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointCollision is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();

	if(enabled)
		desc->jointFlags |= NX_JF_COLLISION_ENABLED;
	else
		desc->jointFlags &= (0xFFFF - NX_JF_COLLISION_ENABLED);
#endif

	return 1;
}

int PxFunctions::pxsetD6JointGear(int jointDesc, BOOL enabled, float ratio)
{
#if 0
	MxObject* object = MxPluginData::getObjectFromId(jointDesc);
	if (object == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("Unknown object identifier (%d) supplied to pxsetD6JointGear.\n", jointDesc);
		return 0;
	}
	MxJoint* mxJoint = object->isJoint();
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("The object identifier (%d) supplied to pxsetD6JointGear is not a joint ID.\n", jointDesc);
		return 0;
	}

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();


	if(enabled)
	{
		desc->jointFlags |= NX_D6JOINT_GEAR_ENABLED;
		desc->gearRatio = ratio;
	}
	else
	{
		desc->jointFlags &= (0xFFFF - NX_D6JOINT_GEAR_ENABLED);
		desc->gearRatio = 1.0f;
	}
#endif

	return 1;
}

int PxFunctions::pxaddjoint(INode* node, Point3& axis, Point3& normal, Point4& limits, Point3& translimitmin, Point3& translimitmax)
{

	if (node == NULL) return 0;
#if 0
	MxUtils::PreparePlugin();
	//MxPluginData::removeOldObjects(node, node->GetName());

	INode* parent = node->GetParentNode();
	while (parent && parent->IsGroupMember()) parent=parent->GetParentNode(); // get the rigid body node for parent if not null
	MxActor* actor1 = MxUtils::GetActorFromNode(node) ;
	MxActor* actor0 = MxUtils::GetActorFromNode(parent);  // returns null if no parent
	NxActor* p = (actor0!=NULL)?actor0->getNxActor():NULL;
	NxActor* c = (actor1!=NULL)?actor1->getNxActor():NULL;

	p = (p && p->isDynamic())?p:NULL;
	if (!c)
	{
		if (gCurrentstream) gCurrentstream->printf("pxaddjoint:  The node \"%s\" has no PhysX SDK actor associated with it, can't add a joint to it.\n", node->GetName());
		return 0;
	}
	MxJoint* mxJoint = MxPluginData::createUninitializedJoint(node);
	if (mxJoint == NULL)
	{
		if (gCurrentstream) gCurrentstream->printf("pxaddjoint: Unable to create a joint object for node \"%s\", check so that the plugin is correctly installed.\n", node->GetName());
		return 0;
	}

	mxJoint->translimitmin=(NxVec3&)translimitmin;
	mxJoint->translimitmax=(NxVec3&)translimitmax;

	if(p) mxJoint->setActor0(actor0);
	if(c) mxJoint->setActor1(actor1);

	NxD6JointDesc* desc = mxJoint->getD6JointDesc();
	setjointdescangularlimits(*desc,limits);

	desc->xMotion = NX_D6JOINT_MOTION_LOCKED;
	desc->yMotion = NX_D6JOINT_MOTION_LOCKED;
	desc->zMotion = NX_D6JOINT_MOTION_LOCKED;
	TimeValue t = GetCOREInterface()->GetTime();
	//TODO: GetObjectTM() instead?
	Matrix3 toparent   = node->GetNodeTM(t) * ((p) ? Inverse(parent->GetNodeTM(t)) : Matrix3(1));
	Matrix3 tochildrot = ((p) ? parent->GetNodeTM(t) : Matrix3(1)) * Inverse(node->GetNodeTM(t)) ;
	tochildrot.NoTrans();
	desc->localAnchor[0]= MxMathUtils::Point3ToNxVec3(Point3(0,0,0) *  toparent); 
	desc->localAxis[0]  = MxMathUtils::Point3ToNxVec3(axis);
	desc->localNormal[0]= MxMathUtils::Point3ToNxVec3(normal);
	desc->localAnchor[1]= NxVec3(0,0,0);
	desc->localAxis[1]  = MxMathUtils::Point3ToNxVec3(axis * tochildrot);
	desc->localNormal[1]= MxMathUtils::Point3ToNxVec3(normal * tochildrot);

	//try and create the NxJoint from the MxJoint object
	NxJoint* nxJoint = mxJoint->getNxJoint();
	if (nxJoint == NULL) 
	{
		if (gCurrentstream) gCurrentstream->printf("pxaddjoint: Joint Creation Failed for joint \"%s\"\n", node->GetName());
		MxPluginData::releaseJoint(mxJoint);
		return 0;
	}

	Point3 trans = translimitmax-translimitmin;
	if (fabsf(trans.x) > FLT_EPSILON || fabsf(trans.y) > FLT_EPSILON || fabsf(trans.z) > FLT_EPSILON)
	{
		setnxjointlinearlimits(mxJoint, translimitmin, translimitmax);
	}

	if (gCurrentstream) gCurrentstream->printf("Created joint %s: node %s to %s.\n", mxJoint->getName(), node->GetName(), (p)?parent->GetName():"'world'");
	return mxJoint->getID();
#endif 
	return 1;

}

void PxFunctions::updateskinwidth(NxReal skinWidth)
{
#if 0

	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return;

	NxU32 actorCount = scene->getNbActors();
	for (NxU32 i = 0; i < actorCount; i++)
	{
		NxActor* actor = scene->getActors()[i];
		NxShape*const* shapes = actor->getShapes();
		NxU32 shapeCount = actor->getNbShapes();
		for (NxU32 j = 0; j < shapeCount; j++)
		{
			shapes[j]->setSkinWidth(skinWidth);
		}
	}
#endif

}

int PxFunctions::pxprep()
{
#if 0
	class ActorDepthCompareClass{
	public:
		static int depth(INode *node)
		{
			int n=0;
			while(node)
			{
				node=node->GetParentNode();
				n++;
			}
			return n;
		}

		static int ActorDepthCompare(const void *_a,const void *_b) {
			NxActor *a = *((NxActor **) _a);
			NxActor *b = *((NxActor **) _b);
			MxActor* mxA = (MxActor*)a->userData;
			MxActor* mxB = (MxActor*)b->userData;
			if (mxA != NULL && mxB != NULL)
			{
				return depth(mxA->getNode()) - depth(mxB->getNode());
			}
			return 0;
		}
	};

	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return 0;

	NxArray<MxObject*> objects;
	MxPluginData::getAllObjects(objects);

	Interface* ip = GetCOREInterface();
	NxU32 i = 0;
	for (i = 0; i < objects.size(); i++)
	{
		//can not simply delete objects and then continue, as removing one object (e.g. an actor can have side effects on e.g. joints)
		MxObject* object = objects[i];
		if ((object && object->getName() != NULL && object->getNode() != NULL) && (object->getNode() != ip->GetINodeByName(object->getName())))
		{
			bool sideEffects = false;
			TSTR name = object->getName();
			bool removed = MxPluginData::releaseObject(object, &sideEffects); //doesn't matter if the object was removed or not (won't be removed if it e.g. is a shape or material
			if (removed && gCurrentstream) gCurrentstream->printf("PhysX plugin: Removed object \"%s\", it has no corresponding INode in the scene.\n", name.data());
			if (sideEffects) //start over if we removed objects that could have removed other objects
			{
				objects.clear();
				MxPluginData::getAllObjects(objects);
				i = (NxU32)-1;
			}
		}
	}
 
	//The permutation is used for updating hierarchies in the correct order
	if (permutation != NULL)
		delete[] permutation;
	permutation = new NxActor*[scene->getNbActors()];
	for (i = 0; i < scene->getNbActors(); i++) 
		permutation[i]=scene->getActors()[i];
	qsort(permutation, scene->getNbActors(), sizeof(NxActor*), ActorDepthCompareClass::ActorDepthCompare);
#endif

	return 1;
}

int PxFunctions::setdynamic(INode *node, int _dynamic)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL)  return  0;
	if (!actor->isDynamic()) //suppress SDK error
	{
		return 0;
	}
	if (_dynamic)
	{
		actor->clearBodyFlag(NX_BF_KINEMATIC);
		mxActor->setAsKenematic(false);
	}
	else
	{
		actor->raiseBodyFlag(NX_BF_KINEMATIC);
		mxActor->setAsKenematic(true);
	}
#endif

	return 1;
}

int PxFunctions::isdynamic(INode *node)
{
	return true;
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return -1;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL)  return  -1;
	if (actor->readBodyFlag(NX_BF_KINEMATIC)) return 0;
	return actor->isDynamic()?1:0;
#endif

}

int PxFunctions::isactor(INode *node)
{
	return 0;
#if 0
	MxActor* actor = MxUtils::GetActorFromNode(node);
	if (actor == NULL) return 0;
	return actor->getID();
#endif

}

int PxFunctions::isjoint(INode *node)
{
	return 0;
#if 0
	MxJoint* mxJoint = MxUtils::GetJointFromNode(node);
	if (mxJoint == NULL) return 0;
	return mxJoint->getID();
#endif
}

int PxFunctions::pxsync()
{
#if 0
	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return 0;

	PxFunctions::pxprep();
	TimeValue t = GetCOREInterface()->GetTime();
	for(NxU32 i = 0; i < scene->getNbActors(); i++)
	{
		NxActor *actor = scene->getActors()[i];
		MxActor* mxActor = (MxActor*)actor->userData;
		if (mxActor == NULL) continue; //happens when you are picking objects in the VRD
		INode *node = mxActor->getNode();
		actor->setGlobalPose(MxMathUtils::MaxMatrixToNx(node->GetNodeTM(t)));
		if(actor->isDynamic() && !actor->readBodyFlag(NX_BF_KINEMATIC))
		{
			NxBodyDesc* bodyDesc = mxActor->getBodyDesc();
			actor->setLinearVelocity(bodyDesc->linearVelocity);
			actor->setAngularVelocity(bodyDesc->angularVelocity);
		}
	}
#endif

	return 1;
}

int PxFunctions::pxrestart()
{
#if 0

	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return 0;

	PxFunctions::pxprep();

	bool findFluid = false;
	TimeValue t = GetCOREInterface()->GetTime();
	for(ccPhysXNodeContainer::iterator it = MxObject::PhysXObjects.physXNodes.begin(); it != MxObject::PhysXObjects.physXNodes.end(); it++) {
		MxObject* mo = *it;
		switch(mo->getType())
		{
		case MX_OBJECTTYPE_ACTOR: 
			{
				MxActor* mxActor = (MxActor*) mo;
				if(mxActor) {
					NxActor* actor = mxActor->getPhysXActor();
					NxActorDesc* actorDesc = mxActor->getActorDesc();
					actor->setGlobalPose(actorDesc->globalPose);
					if(actor->isDynamic() && !actor->readBodyFlag(NX_BF_KINEMATIC))
					{
						NxBodyDesc* bodyDesc = mxActor->getBodyDesc();
						actor->setLinearVelocity(bodyDesc->linearVelocity);
						actor->setAngularVelocity(bodyDesc->angularVelocity);
					}
					INode* node = mxActor->getNode();
					mxActor->resetObject();
					if(mxActor->m_sleep)
					{
						actor->putToSleep();
					}
				}
			}
			break;
		case MX_OBJECTTYPE_CLOTH: 
			{
				MxCloth* mx = (MxCloth*) mo;
				if(mx)
					mx->resetObject();
			}
			break;
		case MX_OBJECTTYPE_SOFTBODY: 
			{
				MxSoftBody* mx = (MxSoftBody*) mo;
				if(mx)
					mx->resetObject();
			}
			break;
		case MX_OBJECTTYPE_FORCEFIELD: 
			{
				MxForceField* mx = (MxForceField*) mo;
				if(mx)
					mx->resetObject();
			}
			break;
		case MX_OBJECTTYPE_FLUID: 
			{
				findFluid = true;
				MxFluid* fluid = (MxFluid*) mo;
				if(fluid)
					fluid->resetObject();
			}
			break;
		case MX_OBJECTTYPE_SHAPE:
		case MX_OBJECTTYPE_JOINT:
		case MX_OBJECTTYPE_MATERIAL:
		case MX_OBJECTTYPE_MESH:
		case MX_OBJECTTYPE_COMPARTMENT:
			break;
		default:
			if (gCurrentstream) {
				INode* node = mo->getNode();
				if(node)
					gCurrentstream->printf("Find unknown object type(%d), node(%s)!\n", mo->getType(), node->GetName());
				else
					gCurrentstream->printf("Find unknown object type(%d)!\n", mo->getType());
			}
		}
	}
	MxObject::PhysXObjects.physXNodes.clear();
	

	//redraw the debug shapes at their correct locations, also need to update the fluids (their number of particles aren't correctly updated until a simulate call has been made)
	//if(mSetting_debugphysics || mxFluids.size() > 0)
	if(mSetting_debugphysics || findFluid)
		pxsim(0.001f); //can't simulate 0.0 when there are fluids or cloths (it seems)
	return 1;
#endif

	return 0;
}


int PxFunctions::pxvisualizephysics(BOOL enable)
{
#if 0
	if(!gDebugVisualizer)
	{
		MaxMsgBox(NULL, _T("Please add physics objects to the scene from the Bullet Control Panel before enabling debug visualization"), _T("Error"), MB_OK);
		return 0;
	}

	if(enable)
	{
		mSetting_debugphysics = TRUE;
		GetCOREInterface()->RegisterViewportDisplayCallback(FALSE, gDebugVisualizer);
	}
	else
	{
		mSetting_debugphysics = FALSE;
		GetCOREInterface()->UnRegisterViewportDisplayCallback(FALSE, gDebugVisualizer);
	}
#endif

	return 1;
}


int PxFunctions::pxsnap(float)
{
#if 0
	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return 0;

	int i;
	if(!theHold.Holding())
	{
		gCurrentstream->printf(" Unable to take snapshot of inodes TM states\n");
		return 0;
	}
	class RestoreTime :public RestoreObj 
	{
	public:
		TimeValue t; 
		RestoreTime(TimeValue _t):t(_t){}
		void Restore(int)
		{
			GetCOREInterface()->SetTime(t);
			pxsync();
		}
		void Redo(){}
		int Size() {return sizeof(*this);}
	};
	class RestoreTrans :public RestoreObj 
	{
	public:
		INode *node ; 
		Matrix3 tm ;  
		TimeValue t;
		RestoreTrans(INode *n,Matrix3& m,TimeValue _t):node(n),tm(m),t(_t){}
		void Restore(int)
		{
			node->SetNodeTM(t,tm);
		}
		
		void Redo(){}
		int Size() 
		{
			return sizeof(*this);
		}
	};
	TimeValue t = GetCOREInterface()->GetTime();
	theHold.Put(new RestoreTime(t));
	i = (int) scene->getNbActors();
	while(i--)
	{
		MxActor* mxActor = (MxActor*)permutation[i]->userData;
		if (mxActor == NULL) continue; //Should only happen when the VRD is running and the user picks an object (which inserts an extra actor)
		INode* node = mxActor->getNode();
		if (mxActor->getProxyNode() != NULL)
		{
			theHold.Put(new RestoreTrans(node,mxActor->getProxyNode()->GetNodeTM(t),t));
			//theHold.Put(new RestoreTrans(mxActor->getProxyNode(),mxActor->getProxyNode()->GetNodeTM(t),t));
		}
		else
		{
			theHold.Put(new RestoreTrans(node,node->GetNodeTM(t),t));
		}
	}
#endif
	return 1;
}

unsigned int PxFunctions::subSimSubsteps = 1;

int PxFunctions::setSubSimSteps(int numSteps)
{
#if 0
	if(numSteps < 1)
		PxFunctions::subSimSubsteps = 1;
	else
		PxFunctions::subSimSubsteps = numSteps;
	return PxFunctions::subSimSubsteps;
#endif
	return 1;
}

class PxClockData;
class PxClock
{
public:
	PxClock();
	virtual ~PxClock();

	void start();
	float stop(); // return time in seconds

private:
	PxClockData* data;
};

struct PxClockData
{
	PxClockData();

	static LARGE_INTEGER lnFreq;

	bool health;
	LARGE_INTEGER start;
	LARGE_INTEGER stop;
};

LARGE_INTEGER PxClockData::lnFreq;

PxClockData::PxClockData() : health(true)
{
	if (0 == lnFreq.QuadPart)
	{
		SetThreadAffinityMask(GetCurrentThread(),1);
		QueryPerformanceFrequency(&lnFreq);
	}
	health = (0 != lnFreq.QuadPart);
}

PxClock::PxClock()  : data(0)
{
	data = new PxClockData;
	start(); 
}

PxClock::~PxClock()
{
	delete data;
}

void PxClock::start()
{
	if(data->health)
		data->health = QueryPerformanceCounter(&data->start);
}

float PxClock::stop()
{
	if(data->health)
		data->health = QueryPerformanceCounter(&data->stop);

	if(data->health)
		return static_cast<double>(data->stop.QuadPart - data->start.QuadPart) / static_cast<double>(data->lnFreq.QuadPart);
	else
		return -1.0f;
}

static PxClock gPxClock;
static float gPxFPS = 0.0f;
static bool gPxPrintFPS = false;

float PxFunctions::getFPS()
{
	return gPxFPS;
}

bool PxFunctions::printFPS(bool onoff)
{
	return gPxPrintFPS = onoff;
}

int PxFunctions::pxsim(float deltat)
{
#if 0
	NxScene* scene = MxPluginData::getSceneStatic();
	if (scene == NULL) return 0;

	scene->setGravity(MxMathUtils::Point3ToNxVec3(mSetting_gravity));

	NxU32 i = 0;
	//Update solver iteration count setting on objects
	if (gSetting_solveriterationcount_old != PxFunctions::mSetting_solveriterationcount) 
	{
		gSetting_solveriterationcount_old = PxFunctions::mSetting_solveriterationcount;
		i = scene->getNbActors();
		while(i--)
		{
			NxActor* actor = scene->getActors()[i];
			actor->setSolverIterationCount(PxFunctions::mSetting_solveriterationcount);
		}
	}

	Interface *ip = GetCOREInterface();
	TimeValue t = ip->GetTime();
	scene->resetJointIterator();
	NxJoint *joint;
	while(joint=scene->getNextJoint())
	{
		if(!joint->userData) continue;
		MxJoint* mxJoint = (MxJoint*)joint->userData;
		if (mxJoint == NULL) continue;
		INode* dnode = mxJoint->getDriverNode();
		if (dnode == NULL) continue; 
		INode* pnode = dnode->GetParentNode();
		Matrix3 drive = dnode->GetNodeTM(t)*((pnode)?Inverse(pnode->GetNodeTM(t)):Matrix3(1)) *
			(mxJoint->fromstartframe 
			? Inverse(dnode->GetNodeTM(0)*((pnode)?Inverse(pnode->GetNodeTM(0)):Matrix3(1)))  
			: Matrix3(1)
			);
		joint->isD6Joint()->setDriveOrientation(NxQuat(MxMathUtils::MaxMatrixToNx(drive).M));
		joint->isD6Joint()->setDrivePosition((NxVec3&) drive.GetRow(3));
	}
	scene->resetJointIterator();
	for(i=0;(unsigned)i<scene->getNbActors();i++)
	{
		NxActor *actor = permutation[i];
		//if (!actor->isDynamic()) continue;
		//if(!actor->readBodyFlag(NX_BF_KINEMATIC)) continue;
		MxActor* mxActor = (MxActor*)actor->userData;
		if (mxActor == NULL) continue; //happens when you are picking objects in the VRD
		mxActor->ActionBeforeSimulation();
		////NxActorDesc ad;
		////actor->saveToDesc(ad);
		////if(ad.body == NULL) continue; // it is static actor
		//NxBodyDesc bd;
		//bool isDynamic = actor->saveBodyToDesc(bd);
		//if(! isDynamic) continue; // it is static actor

		//INode *node = mxActor->getNode();
		//if(!actor->isDynamic()) // static RB
		//{
		//	actor->moveGlobalPose(MxMathUtils::MaxMatrixToNx(node->GetNodeTM(t)));
		//}
		//else if(mxActor->isKenematic())
		//{
		//	if(PxFunctions::m_mimicStaticRB)
		//	{
		//		// this action 'mass = 0' is not allowed by SDK. But it seems it generates the result that I wanted - to wake up sleeping actors
		//		actor->setMass(0.0f);  
		//		actor->moveGlobalPose(MxMathUtils::MaxMatrixToNx(node->GetNodeTM(t)));
		//	}
		//	else
		//	{
		//		actor->setGlobalPose(MxMathUtils::MaxMatrixToNx(node->GetNodeTM(t)));
		//	}
		//} 
		//else if(actor->isDynamic())
		//{
		//	//actor->setGlobalPose(MxMathUtils::MaxMatrixToNx(node->GetNodeTM(t)));
		//}
	}

	MxContactReport::getMyContactReport().init();

	gPxClock.start();
	unsigned int& subSteps = PxFunctions::subSimSubsteps;
	float interval = deltat/subSteps;
	for(int k = 0; k < subSteps; ++k)
	{
		scene->setTiming(0,0,NX_TIMESTEP_VARIABLE );
		//scene->setTiming(deltat, 8,NX_TIMESTEP_FIXED);
		scene->simulate(interval);
		scene->flushStream();
		scene->fetchResults(NX_RIGID_BODY_FINISHED,true);  //
	}

	gPxFPS = 1.0f/gPxClock.stop();
	if(gPxPrintFPS && gCurrentstream)
		gCurrentstream->printf("%f\n", gPxFPS);

	if(MxObject::PhysXObjects.physXNodes.size() < 1)
		MxObject::PhysXObjects.arrange();
	for(ccPhysXNodeContainer::iterator it = MxObject::PhysXObjects.physXNodes.begin(); it != MxObject::PhysXObjects.physXNodes.end(); it++) {
		MxObject* mo = *it;
		switch(mo->getType())
		{
		case MX_OBJECTTYPE_ACTOR: 
			{
				MxActor* mxActor = (MxActor*) mo;
				if(mxActor)
					mxActor->ActionAfterSimulation();
			}
			break;
		case MX_OBJECTTYPE_CLOTH: 
			{
				MxCloth* mx = (MxCloth*) mo;
				if(mx)
					mx->updateMeshFromSimulation(t);
			}
			break;
		case MX_OBJECTTYPE_SOFTBODY: 
			{
				MxSoftBody* mx = (MxSoftBody*) mo;
				if(mx)
					mx->updateMeshFromSimulation(t);
			}
			break;
		case MX_OBJECTTYPE_FORCEFIELD: 
			{
				MxForceField* mx = (MxForceField*) mo;
				if(mx)
					mx->updateMeshFromSimulation(t);
			}
			break;
		case MX_OBJECTTYPE_FLUID: 
			{
				MxFluid* fluid = (MxFluid*) mo;
				if(fluid)
					fluid->invalidateMesh();
			}
			break;
		case MX_OBJECTTYPE_SHAPE:
		case MX_OBJECTTYPE_JOINT:
		case MX_OBJECTTYPE_MATERIAL:
		case MX_OBJECTTYPE_MESH:
		case MX_OBJECTTYPE_COMPARTMENT:
			break;
		default:
			if (gCurrentstream) {
				INode* node = mo->getNode();
				if(node)
					gCurrentstream->printf("Find unknown object type(%d), node(%s)!\n", mo->getType(), node->GetName());
				else
					gCurrentstream->printf("Find unknown object type(%d)!\n", mo->getType());
			}
		}
	}
	
	// Redraw...
	ip->ForceCompleteRedraw();
#endif 
	return 1;
}

int PxFunctions::updateInitialVelocityAndSpin(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();

	if (actor == NULL) return 0;

	NxBodyDesc* bodyDesc = mxActor->getBodyDesc();

	if(!MxUserPropUtils::GetUserPropFloat(node,"InitialVelocityX",bodyDesc->linearVelocity.x))
		bodyDesc->linearVelocity.x = 0.0f;
	if(!MxUserPropUtils::GetUserPropFloat(node,"InitialVelocityY",bodyDesc->linearVelocity.y))
		bodyDesc->linearVelocity.y = 0.0f;
	if(!MxUserPropUtils::GetUserPropFloat(node,"InitialVelocityZ",bodyDesc->linearVelocity.z))
		bodyDesc->linearVelocity.z = 0.0f;
	if(!MxUserPropUtils::GetUserPropFloat(node,"InitialSpinX",bodyDesc->angularVelocity.x))
		bodyDesc->angularVelocity.x = 0.0f;
	if(!MxUserPropUtils::GetUserPropFloat(node,"InitialSpinY",bodyDesc->angularVelocity.y))
		bodyDesc->angularVelocity.y = 0.0f;
	if(!MxUserPropUtils::GetUserPropFloat(node,"InitialSpinZ",bodyDesc->angularVelocity.z))
		bodyDesc->angularVelocity.z = 0.0f;

	actor->setLinearVelocity(bodyDesc->linearVelocity);
	actor->setAngularVelocity(bodyDesc->angularVelocity);
#endif

	return 1;
}

int PxFunctions::setLinearVelocity(INode *node,Point3 &linearvelocity) 
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	actor->setLinearVelocity((NxVec3&)linearvelocity);
#endif

	return 1;
}

int PxFunctions::setAngularVelocity(INode *node, Point3 &angularvelocity)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	actor->setAngularVelocity((NxVec3&)angularvelocity);
#endif
	return 1;
}

Point3 PxFunctions::getGlobalPosition(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return Point3(0,0,0);
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return Point3(0,0,0);
	return (Point3&)actor->getGlobalPosition();
#endif

	return Point3(0,0,0);
}

// it only sets the physics position and  does not change the node's max position
int PxFunctions::setGlobalPosition(INode *node, Point3& position)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	actor->setGlobalPosition((NxVec3&) position);

	TimeValue t = GetCOREInterface()->GetTime();
	node->SetNodeTM(t, MxMathUtils::NxMatrixToMax(actor->getGlobalPose()));
#endif

	return 1;
}


Matrix3 PxFunctions::getGlobalPose(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return Matrix3(true);
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return Matrix3(true);
	return MxMathUtils::NxMatrixToMax(actor->getGlobalPose());
#endif

	return Matrix3(true);
}

// it only sets the physics pose and  does not change the node's max pose
int PxFunctions::setGlobalPose(INode *node, Matrix3& pose)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	actor->setGlobalPose(MxMathUtils::MaxMatrixToNx(pose));
	TimeValue t = GetCOREInterface()->GetTime();
	node->SetNodeTM(t, pose);
#endif
	return 1;
}

Point3  PxFunctions::getLinearVelocity(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return Point3(0,0,0);
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return Point3(0,0,0);
	return (Point3&) actor->getLinearVelocity();
#endif

	return Point3(0,0,0);
}

Point3  PxFunctions::getAngularVelocity(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return Point3(0,0,0);
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return Point3(0,0,0);
	return (Point3&) actor->getAngularVelocity();
#endif

	 return Point3(0,0,0);
}

float      PxFunctions::getDynamicFriction(INode *node)
{

	float result = 0.0f;
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return result;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return result;
	NxU32 n = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	if(n > 0)
	{
		NxScene* scene = &actor->getScene();
		NxMaterialIndex index = shapes[0]->getMaterial();
		NxMaterial* m = scene->getMaterialFromIndex(index);
		NxMaterialDesc mdesc;
		m->saveToDesc(mdesc);
		result = mdesc.dynamicFriction;
	}
#endif

	return result;
}

float      PxFunctions::getStaticFriction(INode *node)
{
	float result = 0.0f;
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return result;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return result;
	NxU32 n = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	if(n > 0)
	{
		NxScene* scene = &actor->getScene();
		NxMaterialIndex index = shapes[0]->getMaterial();
		NxMaterial* m = scene->getMaterialFromIndex(index);
		NxMaterialDesc mdesc;
		m->saveToDesc(mdesc);
		result = mdesc.staticFriction;
	}
#endif
	return result;
}

float      PxFunctions::getRestitution(INode *node)
{
	float result = 0.0f;
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return result;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return result;
	NxU32 n = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	if(n > 0)
	{
		NxScene* scene = &actor->getScene();
		NxMaterialIndex index = shapes[0]->getMaterial();
		NxMaterial* m = scene->getMaterialFromIndex(index);
		NxMaterialDesc mdesc;
		m->saveToDesc(mdesc);
		result = mdesc.restitution;
	}
#endif

	return result;
}

int      PxFunctions::setDynamicFriction(INode *node, float value)
{
#if 0
	node->SetUserPropFloat("Friction", value);

	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	NxU32 n = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	if(n > 0)
	{
		NxScene* scene = &actor->getScene();
		NxMaterialIndex index = shapes[0]->getMaterial();
		NxMaterial* m = scene->getMaterialFromIndex(index);
		NxMaterialDesc mdesc;
		m->saveToDesc(mdesc);
		mdesc.dynamicFriction = value;
		NxMaterial* newMat = MxUtils::findExistingMaterial(scene, mdesc);
		if(newMat == NULL) newMat = scene->createMaterial(mdesc);
		if(newMat)
		{
			index = newMat->getMaterialIndex();
			for(NxU32 i = 0; i < n; ++i)
			{
				shapes[i]->setMaterial(index);
			}
			return 1;
		}
	}
#endif

	return 0;
}

int      PxFunctions::setStaticFriction(INode *node, float value)
{
	node->SetUserPropFloat("StaticFriction", value);
#if 0

	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	NxU32 n = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	if(n > 0)
	{
		NxScene* scene = &actor->getScene();
		NxMaterialIndex index = shapes[0]->getMaterial();
		NxMaterial* m = scene->getMaterialFromIndex(index);
		NxMaterialDesc mdesc;
		m->saveToDesc(mdesc);
		mdesc.staticFriction = value;
		NxMaterial* newMat = MxUtils::findExistingMaterial(scene, mdesc);
		if(newMat == NULL) newMat = scene->createMaterial(mdesc);
		if(newMat)
		{
			index = newMat->getMaterialIndex();
			for(NxU32 i = 0; i < n; ++i)
			{
				shapes[i]->setMaterial(index);
			}
			return 1;
		}
	}
#endif

	return 0;
}

int      PxFunctions::setRestitution(INode *node, float value)
{
	node->SetUserPropFloat("Restitution", value);

#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	NxU32 n = actor->getNbShapes();
	NxShape*const* shapes = actor->getShapes();
	if(n > 0)
	{
		NxScene* scene = &actor->getScene();
		NxMaterialIndex index = shapes[0]->getMaterial();
		NxMaterial* m = scene->getMaterialFromIndex(index);
		NxMaterialDesc mdesc;
		m->saveToDesc(mdesc);
		mdesc.restitution = value;
		NxMaterial* newMat = MxUtils::findExistingMaterial(scene, mdesc);
		if(newMat == NULL) newMat = scene->createMaterial(mdesc);
		if(newMat)
		{
			index = newMat->getMaterialIndex();
			for(NxU32 i = 0; i < n; ++i)
			{
				shapes[i]->setMaterial(index);
			}
			return 1;
		}
	}
#endif

	return 0;
}

float   PxFunctions::getMass(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0.0f;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0.0f;
	return actor->getMass();
#endif

	return 0.0f;
}

Point3  PxFunctions::getMassSpaceInertiaTensor(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return Point3(0,0,0);
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return Point3(0,0,0);
	return (Point3&) actor->getMassSpaceInertiaTensor();

#endif
	return Point3(0,0,0);
}

Matrix3 PxFunctions::getCMassLocalPose(INode *node)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return Matrix3(true);
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return Matrix3(true);	
	return MxMathUtils::NxMatrixToMax(actor->getCMassLocalPose());
#endif

	return Matrix3(true);
}

int PxFunctions::updateMassFromShapes(INode *node,float density,float totalMass)
{
#if 0
	MxActor* mxActor = MxUtils::GetActorFromNode(node);
	if (mxActor == NULL) return 0;
	NxActor* actor = mxActor->getNxActor();
	if (actor == NULL) return 0;
	actor->updateMassFromShapes(density,totalMass);
#endif

	return 1;
}

int PxFunctions::pxnxuexport(char *fname, char *ext)
{

	int ret = 0;

	if (!gDynamicsWorld)
	{
		MaxMsgBox(NULL, _T("No dynamics data to export"), _T("Error"), MB_OK);
		return 0;
	}

	if ( stricmp(ext,"bullet") == 0 )
	{
		//create a large enough buffer. There is no method to pre-calculate the buffer size yet.
		
			//todo: copy names into rigid bodies, using serializer->registerNameForPointer(ptr,name);

			int maxSerializeBufferSize = 1024*1024*5;
		 
			btDefaultSerializer*	serializer = new btDefaultSerializer(maxSerializeBufferSize);
			gDynamicsWorld->serialize(serializer);
		 
			FILE* file = fopen(fname,"wb");
			fwrite(serializer->getBufferPointer(),serializer->getCurrentBufferSize(),1, file);
			fclose(file);
			delete serializer;

			ret = 1;
	}  else
	{
		//todo	
		//	if ( stricmp(ext,"dae") == 0 )
		MaxMsgBox(NULL, _T("Unsupported extension"), _T("Error"), MB_OK);
	}



	return ret;
}


int PxFunctions::debugprint()
{
#if 0
	if (gPluginData != NULL)
	{
		gPluginData->debugPrint();
	} else
	{
		if (gCurrentstream) gCurrentstream->printf("No plugin object available yet\n");
	}
#endif

	return 0;
}

int PxFunctions::getFluidParticleCount(INode* node)
{
#if 0
	MxFluid* mxFluid = (MxFluid*) MxUtils::GetFirstObjectOfTypeFromNode(node, MX_OBJECTTYPE_FLUID);
	if (mxFluid != NULL) {
		return mxFluid->getNumParticles();
	}
#endif
	return 0;
}

int PxFunctions::getFluidUpdateNr(INode* node)
{
#if 0
	MxFluid* mxFluid = (MxFluid*) MxUtils::GetFirstObjectOfTypeFromNode(node, MX_OBJECTTYPE_FLUID);
	if (mxFluid != NULL) {
		return mxFluid->getUpdateNr();
	}
#endif

	return 0;	
}

Tab<float> PxFunctions::getFluidParticles(INode* node)
{
	Tab<float> result;
	result.Resize(0);
#if 0
	MxFluid* mxFluid = (MxFluid*) MxUtils::GetFirstObjectOfTypeFromNode(node, MX_OBJECTTYPE_FLUID);
	if (mxFluid != NULL) {
		//return mxFluid->getNumParticles();
		mxFluid->getParticles(result);
	}
#endif

	return result;
}

Mesh* PxFunctions::pxCreateFluidMesh(INode* node, Mesh* currentMesh)
{
#if 0
	MxFluid* mxFluid = (MxFluid*) MxUtils::GetFirstObjectOfTypeFromNode(node, MX_OBJECTTYPE_FLUID);
	if (mxFluid != NULL) {
		return mxFluid->createMesh(currentMesh);
	}
#endif

	return NULL;
}

Mesh* PxFunctions::pxCreateHull(Mesh* src, int vertLimit, float inflation)
{
#if 0
	return MxUtils::pxCreateConvexHull(src, vertLimit, inflation);
#endif
	return 0;

}

Mesh* PxFunctions::createConvexFromPoints(Tab<float>& points, int vertLimit, float inflation)
{
#if 0
	PxSimpleMesh sm;
	sm.alloc(points.Count()/3, 0);
	for(int i = 0; i < sm.numPoints; ++i)
		sm.points[i].set(points[3*i],points[3*i + 1], points[3*i + 2]);
	PxSimpleMesh ret;
	MxUtils::pxCreateConvexHull(ret, sm, vertLimit, inflation);
	Mesh* mesh = MxUtils::pxSimpleMeshToNode(ret);
	ret.release();
	sm.release();
	return mesh;
#endif
	return 0;
}

INode* PxFunctions::pxCreateINode(Object* obj)
{
	if (obj == NULL) return NULL;

	INode* newNode = GetCOREInterface()->CreateObjectNode(obj);
	if (newNode != NULL) {
		Matrix3 matrix;
		matrix.IdentityMatrix();
		newNode->SetNodeTM(0, matrix);
		return newNode;
	}

	return NULL;
}

Mesh* PxFunctions::pxCreateSoftBodyMesh(INode* node, BOOL editableMesh, int subDivision, float simplificationFactor, BOOL createIsoSurface, BOOL singleIsoSurface)
{
	return 0;//MxSoftBody::createSoftBodyMesh(node, editableMesh == TRUE, subDivision, simplificationFactor, createIsoSurface == TRUE, singleIsoSurface == TRUE);
}

int PxFunctions::pxAddForceField(INode* node)
{
	if (node == NULL) return NULL;
#if 0
	MxUtils::PreparePlugin();
	MxPluginData::removeOldObjects(node, node->GetName());

	if(gCurrentstream) gCurrentstream->printf("Called pxAddForceField with node \"%s\".\n", node->GetName());
	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState(t); 
	if (MxUtils::checkNodeValidity(node, os) < 1) {
		if (gCurrentstream) gCurrentstream->printf("Unable to add \"%s\" as a ForceField.\n", node->GetName());
		return 0;
	}

	MxForceField* mx = MxPluginData::createForceField(node);
	return (mx != NULL)? mx->getID():0;
#endif
	return 0;

}

int PxFunctions::pxAddSoftBody(INode* node)
{
#ifdef PXPLUGIN_SOFTBODY
	if (node == NULL) return NULL;

	MxUtils::PreparePlugin();
	MxPluginData::removeOldObjects(node, node->GetName());

	if(gCurrentstream) gCurrentstream->printf("Called pxAddSoftBody with node \"%s\".\n", node->GetName());
	TimeValue t = GetCOREInterface()->GetTime();
	ObjectState os = node->EvalWorldState(t); 
	if (MxUtils::checkNodeValidity(node, os) < 1) {
		if (gCurrentstream) gCurrentstream->printf("Unable to add \"%s\" as a SoftBody.\n", node->GetName());
		return 0;
	}

	MxSoftBody* mxSoftBody = MxPluginData::createSoftBody(node);
	return (mxSoftBody != NULL)?mxSoftBody->getID():0;

#else
	return 0;
#endif //PXPLUGIN_SOFTBODY
}

bool PxFunctions::getPoint3RootSetting(TSTR key, Point3& dest)
{
	INode* rootNode = GetCOREInterface()->GetRootNode();
	if (rootNode != NULL) {
		TSTR value("");
		BOOL result = rootNode->GetUserPropString(key, value);
		if (result) {
			return getPoint3FromString(value, dest);
		}
	}
	dest = Point3(0, 0, 0);
	return false;
}

bool PxFunctions::setPoint3RootSetting(TSTR key, Point3 value)
{
	INode* rootNode = GetCOREInterface()->GetRootNode();
	if (rootNode != NULL) {
		TSTR valueStr("");
		valueStr.printf("[%f|%f|%f]", value.x, value.y, value.z);
		rootNode->SetUserPropString(key, valueStr);
		return true;
	}
	return false;
}

bool PxFunctions::getFloatRootSetting(TSTR key, float& dest)
{
	INode* rootNode = GetCOREInterface()->GetRootNode();
	if (rootNode != NULL) {
		TSTR value("");
		BOOL result = rootNode->GetUserPropString(key, value);
		if (result)
			return getFloatFromString(value, dest);
	}
	dest = 0.0f;
	return false;
}

bool PxFunctions::setFloatRootSetting(TSTR key, float value)
{
	INode* rootNode = GetCOREInterface()->GetRootNode();
	if (rootNode != NULL) {
//		TSTR value("");
//		value.printf("%f", value);
		rootNode->SetUserPropFloat(key, value);
		return true;
	}
	return false;
}

bool PxFunctions::getFloatFromString(TSTR value, float& dest)
{
	//* -- old code: it fails on my PC
	if (!value.isNull() && value.length() > 0) {
		char* tempStr = (char*)alloca(value.length() + 1);
		assert(tempStr != NULL);
		memcpy(tempStr, value.data(), value.length()+1);
		for (int i = 0; i < value.length(); i++) {
			if (value[i] == ',')
				tempStr[i] = '.';
		}
		dest = (float)atof(tempStr);
		return true;
	}
	dest = 0.0f;
	
	/* another simple solution
	if (!value.isNull() && value.length() > 0) {
		dest = (float) atof(value.data());
		return true;
	}
	*/
	return false;
}

bool PxFunctions::getPoint3FromString(TSTR value, Point3& dest)
{
	if (!value.isNull() && value.length() > 0) {
		char* tempStr = (char*)alloca(value.length() + 1);
		if (tempStr != NULL) {
			memcpy(tempStr, value.data(), value.length()+1);

			const char* v[3];
			v[0] = v[1] = v[2] = NULL;
			int pos = 0;
			int nr = 0;
			int currentLength = 0;
			while (nr < 3 && pos < value.length())
			{
				if (value[pos] == '|' || value[pos] == ' ' || value[pos] == '[' || value[pos] == ']') {
					if (currentLength > 0) {
						nr++;
					}
					tempStr[pos] = '\0';
					pos++;
					currentLength = 0;
					continue;
				}

				if (value[pos] == ',')
					tempStr[pos] = '.';

				if (currentLength == 0)
					v[nr] = &(tempStr[pos]);

				currentLength++;
				pos++;
			}

			if (v[0] != NULL && v[1] != NULL && v[2] != NULL) {
				double x = atof(v[0]);
				double y = atof(v[1]);
				double z = atof(v[2]);
				dest = Point3(x, y, z);
				return true;
			}
		}
	}
	dest = Point3(0, 0, 0);
	return false;
}

Point3 PxFunctions::getGravity()
{
	INode* rootNode = GetCOREInterface()->GetRootNode();
	if (rootNode != NULL) {
		TSTR value("");
		BOOL result = rootNode->GetUserPropString(TSTR("px_gravity"), value);
		if (result) {
			Point3 temp(0, 0, 0);
			if (getPoint3FromString(value, temp))
				return temp;
		}
	}

	//If we reach this point, then there is no gravity setting in the root node, store the current or default value there
	//setGravity(mSetting_gravity);
	return mSetting_gravity;
}

int PxFunctions::setGravity(Point3&	gravity)
{
	mSetting_gravity = gravity;
	bool result = setPoint3RootSetting(TSTR("px_gravity"), gravity);
	return result?1:0;
}

TSTR getParamTypeName(ParamType2 type) {
	switch(type) {
		case TYPE_FLOAT: {
			return TSTR(_T("float"));
			break;
						 }
		case TYPE_INT: {
			return TSTR(_T("int"));
			break;
					   }
		case TYPE_POINT3: {
			return TSTR(_T("Point3"));
			break;
						  }
		case TYPE_BOOL: {
			return TSTR(_T("bool"));
			break;
						}
		case TYPE_STRING: {
			return TSTR(_T("string"));
			break;
						  }
		case TYPE_FILENAME: {
			return TSTR(_T("filename"));
			break;
							}
		case TYPE_INODE: {
			return TSTR(_T("INode*"));
			break;
						 }
		case TYPE_REFTARG: {
			return TSTR(_T("ReferenceTarget"));
			break;
						   }
		case TYPE_VOID: {
			return TSTR(_T("void"));
			break;
						}
		case TYPE_INTERVAL: {
			return TSTR(_T("Interval"));
			break;
							}
		case TYPE_OBJECT: {
			return TSTR(_T("Object"));
			break;
						  }
		case TYPE_TSTR: {
			return TSTR(_T("TSTR"));
			break;
						}
		case TYPE_IOBJECT: {
			return TSTR(_T("IObject*"));
			break;
						   }
		case TYPE_FPVALUE: {
			return TSTR(_T("FPValue"));
			break;
						   }
		case TYPE_VALUE: {
			return TSTR(_T("Value"));
			break;
						 }
		case TYPE_DWORD: {
			return TSTR(_T("DWORD"));
			break;
						 }
		case TYPE_INODE_TAB: {
			return TSTR(_T("INode_TAB"));
			break;
							 }
		case TYPE_FLOAT_BR: {
			return TSTR(_T("float_br"));
			break;
							}				
		case TYPE_POINT3_BR: {
			return TSTR(_T("Point3_BR"));
			break;
							 }				
		case TYPE_POINT3_BV: {
			return TSTR(_T("Point3"));
			break;
							 }				
		case TYPE_TSTR_BV: {
			return TSTR(_T("String"));
			break;
						   }				
		case TYPE_MATRIX3_BV: {
			return TSTR(_T("Matrix3"));
							  }
		case TYPE_FPVALUE_BV: {
			return TSTR(_T("FPValue_BV"));
			break;
							  }
		case TYPE_POINT4_BR: {
			return TSTR(_T("Point4_BR"));
			break;
							  }
		case TYPE_MATRIX3_BR: {
			return TSTR(_T("Matrix3_BR"));
			break;
							 }
		case TYPE_FLOAT_TAB_BV: {
			return TSTR(_T("Tab<float>"));
			break;
							 }
		case TYPE_MESH: {
			return TSTR(_T("Mesh"));
			break;
							 }
		default:
			return TSTR(_T(""));

	};
}

void PxFunctions::pxPrintInterfaceDescription() {
	if (gCurrentstream == NULL)
		return;

	TSTR objname("PX");
	if (objname.length() > 0) {
		gCurrentstream->printf("Functions for %s:\n", objname.data());
	}

	FPInterfaceDesc* interfaceDesc = &PxInterfaceDescription;

	Tab<FPFunctionDef*>& functions = interfaceDesc->functions;
	for (int i = 0; i < functions.Count(); i++) {
		FPFunctionDef* def = functions[i];
		gCurrentstream->printf("%s %s(", getParamTypeName(def->result_type).data(), def->internal_name.data());

		Tab<FPParamDef*>& params = def->params;
		for (int j = 0; j < params.Count(); j++) {
			FPParamDef* p = params[j];
			gCurrentstream->printf("%s %s", getParamTypeName(p->type).data(), p->internal_name.data());
			if (j < (params.Count() - 1)) {
				gCurrentstream->printf(", ");
			}
		}
		gCurrentstream->printf(")\n");
	}

	Tab<FPPropDef*>& props = interfaceDesc->props;
	if (props.Count() > 0) {
		gCurrentstream->printf("\nProperties:\n");

		for (int i = 0; i < props.Count(); i++) {
			FPPropDef* prop = props[i];
			gCurrentstream->printf("\t.%s : %s\n", prop->internal_name.data(), getParamTypeName(prop->prop_type).data());
		}
	}
	gCurrentstream->printf("\n");
}

int     PxFunctions::getNumContacts()
{
	return MxContactReport::getMyContactReport().getNumContacts();
}

bool    PxFunctions::getNextContact()
{
	return MxContactReport::getMyContactReport().getNextContact();
}

Point3  PxFunctions::getContactForce()
{
	return MxContactReport::getMyContactReport().getContactForce();
}

Point3  PxFunctions::getContactPoint()
{
	return MxContactReport::getMyContactReport().getContactPoint();
}

INode*  PxFunctions::getContactNode0()
{
	return MxContactReport::getMyContactReport().getContactNode0();
}

INode*  PxFunctions::getContactNode1()
{
	return MxContactReport::getMyContactReport().getContactNode1();
}

float PxFunctions::setContactFilter(float force)
{
	return MxContactReport::getMyContactReport().setFilter(force);
}

float PxFunctions::getContactFilter()
{
	return MxContactReport::getMyContactReport().getFilter();
}

/*
	interfaces for user to get all sleeping actors
*/
std::vector<INode*> PxFunctions::mSleepingActors;

int PxFunctions::findSleepingActors()
{
#if 0
	mSleepingActors.clear();
	NxScene* scene = MxPluginData::getSceneStatic();
	if(NULL == scene) return 0;
	NxU32 count = scene->getNbActors();
	NxActor** a = scene->getActors();
	for(NxU32 i = 0; i < count; ++i)
	{
		NxActor* pa = a[i];
		if(pa->isSleeping() && pa->isDynamic() && !pa->readBodyFlag(NX_BF_KINEMATIC))
		{
			MxActor* a = (MxActor*) pa->userData;
			INode* node = a->getNode();
			if(NULL != node)
			{
				mSleepingActors.push_back(node);
			}
		}
	}
	return mSleepingActors.size();
#endif
	return 0;

}

INode* PxFunctions::getSleepingActor(int index)
{
	if(index >= 0 && index < mSleepingActors.size())
	{
		return mSleepingActors[index];
	}
	return NULL;
}

/*
	it is used to make Kinematic RB to mimic static RB with driven by animation.
*/
bool PxFunctions::m_mimicStaticRB = false;

bool PxFunctions::mimicStaticRB(bool flag)
{
	return m_mimicStaticRB = flag;
}


int PxFunctions::getVersion()
{
	return NX_SDK_VERSION_NUMBER;
}


bool PxFunctions::hwAvailable()
{
	MxUtils::PreparePlugin();
	return MxPluginData::HwAvailable();
}

float PxFunctions::setUnitChange(float maxToPhysics)
{
	pxremoveall();
	ccMaxWorld::SetUnitChange(maxToPhysics);
	return maxToPhysics;
}

float PxFunctions::getUnitChange()
{
	return ccMaxWorld::GetUnitChange();
}

int PxFunctions::PrintTM(INode *node)
{

	const TimeValue  t  = ccMaxWorld::MaxTime();
	Matrix3 nodeTM = node->GetNodeTM(t);
	Matrix3 parentTM = node->GetParentTM(t);
	Matrix3 objectTM = node->GetObjectTM(t);

	Matrix3 pivotTM(1);
	Point3 pvTrans = node->GetObjOffsetPos();
	pivotTM.SetTrans(pvTrans);
	Quat   pvRot  = node->GetObjOffsetRot();
	PreRotateMatrix(pivotTM, pvRot);
	Matrix3 newPV = pivotTM;
	ScaleValue pvScale = node->GetObjOffsetScale();
	ApplyScaling(pivotTM, pvScale);

	gCurrentstream->printf("PivotTM = "); 	MxUtils::PrintMatrix3(pivotTM); 
	gCurrentstream->printf("\nnodeTM = ");	MxUtils::PrintMatrix3(nodeTM); 
	gCurrentstream->printf("\nparentTM = ");	MxUtils::PrintMatrix3(parentTM); 
	gCurrentstream->printf("\nobjectTM = ");	MxUtils::PrintMatrix3(objectTM); 
	gCurrentstream->printf("\n");
	Point3 pv = pivotTM.GetRow(3);
	gCurrentstream->printf("pivot = [%f, %f, %f]\n", pv.x, pv.y, pv.z);

	ccMaxNode* ccNode = ccMaxWorld::FindNode(node);
	if(ccNode) {
		gCurrentstream->printf("PhysicsPoseTM = "); 	MxUtils::PrintMatrix3(ccNode->PhysicsNodePoseTM); 
		pv = ccNode->NodePosInPhysics;
		gCurrentstream->printf("\nNode Pos in Physics = [%f, %f, %f]\n", pv.x, pv.y, pv.z);
	}
	return 0;
}

int PxFunctions::setPivotScale(INode *node, Point3& scale)
{
	//node->SetObjOffsetPos(pv.GetTrans()); 
	//node->SetObjOffsetRot(IdentQuat()); 
	node->SetObjOffsetScale(ScaleValue(scale)); 
	return 1;
}

