#ifndef MX_PLUGINDATA_H
#define MX_PLUGINDATA_H

#include "MxObjects.h"

class MxUserOutputStream;
enum MxMeshType;

class CharStream;

/**
	This class helps keeping track of objects in 3ds max. It needs to keep track of:

	PhysX SDK
	PhysX Scene
	PhysX Compartments

	Max object (geometry) <-> PhysX shape
	Max geometry <-> PhysX (trimesh/convex) geometry
	Max object <-> PhysX actor
	Max Joint object <-> PhysX Joint object
	Max Material <-> PhysX Material
	Max Fluid object <-> PhysX Fluid object
	Max Fluid emitter <-> PhysX Fluid emitter
	Fluid emitter <-> Fluid
	Max Cloth object <-> PhysX Cloth object
*/
class MxPluginData {
public:
	static MxObject* getObjectFromId(NxU32 id);
	static void getObjectsFromName(const char* name, NxArray<MxObject*>& destination);
	static void getObjectsFromNode(INode* node, NxArray<MxObject*>& destination);
	static void getAllObjects(NxArray<MxObject*>& destination);

	//removes old objects, returns number of removed objects
	static NxU32 removeOldObjects(INode* node, const char* name);

	//creates the rigid body, but no shapes (automatic shape creation (i.e. createShapes=true) is not implemented yet)
	static MxActor* createActor(INode* node, bool createShapes=false);
	//releases the rigid body (including the shapes), also releases objects referencing the actor (e.g. joints)
	static bool releaseActor(MxActor* actor, bool* sideEffects=NULL);

	//static MxShape* createShape(INode* node, NxShapeType type, bool needCCD);
	////also decreases the refcount on any meshes that the shape is holding on to
	//static bool releaseShape(MxShape* shape);

	static MxJoint* createJoint(INode* node);
	static MxJoint* createUninitializedJoint(INode* node); //for use with pxaddjoint
	static bool releaseJoint(MxJoint* joint);

	//if shape is supplied, it is also assigned the material
	//static MxMaterial* createMaterial(INode* node, MxShape* shape=NULL);
	//static bool releaseMaterial(MxMaterial* material, bool* sideEffects=NULL);

	static MxFluid* createFluid(INode* node);
	static bool releaseFluid(MxFluid* fluid, bool* sideEffects=NULL);

	static MxFluidEmitter* createFluidEmitter(INode* node);
	static bool releaseFluidEmitter(MxFluidEmitter* emitter);
	static void getAllEmitters(NxArray<MxFluidEmitter*>& destination);

	static MxCloth* createCloth(INode* node, bool isMetalCloth);
	static bool releaseCloth(MxCloth* cloth);

	static MxSoftBody* createSoftBody(INode* node);
	static bool releaseSoftBody(MxSoftBody* softbody);

	static MxForceField* createForceField(INode* node);
	static bool releaseForceField(MxForceField* forceField);

	static MxCompartment* createCompartment(NxCompartmentType type, NxU32 compartmentID);
	static bool releaseCompartment(MxCompartment* compartment);

	static bool releaseObject(MxObject* object, bool* sideEffects=NULL);

	static NxPhysicsSDK* getPhysicsSDKStatic();
	
	static bool HwAvailable() { return false;}//m_hwPhysX; }
	//static NxScene* getSceneStatic() { return m_instance?m_instance->m_scene:NULL; }
//	NxPhysicsSDK* getPhysicsSDK() { return 0;} //m_physicsSDK; }
//	NxScene* getScene() { return m_scene; }

	//also initializes the cooking library, no need to close it (it is done when MxPluginData is released)
	NxCookingInterface* getCookingInterface();

	void setOutputStream(CharStream* outputStream) { m_outputStream = outputStream; }
	MxUserOutputStream* getUserOutputStream() { return m_userOutputStream; }

	static MxPluginData* getInstance() { return m_instance; }

	void debugPrint();

	//creates a new mesh, or uses an already existing one and refcounts
	MxMesh* createMesh(INode* node, MxMeshType type);
	//decrease the refcount of the mesh, if it reaches zero, then release it
	bool releaseMesh(MxMesh* mesh);

private:
	static MxObject* getObjectFromNode(INode* node);

	friend class MxUtils;
	//Private, so that this can only be created by the nxmax object
	MxPluginData(CharStream* outputStream);
	virtual ~MxPluginData();
	//init SDK and create a scene, return true if the MxPluginData object was correctly initialized
	bool init();

	void getAllObjectsOfType(MxObjectType type, NxArray<MxObject*>& destination);

	MxUserOutputStream* m_userOutputStream;
	NxPhysicsSDK* m_physicsSDK;
	static bool m_hwPhysX;

	NxScene* m_scene;
	NxCookingInterface* m_cookingInterface;
	bool m_cookingInitialized;

	//not optimal to have all objects in the same array, should move it to different ones
	NxArray<MxObject*> m_objects;
	//NxArray<MxShape*> m_shapes;
	NxArray<MxObject*>& getObjectArray() { return m_objects; }
	bool removeObjectFromArray(MxObject* obj);

	//an array of "dead" compartments, which are not in use but have not been released yet
	//can be used instead of creating a new compartment (if any is of the right kind)
	NxArray<MxCompartment*> m_deadCompartments;
	NxArray<MxCompartment*>& getDeadCompartments() { return m_deadCompartments; }

	static MxPluginData* m_instance;
	static CharStream* m_outputStream;
};
extern MxPluginData* gPluginData;

struct MxSDKParam {
public:
	bool inUse;
	float value;
	float defaultValue;
};
extern MxSDKParam* gSDKParamSettings;

#endif //MX_PLUGINDATA_H