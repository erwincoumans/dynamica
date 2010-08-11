/* Copyright (c) 2008 NVIDIA CORPORATION

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

//For feedback and latest version see http://dynamica.googlecode.com

#ifndef MX_UTILS_H
#define MX_UTILS_H
 
#include <max.h>
#include <iparamb2.h>
#include "btBulletDynamicsCommon.h"
class TriObject;
class INode;
#include "maxtypes.h"

typedef btScalar NxF32;
typedef btScalar NxReal;
typedef btVector3 NxVec3;
typedef unsigned int NxU32;
#define NxArray btAlignedObjectArray

///bullet2Max and max2Bullet are not optimized!

static inline void bullet2Max(const btTransform& trans, Matrix3& outMatrix)
{
	Point3 pos(trans.getOrigin().getX(),trans.getOrigin().getY(),trans.getOrigin().getZ());
	outMatrix.SetRow(3,pos);

	Point3 row;

	row.x = trans.getBasis()[0].getX();
	row.y = trans.getBasis()[1].getX();
	row.z = trans.getBasis()[2].getX();
	outMatrix.SetRow(0,row);

	row.x = trans.getBasis()[0].getY();
	row.y = trans.getBasis()[1].getY();
	row.z = trans.getBasis()[2].getY();
	outMatrix.SetRow(1,row);

	row.x = trans.getBasis()[0].getZ();
	row.y = trans.getBasis()[1].getZ();
	row.z = trans.getBasis()[2].getZ();
	outMatrix.SetRow(2,row);

}

static inline void max2Bullet(const Matrix3& mat, btTransform& outTrans )
{
	//todo: check this
	btTransform trans;

	Point3& wp = mat.GetRow(3);
	btVector3 worldPos(wp.x,wp.y,wp.z);
	//btMatrix3x3 bm(
	//	mat.GetRow(0).x,mat.GetRow(0).y,	mat.GetRow(0).z,
	//	mat.GetRow(1).x,mat.GetRow(1).y,	mat.GetRow(1).z,
	//	mat.GetRow(2).x,mat.GetRow(2).y,	mat.GetRow(2).z);
	
	btMatrix3x3 bm(
		mat.GetRow(0).x,mat.GetRow(1).x,	mat.GetRow(2).x,
		mat.GetRow(0).y,mat.GetRow(1).y,	mat.GetRow(2).y,
		mat.GetRow(0).z,mat.GetRow(1).z,	mat.GetRow(2).z);
	
	outTrans.setOrigin(worldPos);
	outTrans.setBasis(bm);
}

enum NxCompartmentType
{
	NX_SCT_SOFTBODY = 1,
	NX_SCT_FLUID
};

#define NX_SDK_VERSION_NUMBER 277 //keep it low, to avoid enabling soft body, fluids etc

struct NxCompartment
{
};

struct NxPhysicsSDK
{
	void	setParameter(int paramType, btScalar value)
	{
	}
};



struct NxCookingInterface
{
};

struct NxJoint
{
	void* userData;
};

struct NxActor
{
	void	putToSleep() {}
	void* userData;
};

struct NxTriangleMeshDesc
{

};
struct NxTriangleMesh
{
};
struct NxConvexMesh
{
};
struct NxUserOutputStream
{

};

enum NxErrorCode
{
	NXE_DB_INFO = 1
};

struct	NxSwingLimit
{
	float value;
	float restitution;
	float spring;
	float damping;
};

enum SwingMotion
{
	NX_D6JOINT_MOTION_FREE = 1,
	NX_D6JOINT_MOTION_LIMITED,
	NX_D6JOINT_MOTION_LOCKED,
	

};

struct	NxTwistLimit
{
	NxSwingLimit low;
	NxSwingLimit high;
};

struct NxLinearLimit
{
	float value;
};

enum NxProjectionMode
{
	NX_JPM_NONE=1,
	NX_JPM_POINT_MINDIST,
	NX_JPM_LINEAR_MINDIST
};

enum NxJointFlags
{
	NX_JF_COLLISION_ENABLED = 1,
	NX_D6JOINT_GEAR_ENABLED = 2
};



struct NxD6JointDesc
{
	char* name;
	//NxActor* actor[2];
	class btRigidBody* bulletBodies[2];

	bool isValid() { return true;}

	btVector3 localAxis[2];
	btVector3  localNormal[2];
	btVector3  localAnchor[2];

	int swing1Motion;
	NxSwingLimit	swing1Limit;

	int swing2Motion;
	NxSwingLimit	swing2Limit;

	int twistMotion;
	NxTwistLimit	twistLimit;

	int	xMotion;
	int	yMotion;
	int	zMotion;

	NxLinearLimit linearLimit;

	short int projectionMode;
	float projectionDistance;
	float projectionAngle;

	int jointFlags;
	float gearRatio;
};


enum NxAssertResponse
{
	NX_AR_CONTINUE = 1
};

struct NxScene
{
	void	releaseActor(NxActor* actor) {}

	NxJoint* createJoint(NxD6JointDesc& desc) { return 0;}
};

struct NxUserContactReport
{
};

struct NxContactPair
{
};

struct NxShape
{
};

struct NxFluid
{
};

struct NxFluidDesc
{
};
struct NxBodyDesc
{
	int flags;
	btScalar mass;
	int solverIterationCount;
	btVector3 linearVelocity;
	btVector3 angularVelocity;
};

enum BodyFlags
{
	NX_BF_KINEMATIC = 1
};
struct NxActorDesc
{
	NxBodyDesc* body;
	btScalar mass;
	btScalar density;
	int flags;
	char* name;
	void* userData;
	Matrix3 globalPose;
	Matrix3 localPose;
	bool isValid() { return true;}
};

enum NxShapeType
{
	BLAAT=1,
	NX_SHAPE_SPHERE,
	NX_SHAPE_BOX,
	NX_SHAPE_CAPSULE,
	NX_SHAPE_CONVEX,
	NX_SHAPE_MESH
};

struct NxConvexMeshDesc
{
	int numVertices;
	int numTriangles;
	btVector3* points;
	int pointStrideBytes;
	int triangleStrideBytes;
	int* triangles;
};
class MxActor;
class MxJoint;
class MxObject;
class ObjectState;

class MxUtils
{
public:
	static bool PreparePlugin();
	static bool ReleasePlugin(bool quitting=false);
	static TriObject* GetTriObjectFromNode(INode* node, TimeValue t, int& needDelete);
	static MxJoint* GetJointFromNode(INode* node);
	static MxActor* GetActorFromName(const char* name);
	static MxActor* GetActorFromNode(INode* node);
	static int checkNodeValidity(INode* node, const ObjectState& os);
	static void     PrintMatrix3(Matrix3& m);
};

#if 0

#include <Max.h>
#include <iparamb2.h>


enum MxObjectType;

class MxMathUtils {
public:

	static Matrix3 NxMatrixToMax(const NxMat34& nxm)
	{
		float a[16];
		nxm.getColumnMajor44(a);
		return Matrix3(Point3(a[0],a[1],a[2]),Point3(a[4],a[5],a[6]),Point3(a[8],a[9],a[10]),Point3(a[12],a[13],a[14]));
	}

	static NxMat34 MaxMatrixToNx(const Matrix3& m)
	{
		NxMat34 nxm;
		(Point3&)nxm.t = m[3];
		for(int i=0;i<3;i++) nxm.M.setColumn(i,(const NxVec3&)m[i]);
		return nxm;
	}

	static NxVec3 Point3ToNxVec3(const Point3& p)
	{
		return NxVec3(p.x, p.y, p.z);
	}

	static Point3 NxVec3ToPoint3(const NxVec3& v)
	{
		return Point3(v.x, v.y, v.z);
	}

	static inline NxMat34 transformToParent(const NxMat34& ref, const NxMat34& t) 
	{
		return ref * t;
	}

	static inline NxMat34 transformToLocal(const NxMat34& ref, const NxMat34& t) 
	{
		NxMat34 inv;
		ref.getInverse(inv);
		return inv * t;
	}

	static inline NxMat33 transformToParent(const NxMat33& ref, const NxMat33& t) 
	{
		return ref * t;
	}

	static inline NxMat33 transformToLocal(const NxMat33& ref, const NxMat33& t) 
	{
		NxMat33 inv;
		ref.getInverse(inv);
		return inv * t;
	}

	static inline NxVec3 transformToParent(const NxMat34& ref, const NxVec3& v) 
	{
		return ref * v;
	}

	static inline NxVec3 transformToLocal(const NxMat34& ref, const NxVec3& v) 
	{
		NxMat34 inv;
		ref.getInverse(inv);
		return inv * v;
	}

	static inline NxVec3 transformToParent(const NxMat33& ref, const NxVec3& v) 
	{
		return ref * v;
	}

	static inline NxVec3 transformToLocal(const NxMat33& ref, const NxVec3& v) 
	{
		NxMat33 inv;
		ref.getInverse(inv);
		return inv * v;
	}

	static Point3 getScaleFromMaxMatrix(const Matrix3& tm) {
		Point3 X = tm.GetRow(0);
		Point3 Y = tm.GetRow(1);
		Point3 Z = tm.GetRow(2);

		Point3 scale;

		scale.x = X.Length();
		X.Normalize();

		Y = Y - (X * DotProd(Y, X));
		Z = Z - (X * DotProd(Z, X));

		scale.y = Y.Length();
		Y.Normalize();

		Z = Z - (Y * DotProd(Z, Y));

		scale.z = Z.Length();
		Z.Normalize();

		return scale;
	}

	static Matrix3 normalizeMaxMatrix(const Matrix3& tm) {
		Point3 X = tm.GetRow(0);
		Point3 Y = tm.GetRow(1);
		Point3 Z = tm.GetRow(2);

		X.Normalize();
		Y = Y - (X * DotProd(Y, X));
		Z = Z - (X * DotProd(Z, X));
		Y.Normalize();
		Z = Z - (Y * DotProd(Z, Y));
		Z.Normalize();

		Matrix3 result = tm;
		result.SetRow(0, X);
		result.SetRow(1, Y);
		result.SetRow(2, Z);

		return result;
	}

	static Matrix3 transformToLocal(const Matrix3& ref, const Matrix3& tm)
	{
		return tm * Inverse(ref); //taken from Meqon plugin
	}

	static Matrix3 transformToParent(const Matrix3& ref, const Matrix3& tm)
	{
		return ref * tm; //taken from Meqon plugin
	}

	Matrix3 getDifferenceTransform(Matrix3& ref, Matrix3& tm) {
		return transformToLocal(ref, tm);
	}

	static Point3 transformToParentPoint(const Matrix3& ref, const Point3& point)
	{
		return ref.PointTransform(point);
	}

	static Point3 transformToParentVector(const Matrix3& ref, const Point3& vec)
	{
		return ref.VectorTransform(vec);
	}

	static Point3 transformToLocalPoint(const Matrix3& ref, const Point3& point)
	{
		Matrix3 inv = Inverse(ref);
		return inv.PointTransform(point);
	}

	static Point3 transformToLocalVector(const Matrix3& ref, const Point3& vec)
	{
		Matrix3 inv = Inverse(ref);
		return inv.VectorTransform(vec);
	}
};

#endif

class MxParamUtils {
public:
	static INode* GetINodeParam(IParamBlock2* pb, TimeValue t, const char* name, INode* def)
	{
		int id  = FindParam(pb, name);
		if (id < 0) return def;
		return pb->GetINode(id, t);
	}

	static NxVec3 GetVectorParam(IParamBlock2* pb, TimeValue t, const char* name, NxVec3& def)
	{
		int id  = FindParam(pb, name);
		if (id < 0) return def;
		Point3 p = pb->GetPoint3(id, t);
		return NxVec3(p.x, p.y, p.z);
	}

	static bool GetBoolParam(IParamBlock2* pb, TimeValue t, const char* name, bool def)
	{
		int id  = FindParam(pb, name);
		if (id < 0) return def;
		return pb->GetInt(id, t) != 0;
	}

	static int GetIntParam(IParamBlock2* pb, TimeValue t, const char* name, int def) 
	{
		int id  = FindParam(pb, name);
		if (id < 0) return def;
		return pb->GetInt(id, t);
	}

	static float GetFloatParam(IParamBlock2* pb, TimeValue t, const char* name, float def) 
	{
		int id  = FindParam(pb, name);
		if (id < 0) return def;
		return pb->GetFloat(id, t);
	}

	static int FindParam(IParamBlock2* pb, const char* name)
	{
		int numParams = pb->NumParams();
		for (int i = 0; i < numParams; i++) {
			if (_stricmp(pb->GetParamDef(i).int_name, name) == 0) return i;
		}
		return -1;
	}
};


class MxUserPropUtils
{
public:

	static int GetUserPropFloat(INode* node, const char* k, float& x)
	{
		TSTR s;
		int r = node->GetUserPropString(k, s);
		if (!r) return 0;
		if (!s.data() || !*s.data()) return 0;
		r = sscanf_s(s.data(), "%f", &x);
		return r;
	}

	static int GetUserPropInt(INode* node, char* k, int defaultVal)
	{
		int val = defaultVal;
		node->GetUserPropInt(k, val);
		return val;
	}

	static bool GetUserPropBool(INode* node, char* k)
	{
		BOOL val;
		BOOL result = node->GetUserPropBool(k, val);
		if (!result) return false;
		return val != FALSE;
	}

	static BOOL GetUserPropBool(INode* node, char* k, BOOL defaultVal)
	{
		BOOL val = defaultVal;
		node->GetUserPropBool(k, val);
		return val;
	}

	static TSTR GetUserPropStr(INode *node,char *k)
	{
		TSTR s;
		int r = node->GetUserPropString(k,s);
		if(r) return s;
		return TSTR("");
	}

};



struct PxSimpleMesh
{
	PxSimpleMesh() : numPoints(0), numFaces(0), points(0), normals(0), faces(0) {}

	void buildFrom(NxConvexMeshDesc& mesh)
	{
		alloc(mesh.numVertices, mesh.numTriangles);

		NxU32 size = numFaces * 3;
		faces = new NxU32[size];

		for(NxU32 i = 0; i < numPoints; ++i)
		{
			NxVec3* p = (NxVec3*) ((char*) mesh.points + i * mesh.pointStrideBytes);
			points[i] = *p;
		}

		for(NxU32 i = 0; i < numFaces; ++i)
		{
			NxU32* p = (NxU32*) ((char*) mesh.triangles + i * mesh.triangleStrideBytes);
			faces[i*3] = p[0];
			faces[i*3 + 1] = p[1];
			faces[i*3 + 2] = p[2];
		}
	}

	void alloc(NxU32 numberPoints, NxU32 numberFaces)
	{
		release();

		if(numberPoints > 0) {
			numPoints = numberPoints;
			points = new NxVec3[numPoints];
			normals = new NxVec3[numPoints];
		}
		if(numberFaces > 0) {
			numFaces = numberFaces;
			faces = new NxU32[numberFaces*3];
		}
	}

	void release() 
	{
		if(points) 
		{
			delete [] points; 
			points = 0;
		}
		if(normals)
		{
			delete [] normals;
			normals = 0;
		}
		if(faces)
		{
			delete [] faces;
			faces = 0;
		}
		numPoints = numFaces = 0;
	}

	void clone(PxSimpleMesh& another)
	{
		if(this != &another)
		{
			alloc(another.numPoints, another.numFaces);
			memcpy(points, another.points, sizeof(NxVec3) * numPoints);
			memcpy(normals, another.normals, sizeof(NxVec3) * numPoints);
			memcpy(faces, another.faces, sizeof(NxU32) * numFaces * 3);
		}
	}

	NxU32 numPoints;
	NxU32 numFaces;
	NxVec3* points;
	NxVec3* normals;
	NxU32* faces;
};

#if 0

class MxUtils
{
public:
	static bool PreparePlugin();
	static bool ReleasePlugin(bool quitting=false);

	static NxShape* GetNxShapeFromName(const char* name);

	static MxActor* GetActorFromNode(INode* node);
	static MxActor* GetActorFromName(const char* name);
	static MxJoint* GetJointFromNode(INode* node);
	static MxObject* GetFirstObjectOfTypeFromNode(INode* node, MxObjectType type);
	static TriObject* GetTriObjectFromNode(INode* node, TimeValue t, int& needDelete);
	static int checkNodeValidity(INode* node, const ObjectState& os);
	static NxMaterial* findExistingMaterial(NxScene* scene, const NxMaterialDesc& desc);

	static NxTriangleMesh* nodeToNxTriangleMesh(PxSimpleMesh& mesh);
	static NxConvexMesh* nodeToNxConvexMesh(PxSimpleMesh& mesh);

	static NxVec3 meshCenter(PxSimpleMesh& mesh);
	static void scaleMesh(PxSimpleMesh& mesh, float scale);
	static bool nodeToPxSimpleMesh(PxSimpleMesh& result, INode* node);
	static Mesh* pxSimpleMeshToNode(PxSimpleMesh& src);

	//static PxSimpleMesh	pxCreateConvexHull(PxSimpleMesh& src, int vertLimit, float inflation);
	static void	    pxCreateConvexHull(PxSimpleMesh& ret, PxSimpleMesh& src, int vertLimit, float inflation);
	static Mesh*	pxCreateConvexHull(Mesh* src, int vertLimit, float inflation);

	static void     PrintMatrix3(Matrix3& m);
};

#endif


#endif //MX_UTILS_H