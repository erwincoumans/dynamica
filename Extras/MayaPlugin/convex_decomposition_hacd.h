/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2006 Erwin Coumans  http://continuousphysics.com/Bullet/

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#ifndef CONVEX_DECOMPOSITION_HACD_H
#define CONVEX_DECOMPOSITION_HACD_H

#include "LinearMath/btAlignedObjectArray.h"


class btBroadphaseInterface;
class btOverlappingPairCache;
class btCollisionDispatcher;
class btConstraintSolver;
struct btCollisionAlgorithmCreateFunc;
class btDefaultCollisionConfiguration;
class btTriangleMesh;

class convex_decomposition_hacd
{
   int   	mBaseCount;
   int		mHullCount;

   //btTriangleMesh* mMergedTriangleMesh;
   std::vector<float> mMergedVertices;
   std::vector<int> mMergedIndices;

   void ConvexDecompResult(unsigned int hvcount,const float *hvertices,unsigned int htcount,const unsigned int *hindices);

public:
   btAlignedObjectArray<btTriangleMesh*> m_trimeshes;
   btAlignedObjectArray<btConvexHullShape*> m_convexShapes;
   btAlignedObjectArray<btVector3> m_convexCentroids;

   convex_decomposition_hacd();
   
   btCompoundShape* ConvexDecomp(int numVertices, float* vertices, int numIndices, const unsigned int* indices);

   int GetNumMergedVertices();
   int GetNumMergedIndices();
   void GetMergedVertices( float* vertices );
   void GetMergedIndices( int* indices );

};

#endif //CONVEX_DECOMPOSITION_HACD_H
