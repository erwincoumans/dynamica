//
// Main definition file for the PhysX plug-in for 3ds max
//
#ifndef PX_PXPLUGIN_H
#define PX_PXPLUGIN_H

//Conditional inclution of features

//Include legacy functions (currently not available)
//#define PXPLUGIN_LEGACY

//#include <NxPhysics.h>
#include "MxUtils.h"
//Include SoftBody support
//#if NX_SDK_VERSION_NUMBER >= 270
//#define PXPLUGIN_SOFTBODY
//#endif

#include "maxtypes.h"

//Interface- and ClassIDs for the plugin
#define PX_INTERFACE_ID Interface_ID(0x436755, 0x65524)
#define PX_CLASS_ID	 Class_ID(0x3736ad38, 0x276d8303)

#define PXLEGACY_INTERFACE_ID Interface_ID(0x5a1f7fe2, 0x6ee60220)
#define PXLEGACY_CLASS_ID Class_ID(0x1c101275, 0x5334115e)

#define NXPARAM(T,V) T mSetting_ ## V;  T get ## V(){return mSetting_ ## V;} ;  T set ## V(T _ ## V){mSetting_ ## V= _ ## V ; return mSetting_ ## V;} ; 
#define NXENUMS(V)  em_nxget ## V , em_nxset ## V ,
#define NXFMAP(T,V) FN_0(em_nxget ## V    ,TYPE_ ## T      , get ## V )   FN_1(em_nxset ## V    ,TYPE_ ## T      , set ## V , TYPE_ ## T   )
#define NXIFACE(T,V)  em_nxget ## V  , _T("get"  #V)  , 0, TYPE_ ## T, 0, 0    ,\
	em_nxset ## V  , _T("set"  #V)  , 0, TYPE_ ## T, 0, 1 , _T( #V ), 0 , TYPE_ ## T ,\

static const Class_ID PXFLUID_CLASSID(0xe2757b00, 0x91b46400);
static const Class_ID PXFLUIDEMITTER_CLASSID(0xe2757b00, 0x92b46400);
static Class_ID CAPS_CLASS_ID(0x6d3d77ac, 0x79c939a9);  // not found in any header file

#define CAPS_RADIUS		0	// These are the ParamBlock indexes copied from
#define CAPS_HEIGHT		1	// /3dsmax7/maxsdk/samples/howto/mxs_samples/testdlx/tester.cpp
#define CAPS_CENTERS	2	// and /3dsmax7/maxsdk/samples/objects/extendedobjects/scuba.cpp

class CharStream;
extern CharStream* gCurrentstream;

/**
	Class used internally for some automation regarding flags that can be set
	as user parameters on 3ds max objects.
*/
class FlagHolder
{
public:
	FlagHolder(const char* _name, NxU32 _flagValue)
	{
		this->name = _name;
		this->flagValue = _flagValue;
	}
public:
	const char* name;
	NxU32       flagValue;
};

class NxTetraInterface;
// loads the tetra maker DLL and returns the interface pointer.
NxTetraInterface* getNxTetraInterface();

/////////////////////////////////////////////////////////////////////
// support for CCD skeleton
enum PX_CCD_SKELETON
{
	NO_SKELETON,
	POINT_SKELETON,
	BOX_SKELETON,
	CONVEX_SKELETON
};

enum PxRBInteractivity
{
	RB_DYNAMIC    = 1,
	RB_KINEMATIC  = 2,
	RB_STATIC     = 3
};


#endif //PX_PXPLUGIN_H
