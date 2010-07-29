#ifndef PX_PXFUNCTIONS_H
#define PX_PXFUNCTIONS_H

#include "PxPlugin.h"
#include "MxContactReport.h"
#include <vector>

enum PXFunctionEnum { 
	em_setstream,
	em_pxadd,
	em_pxcreateD6JointDesc,
	em_pxaddD6Joint,
	em_pxsetD6JointSwing,
	em_pxsetD6JointTwist,
	em_pxsetD6JointLinear,
	em_pxsetD6JointLocalAxis,
	em_pxsetD6JointBreakable,
	em_pxsetD6JointProjection,
	em_pxsetD6JointCollision,
	em_pxcalcD6jointfromIK,
	em_pxsetD6JointGear,
	em_pxaddjoint,
	em_pxaddcloth,
	em_pxaddfluid,
	em_pxaddfluidemitter,
	em_pxsetshapeflag,
	em_pxremove,
	em_pxremoveall,
	em_pxsim,
	em_setsubsimsteps,
	em_pxnxuexport,
	em_pxsync,
	em_pxrestart,	//not "restart" as in "restart SDK", rather "reset simulation"
	em_pxvisualizephysics,
	em_pxprep,
	em_pxsnap,
	em_setdynamic,
	em_isdynamic,
	em_isactor,
	em_isjoint,
	em_updateInitialVelocityAndSpin,
	em_getLinearVelocity,
	em_setLinearVelocity,
	em_getAngularVelocity,
	em_setAngularVelocity,
	em_getGlobalPosition,
	em_setGlobalPosition,
	em_getGlobalPose,
	em_setGlobalPose,
	em_getMass,
	em_getMassSpaceInertiaTensor,
	em_getCMassLocalPose,
	em_updateMassFromShapes,
	em_setjointlinearlimits ,
	em_setjointangularlimits,
	em_setjointslerpdrive,
	em_setjointtwistswingdrive,
	em_setjointdriver,
	em_convexdecomposition,
	em_debugprint,
	em_getfluidupdatenr,
	em_getfluidparticlecount,
	em_getfluidparticles,
	em_setsdkparameter,
	em_getsdkparameter,
	em_resetsdkparameter,
	em_createconvexhull,
	em_createconvexfrompoints,
	em_createinode,
	em_createsoftbodymesh,
	em_addsoftbody,
	em_addforcefield,
	em_getgravity,
	em_setgravity,
	em_getfluidmesh,
	em_describe,
	em_getnumcontacts,
	em_getnextcontact,
	em_getcontactforce,
	em_getcontactpoint,
	em_getcontactnode0,
	em_getcontactnode1,
	em_setcontactfilter,
	em_getcontactfilter,
	em_findsleepingactors,
	em_getsleepingactor,
	em_mimicstaticrb,
	em_getversion,
	em_hwavailable,
	em_getdynamicfriction,
	em_getstaticfriction,
	em_getrestitution,
	em_setdynamicfriction,
	em_setstaticfriction,
	em_setrestitution,
	em_getfps,
	em_printfps,
	em_printtm,
	em_setunitchange,
	em_getunitchange,
	em_setPivotScale,

	NXENUMS(solveriterationcount)
	NXENUMS(kinematicwriteback)
	NXENUMS(exploitinstances)
	NXENUMS(debugphysics)
	NXENUMS(useHardware)
	NXENUMS(savedefaults)

	// ****** Add enum function definition thing here (1st of 4 places)

	em_num_enums 
};

class PxFunctions : public FPStaticInterface
{
public:
	DECLARE_DESCRIPTOR(PxFunctions)
	BEGIN_FUNCTION_MAP
		FN_1(em_setstream     ,TYPE_INT       ,pxSetOutputStream,     TYPE_VALUE)
		FN_1(em_convexdecomposition ,TYPE_INT ,pxConvexDecomposition,          TYPE_OBJECT)
		FN_1(em_pxadd         ,TYPE_INT       ,pxadd   ,      TYPE_INODE)
		FN_6(em_pxaddjoint    ,TYPE_INT       ,pxaddjoint,    TYPE_INODE, TYPE_POINT3_BR, TYPE_POINT3_BR,  TYPE_POINT4_BR, TYPE_POINT3_BR, TYPE_POINT3_BR)
		FN_1(em_pxaddD6Joint  ,TYPE_INT       ,pxaddD6Joint,  TYPE_INT)
		FN_1(em_pxcreateD6JointDesc  ,TYPE_INT    ,pxcreateD6JointDesc,  TYPE_INODE)
		FN_5(em_pxsetD6JointSwing, TYPE_INT	  ,pxsetD6JointSwing, TYPE_INT, TYPE_INT, TYPE_BOOL, TYPE_BOOL, TYPE_POINT4_BR)
		FN_5(em_pxsetD6JointTwist, TYPE_INT	  ,pxsetD6JointTwist, TYPE_INT, TYPE_BOOL, TYPE_FLOAT, TYPE_FLOAT, TYPE_POINT3_BR)
		FN_5(em_pxsetD6JointLinear, TYPE_INT  ,pxsetD6JointLinear, TYPE_INT, TYPE_INT, TYPE_INT, TYPE_INT, TYPE_FLOAT)
		FN_5(em_pxsetD6JointLocalAxis, TYPE_INT, pxsetD6JointLocalAxis, TYPE_INT, TYPE_INT, TYPE_POINT3_BR, TYPE_POINT3_BR, TYPE_POINT3_BR)
		FN_4(em_pxsetD6JointBreakable, TYPE_INT, pxsetD6JointBreakable, TYPE_INT, TYPE_BOOL, TYPE_FLOAT, TYPE_FLOAT)
		FN_4(em_pxsetD6JointProjection, TYPE_INT, pxsetD6JointProjection, TYPE_INT, TYPE_INT, TYPE_FLOAT, TYPE_FLOAT)
		FN_2(em_pxsetD6JointCollision, TYPE_INT, pxsetD6JointCollision, TYPE_INT, TYPE_BOOL)
		FN_3(em_pxsetD6JointGear, TYPE_INT	  ,pxsetD6JointGear, TYPE_INT, TYPE_BOOL, TYPE_FLOAT)
		FN_2(em_pxaddcloth    ,TYPE_INT       ,pxaddcloth,    TYPE_INODE, TYPE_BOOL)
		FN_1(em_pxaddfluid    ,TYPE_INT       ,pxaddfluid,    TYPE_INODE)
		FN_1(em_pxaddfluidemitter    ,TYPE_INT       ,pxaddfluidemitter,    TYPE_INODE)
		FN_3(em_pxsetshapeflag,TYPE_INT       ,pxsetshapeflag,TYPE_INODE, TYPE_TSTR_BV, TYPE_BOOL)
		FN_1(em_pxremove      ,TYPE_INT       ,pxremove,      TYPE_INODE)
		FN_0(em_pxremoveall      ,TYPE_INT       ,pxremoveall)
		FN_1(em_pxcalcD6jointfromIK    ,TYPE_INT       ,pxcalcD6jointfromIK,    TYPE_INODE)
		FN_2(em_setdynamic    ,TYPE_INT       ,setdynamic,    TYPE_INODE,TYPE_INT)
		FN_1(em_isdynamic     ,TYPE_INT       ,isdynamic,     TYPE_INODE)
		FN_1(em_isactor       ,TYPE_INT       ,isactor  ,     TYPE_INODE)
		FN_1(em_isjoint       ,TYPE_INT       ,isjoint  ,     TYPE_INODE)
		FN_1(em_pxsim         ,TYPE_INT       ,pxsim    ,     TYPE_FLOAT)
		FN_1(em_setsubsimsteps         ,TYPE_INT       , setSubSimSteps   ,     TYPE_INT)
		
		FN_2(em_pxnxuexport   ,TYPE_INT       ,pxnxuexport,   TYPE_STRING, TYPE_STRING)
		FN_0(em_pxsync        ,TYPE_INT       ,pxsync        )
		FN_0(em_pxrestart     ,TYPE_INT       ,pxrestart     )
		FN_1(em_pxvisualizephysics,TYPE_INT      ,pxvisualizephysics, TYPE_BOOL)
		FN_0(em_pxprep        ,TYPE_INT       ,pxprep        )
		FN_1(em_pxsnap        ,TYPE_INT       ,pxsnap  ,      TYPE_FLOAT)
		FN_1(em_updateInitialVelocityAndSpin, TYPE_INT	  , updateInitialVelocityAndSpin ,TYPE_INODE);
		FN_1(em_getLinearVelocity         ,TYPE_POINT3_BV , getLinearVelocity         ,TYPE_INODE)
		FN_2(em_setLinearVelocity         ,TYPE_INT	      , setLinearVelocity         ,TYPE_INODE, TYPE_POINT3_BR)
		FN_1(em_getAngularVelocity        ,TYPE_POINT3_BV , getAngularVelocity        ,TYPE_INODE)
		FN_2(em_setAngularVelocity        ,TYPE_INT       , setAngularVelocity        ,TYPE_INODE, TYPE_POINT3_BR)
		FN_1(em_getGlobalPosition         ,TYPE_POINT3_BV,  getGlobalPosition         ,TYPE_INODE)
		FN_2(em_setGlobalPosition         ,TYPE_INT	      , setGlobalPosition         ,TYPE_INODE, TYPE_POINT3_BR)
		FN_1(em_getGlobalPose             ,TYPE_MATRIX3_BV, getGlobalPose             ,TYPE_INODE)
		FN_2(em_setGlobalPose             ,TYPE_INT	      , setGlobalPose             ,TYPE_INODE, TYPE_MATRIX3_BR)
		FN_1(em_getMass                   ,TYPE_FLOAT     , getMass                   ,TYPE_INODE)
		FN_1(em_getMassSpaceInertiaTensor ,TYPE_POINT3_BV , getMassSpaceInertiaTensor ,TYPE_INODE)
		FN_1(em_getCMassLocalPose         ,TYPE_MATRIX3_BV, getCMassLocalPose         ,TYPE_INODE)
		FN_3(em_updateMassFromShapes      ,TYPE_INT       , updateMassFromShapes      ,TYPE_INODE, TYPE_FLOAT,TYPE_FLOAT)
		FN_3(em_setjointlinearlimits      ,TYPE_INT       , setjointlinearlimits      ,TYPE_INODE, TYPE_POINT3_BR, TYPE_POINT3_BR)
		FN_3(em_setjointangularlimits     ,TYPE_INT       , setjointangularlimits     ,TYPE_INODE, TYPE_POINT3_BR, TYPE_POINT3_BR)
		FN_3(em_setjointslerpdrive        ,TYPE_INT       , setjointslerpdrive        ,TYPE_INODE, TYPE_FLOAT,TYPE_FLOAT)
		FN_4(em_setjointtwistswingdrive   ,TYPE_INT       , setjointtwistswingdrive   ,TYPE_INODE, TYPE_FLOAT,TYPE_FLOAT,TYPE_FLOAT)
		FN_3(em_setjointdriver            ,TYPE_INT       , setjointdriver            ,TYPE_INODE, TYPE_INODE,TYPE_INT)

		FN_1(em_getdynamicfriction        ,TYPE_FLOAT     , getDynamicFriction               ,TYPE_INODE)
		FN_1(em_getstaticfriction         ,TYPE_FLOAT     , getStaticFriction                ,TYPE_INODE)
		FN_1(em_getrestitution            ,TYPE_FLOAT     , getRestitution                   ,TYPE_INODE)

		FN_2(em_setdynamicfriction        ,TYPE_INT	      , setDynamicFriction            ,TYPE_INODE, TYPE_FLOAT)
		FN_2(em_setstaticfriction         ,TYPE_INT	      , setStaticFriction             ,TYPE_INODE, TYPE_FLOAT)
		FN_2(em_setrestitution            ,TYPE_INT	      , setRestitution                ,TYPE_INODE, TYPE_FLOAT)
		FN_0(em_getfps                    ,TYPE_FLOAT     , getFPS  )
		FN_1(em_printfps                    ,TYPE_BOOL     , printFPS  , TYPE_BOOL)

		FN_1(em_printtm             , TYPE_INT, PrintTM             ,TYPE_INODE)
		FN_1(em_setunitchange       , TYPE_FLOAT, setUnitChange     ,TYPE_FLOAT)
		FN_0(em_getunitchange       , TYPE_FLOAT, getUnitChange)

		NXFMAP(BOOL  ,debugphysics)
		NXFMAP(BOOL  ,useHardware)
		NXFMAP(INT   ,solveriterationcount)
		NXFMAP(INT   ,kinematicwriteback)
		NXFMAP(INT   ,exploitinstances)
		NXFMAP(BOOL   ,savedefaults)

		FN_0(em_getgravity, TYPE_POINT3_BV, getGravity)
		FN_1(em_setgravity, TYPE_INT, setGravity, TYPE_POINT3_BV)

		FN_0(em_debugprint      ,TYPE_INT       ,debugprint)
		VFN_0(em_describe						,pxPrintInterfaceDescription);
		FN_1(em_getfluidupdatenr, TYPE_INT, getFluidUpdateNr, TYPE_INODE)
		FN_1(em_getfluidparticlecount, TYPE_INT, getFluidParticleCount, TYPE_INODE)
		FN_1(em_getfluidparticles, TYPE_FLOAT_TAB_BV, getFluidParticles, TYPE_INODE)
		FN_2(em_setsdkparameter, TYPE_INT, setSDKParameter, TYPE_TSTR_BV, TYPE_FLOAT)
		FN_1(em_getsdkparameter, TYPE_FLOAT, getSDKParameter, TYPE_TSTR_BV)
		FN_1(em_resetsdkparameter, TYPE_INT, resetSDKParameter, TYPE_TSTR_BV)
		FN_3(em_createconvexhull, TYPE_MESH, pxCreateHull, TYPE_MESH, TYPE_INT, TYPE_FLOAT)
		FN_3(em_createconvexfrompoints, TYPE_MESH, createConvexFromPoints, TYPE_FLOAT_TAB_BR, TYPE_INT, TYPE_FLOAT)
		FN_1(em_createinode, TYPE_INODE, pxCreateINode, TYPE_OBJECT)
		FN_2(em_getfluidmesh, TYPE_MESH, pxCreateFluidMesh, TYPE_INODE, TYPE_MESH)

		FN_6(em_createsoftbodymesh, TYPE_MESH, pxCreateSoftBodyMesh, TYPE_INODE, TYPE_BOOL, TYPE_INT, TYPE_FLOAT, TYPE_BOOL, TYPE_BOOL)
		FN_1(em_addsoftbody, TYPE_INT, pxAddSoftBody, TYPE_INODE)
		FN_1(em_addforcefield, TYPE_INT, pxAddForceField, TYPE_INODE)

		FN_0(em_getnumcontacts, TYPE_INT, getNumContacts);
		FN_0(em_getnextcontact, TYPE_BOOL, getNextContact);
		FN_0(em_getcontactforce, TYPE_POINT3_BV, getContactForce);
		FN_0(em_getcontactpoint, TYPE_POINT3_BV, getContactPoint);
		FN_0(em_getcontactnode0, TYPE_INODE, getContactNode0);
		FN_0(em_getcontactnode1, TYPE_INODE, getContactNode1);
		FN_1(em_setcontactfilter, TYPE_FLOAT, setContactFilter, TYPE_FLOAT);
		FN_0(em_getcontactfilter, TYPE_FLOAT, getContactFilter);

		FN_0(em_findsleepingactors, TYPE_INT, findSleepingActors);
		FN_1(em_getsleepingactor, TYPE_INODE, getSleepingActor, TYPE_INT);
		FN_1(em_mimicstaticrb, TYPE_BOOL, mimicStaticRB, TYPE_BOOL);
		FN_0(em_getversion, TYPE_INT, getVersion);
		FN_0(em_hwavailable, TYPE_BOOL, hwAvailable);
		
		FN_2(em_setPivotScale         ,TYPE_INT	      , setPivotScale         ,TYPE_INODE, TYPE_POINT3_BR)
		// ****** Add macro function definition thing here (2nd of 4 places)

	END_FUNCTION_MAP

	// ****** Add function definition here (3rd of 4 places)
	static int		pxSetOutputStream(Value *val);
	static int		pxConvexDecomposition(Object *obj) { int ret = 0; return ret; }
	static int		pxadd(INode *node) ;
	static int		pxaddD6Joint(int joint);
	static int		pxcreateD6JointDesc(INode *node);
	static int		pxsetD6JointSwing(int jointDesc, int index, BOOL _limited, BOOL _locked, Point4 values);
	static int		pxsetD6JointTwist(int jointDesc, BOOL twistEnable, float twistLow, float twistHigh, Point3 values);
	static int		pxsetD6JointLinear(int jointDesc, int modeX, int modeY, int modeZ, float radius);
	static int		pxsetD6JointLocalAxis(int jointDesc, int index, Point3 axis, Point3 normal, Point3 anchor);
	static int		pxsetD6JointBreakable(int joint, BOOL breakable, float maxForce, float maxTorque);
	static int		pxsetD6JointProjection(int jointDesc, int mode, float dist, float angle);
	static int		pxsetD6JointCollision(int jointDesc, BOOL enabled);
	static int		pxsetD6JointGear(int jointDesc, BOOL enabled, float ratio);

	static int		pxaddjoint(INode *node, Point3 &axis, Point3 &normal, Point4 &limits, Point3 &translimitmin, Point3 &translimitmax);
	static int		pxaddcloth(INode *node, BOOL isMetalCloth);
	static int		pxaddfluid(INode *node);
	static int		pxaddfluidemitter(INode *node);
	static int		pxsetshapeflag(INode* node, TSTR flagname, BOOL value);
	static int		pxcalcD6jointfromIK(INode *node);
	static int		pxremove(INode *node);
	static int		pxremoveall();
	static int		setdynamic(INode *node, int _dynamic);
	static int		isdynamic(INode *node);
	static int		isactor(INode *node);
	static int		isjoint(INode *node);
	static int		pxsim(float deltat);
	static int		setSubSimSteps(int numSteps);
	static unsigned int subSimSubsteps;
	static int		pxnxuexport(char *fname, char *ext);
	static int		pxsync();
	static int		pxrestart();
	static int		pxvisualizephysics(BOOL enable);
	static int		pxprep();
	static int		pxsnap(float);

	static int		updateInitialVelocityAndSpin(INode *node);
	static Point3	getLinearVelocity (INode *node);
	static int		setLinearVelocity (INode *node,Point3 &linearvelocity);
	static Point3	getAngularVelocity(INode *node);
	static int		setAngularVelocity(INode *node, Point3 &angularvelocity);
	static Point3	getGlobalPosition (INode *node);
	static int	    setGlobalPosition (INode *node, Point3 &position);
	static Matrix3	getGlobalPose(INode *node);
	static int      setGlobalPose(INode *node, Matrix3& pose);

	static float      getDynamicFriction(INode *node);
	static float      getStaticFriction(INode *node);
	static float      getRestitution(INode *node);
	static int      setDynamicFriction(INode *node, float value);
	static int      setStaticFriction(INode *node, float value);
	static int      setRestitution(INode *node, float value);

	static float    getFPS();
	static bool     printFPS(bool onoff);
	static int      PrintTM(INode *node);
	static float    setUnitChange(float maxToPhysics);
	static float    getUnitChange();

	static float	getMass(INode *node);
	static Point3	getMassSpaceInertiaTensor(INode *node);
	static Matrix3	getCMassLocalPose(INode *node);
	static int		updateMassFromShapes(INode *node,float density,float totalMass);
	static int		setjointlinearlimits(INode *node,Point3 &translimitmin, Point3 &translimitmax);
	static int		setjointangularlimits(INode *node,Point3 &rotlimitmin, Point3 &rotlimitmax);
	static int		setjointslerpdrive(INode *node,float spring,float damping);
	static int		setjointtwistswingdrive(INode *node,float twistspring,float swingspring,float damping);
	static int		setjointdriver(INode *node,INode *driver,int fromstartframe);
	static void		updateskinwidth(NxReal skinWidth);

	static int		debugprint();
	static void		pxPrintInterfaceDescription();

	static int		getFluidUpdateNr(INode* node);
	static int		getFluidParticleCount(INode* node);
	static Tab<float>getFluidParticles(INode* node);
	static Mesh*	pxCreateFluidMesh(INode* node, Mesh* mesh);
	static int		setSDKParameter(TSTR paramname, float value);
	static float	getSDKParameter(TSTR paramname);
	static int		resetSDKParameter(TSTR paramname);

	static Mesh*	pxCreateHull(Mesh* src, int vertLimit, float inflation);
	static Mesh*	createConvexFromPoints(Tab<float>& points, int vertLimit, float inflation);
	static INode*	pxCreateINode(Object* obj);

	static Mesh*	pxCreateSoftBodyMesh(INode* node, BOOL editableMesh, int subDivision, float simplificationFactor, BOOL createIsoSurface, BOOL singleIsoSurface);
	static int		pxAddSoftBody(INode* node);

	static int		pxAddForceField(INode* node);

	static Point3	getGravity();
	static int		setGravity(Point3& gravity);

	static NXPARAM(BOOL,	debugphysics);
	static NXPARAM(BOOL,	useHardware);
	static NXPARAM(int,		solveriterationcount);
	static NXPARAM(int,		kinematicwriteback);
	static NXPARAM(int,		exploitinstances);
	static NXPARAM(BOOL,	savedefaults);
	
	static Point3 mSetting_gravity;

	static int     getNumContacts();
	static bool    getNextContact();
	static Point3  getContactForce();
	static Point3  getContactPoint();
	static INode*  getContactNode0();
	static INode*  getContactNode1();
	static float   setContactFilter(float force);
	static float   getContactFilter();

	static int findSleepingActors();
	static INode* getSleepingActor(int index);

	static bool mimicStaticRB(bool flag);
	static int getVersion();
	static bool hwAvailable();

	static int	    setPivotScale (INode *node, Point3 &scale);

protected:
	static bool getPoint3FromString(TSTR value, Point3& dest);
	static bool getFloatFromString(TSTR value, float& dest);
	static bool getPoint3RootSetting(TSTR key, Point3& dest);
	static bool setPoint3RootSetting(TSTR key, Point3 value);
	static bool getFloatRootSetting(TSTR key, float& dest);
	static bool setFloatRootSetting(TSTR key, float value);
	static std::vector<INode*> mSleepingActors;

	static 	bool m_mimicStaticRB;  // it is a flag for Kinematic RB to mimic Static RB. This will cause many warning raised by SDK.
};
#endif //PX_PXFUNCTIONS_H
