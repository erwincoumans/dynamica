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

#include "hacdCircularList.h"
#include "hacdVector.h"
#include "hacdICHull.h"
#include "hacdGraph.h"
#include "hacdHACD.h"

#include <maya/MProgressWindow.h>
#include <maya/MGlobal.h>

//#include "ConvexBuilder.h"

#include "btBulletDynamicsCommon.h"

#include "LinearMath/btVector3.h"
#include "LinearMath/btQuickprof.h"
#include "LinearMath/btIDebugDraw.h"
#include "LinearMath/btGeometryUtil.h"
#include "BulletCollision/CollisionShapes/btShapeHull.h"

#include "convex_decomposition_hacd.h"

////////////////////////////////////

convex_decomposition_hacd::convex_decomposition_hacd()
{
}

void convex_decomposition_hacd::ConvexDecompResult(unsigned int hvcount,const float *hvertices,unsigned int htcount,const unsigned int *hindices)
{
   btVector3	centroid=btVector3(0,0,0);
   btVector3   convexDecompositionObjectOffset(10,0,0);

	btTriangleMesh* trimesh = new btTriangleMesh();
	m_trimeshes.push_back(trimesh);

	//calc centroid, to shift vertices around center of mass
	centroid.setValue(0,0,0);

	btAlignedObjectArray<btVector3> vertices;
	if ( 1 )
	{
		//const unsigned int *src = result.mHullIndices;
		for (unsigned int i=0; i<hvcount; i++)
		{
			btVector3 vertex(hvertices[i*3],hvertices[i*3+1],hvertices[i*3+2]);
			centroid += vertex;
		}
	}

	centroid *= 1.f/(float(hvcount) );

	if ( 1 )
	{
		//const unsigned int *src = result.mHullIndices;
		for (unsigned int i=0; i<hvcount; i++)
		{
			btVector3 vertex(hvertices[i*3],hvertices[i*3+1],hvertices[i*3+2]);
			vertex -= centroid ;
			vertices.push_back(vertex);
		}
	}

	if ( 1 )
	{
		const unsigned int *src = hindices;
		for (unsigned int i=0; i<htcount; i++)
		{
			unsigned int index0 = *src++;
			unsigned int index1 = *src++;
			unsigned int index2 = *src++;


			btVector3 vertex0(hvertices[index0*3], hvertices[index0*3+1],hvertices[index0*3+2]);
			btVector3 vertex1(hvertices[index1*3], hvertices[index1*3+1],hvertices[index1*3+2]);
			btVector3 vertex2(hvertices[index2*3], hvertices[index2*3+1],hvertices[index2*3+2]);

			vertex0 -= centroid;
			vertex1 -= centroid;
			vertex2 -= centroid;


			trimesh->addTriangle(vertex0,vertex1,vertex2);

         //added for maya
         mMergedVertices.push_back(vertex0.x()); mMergedVertices.push_back(vertex0.y()); mMergedVertices.push_back(vertex0.z());
         mMergedVertices.push_back(vertex1.x()); mMergedVertices.push_back(vertex1.y()); mMergedVertices.push_back(vertex1.z());
         mMergedVertices.push_back(vertex2.x()); mMergedVertices.push_back(vertex2.y()); mMergedVertices.push_back(vertex2.z());

			index0+=mBaseCount;
			index1+=mBaseCount;
			index2+=mBaseCount;

         mMergedIndices.push_back(index0);
         mMergedIndices.push_back(index1);
         mMergedIndices.push_back(index2);
		}
	}

	btConvexHullShape* convexShape = new btConvexHullShape(&(vertices[0].getX()),vertices.size());

	convexShape->setMargin(0.01f);
	m_convexShapes.push_back(convexShape);
	m_convexCentroids.push_back(centroid);
	mBaseCount+=hvcount; // advance the 'base index' counter.
};
extern bool gCancelRequest;

bool mayaCallback(const char * msg, double v0, double v1, size_t s1)
{
	if (v1>0.1f)
	{
		CHECK_MSTATUS(MProgressWindow::setProgress((v0/v1)*1000.f));
	} else
	{
		CHECK_MSTATUS(MProgressWindow::setProgress(0.f));
	}

	MString str(msg);
	CHECK_MSTATUS(MProgressWindow::setProgressStatus(str));


	if (MProgressWindow::isCancelled()) 
	{
			MGlobal::displayInfo("Progress interrupted!");
          
		gCancelRequest = true;
		return true;
	}

	return false;

}


btCompoundShape* convex_decomposition_hacd::ConvexDecomp(int numVertices, float* vertices, int numIndices,const unsigned int* indices)
{
	//-----------------------------------------------
	// HACD
	//-----------------------------------------------

	std::vector< HACD::Vec3<HACD::Real> > points;
	std::vector< HACD::Vec3<long> > triangles;

	for(int i=0; i<numVertices; i++ ) 
	{
		int index = i*3;
		HACD::Vec3<HACD::Real> vertex(vertices[index], vertices[index+1],vertices[index+2]);
		points.push_back(vertex);
	}

	for(int i=0;i<numIndices/3;i++)
	{
		int index = i*3;
		HACD::Vec3<long> triangle(indices[index], indices[index+1], indices[index+2]);
		triangles.push_back(triangle);
	}

	HACD::HACD myHACD;
	myHACD.SetPoints(&points[0]);
	myHACD.SetNPoints(points.size());
	myHACD.SetTriangles(&triangles[0]);
	myHACD.SetNTriangles(triangles.size());
	myHACD.SetCompacityWeight(0.1);
	myHACD.SetVolumeWeight(0.0);

	// HACD parameters
	// Recommended parameters: 2 100 0 0 0 0
	size_t nClusters = 2;
	double concavity = 10;
	bool invert = false;
	bool addExtraDistPoints = true;//false;
	bool addNeighboursDistPoints = true;//false;
	bool addFacesPoints = false;       

	myHACD.SetNClusters(nClusters);                     // minimum number of clusters
	myHACD.SetNVerticesPerCH(256);                      // max of 100 vertices per convex-hull
	myHACD.SetConcavity(concavity);                     // maximum concavity
	myHACD.SetAddExtraDistPoints(addExtraDistPoints);   
	myHACD.SetAddNeighboursDistPoints(addNeighboursDistPoints);   
	myHACD.SetAddFacesPoints(addFacesPoints); 

	myHACD.SetAddExtraDistPoints(true);   
	myHACD.SetAddFacesPoints(true); 


	
        MStatus stat = MS::kSuccess;
        MString title = "Esc to stop";
        MString sleeping = "Esc to stop";
        
        int amount = 0;
        int maxProgress = 1000;
        
        // First reserve the progress window.  If a progress window is already
        // active (eg. through the mel "progressWindow" command), this command
        // fails.
        //
        if (!MProgressWindow::reserve())
        {
                MGlobal::displayError("Progress window already in use.");
                stat = MS::kFailure;
        }

        //
        // Set up and print progress window state
        //


        CHECK_MSTATUS(MProgressWindow::setProgressRange(amount, maxProgress));
        CHECK_MSTATUS(MProgressWindow::setTitle(title));
        CHECK_MSTATUS(MProgressWindow::setInterruptable(true));
        CHECK_MSTATUS(MProgressWindow::setProgress(amount));

        MString progressWindowState = MString("Progress Window Info:") +
                MString("\nMin: ") + MProgressWindow::progressMin() +
                MString("\nMax: ") + MProgressWindow::progressMax() + 
                MString("\nTitle: ") + MProgressWindow::title() + 
                MString("\nInterruptible: ") + MProgressWindow::isInterruptable();

        MGlobal::displayInfo(progressWindowState);
        
        CHECK_MSTATUS(MProgressWindow::startProgress());
        
		int i=1;
		MString statusStr = sleeping;
        statusStr += i;

		 CHECK_MSTATUS(MProgressWindow::setProgressStatus(statusStr));
          CHECK_MSTATUS(MProgressWindow::advanceProgress(1));
		   MGlobal::displayInfo(MString("Current progress: ") + MProgressWindow::progress());
		    
		   MGlobal::executeCommand("pause -sec 1", false,false);
		   
        // Count 10 seconds
        //
/*        for (int i = amount; i < maxProgress; i++)
        {
                if (i != 0 && MProgressWindow::isCancelled()) {
                        MGlobal::displayInfo("Progress interrupted!");
                        break;
                }

                MString statusStr = sleeping;
                statusStr += i;
                CHECK_MSTATUS(MProgressWindow::setProgressStatus(statusStr));
                CHECK_MSTATUS(MProgressWindow::advanceProgress(1));

                MGlobal::displayInfo(MString("Current progress: ") + MProgressWindow::progress());

                MGlobal::executeCommand("pause -sec 1", false, false);
        }
		*/

        
        // End the progress, unreserving the progress window so it can be used
        // elsewhere.
        //
     
	myHACD.SetCallBack(mayaCallback);

	bool result = myHACD.Compute();
	if (!result)
	{
		nClusters = 0;
	} else
	{
		nClusters = myHACD.GetNClusters();
	}
	   
	CHECK_MSTATUS(MProgressWindow::endProgress());

	
	

//	myHACD.Save("output.wrl", false);

	btCompoundShape* compound = new btCompoundShape();
//   mMergedTriangleMesh = new btTriangleMesh();

   //now create some bodies
	if (1)
	{
		btTransform trans;
		trans.setIdentity();

		for (int c=0;c<nClusters;c++)
		{
			//generate convex result
			size_t nPoints = myHACD.GetNPointsCH(c);
			size_t nTriangles = myHACD.GetNTrianglesCH(c);

			float* vertices = new float[nPoints*3];
			unsigned int* triangles = new unsigned int[nTriangles*3];
			
			HACD::Vec3<HACD::Real> * pointsCH = new HACD::Vec3<HACD::Real>[nPoints];
			HACD::Vec3<long> * trianglesCH = new HACD::Vec3<long>[nTriangles];
			myHACD.GetCH(c, pointsCH, trianglesCH);

			// points
			for(size_t v = 0; v < nPoints; v++)
			{
				vertices[3*v] = pointsCH[v].X();
				vertices[3*v+1] = pointsCH[v].Y();
				vertices[3*v+2] = pointsCH[v].Z();
			}
			// triangles
			for(size_t f = 0; f < nTriangles; f++)
			{
				triangles[3*f] = trianglesCH[f].X();
				triangles[3*f+1] = trianglesCH[f].Y();
				triangles[3*f+2] = trianglesCH[f].Z();
			}

			delete [] pointsCH;
			delete [] trianglesCH;

			ConvexDecompResult(nPoints, vertices, nTriangles, triangles);
		}

		for (int i=0;i<m_convexShapes.size();i++)
		{
			btVector3 centroid = m_convexCentroids[i];
			trans.setOrigin(centroid);
			btConvexHullShape* convexShape = m_convexShapes[i];
			compound->addChildShape(trans,convexShape);
		}
	}

/*   mMergedTriangleVertices = new float[mNumMergedTriangleVertices*3];
   mMergedTriangleIndices = new int[mNumMergedTriangleIndices];
   for(int i=0; i<m_trimeshes.size(); i++)
   {
      mMergedTriangleVertices[i] = 
   }*/


   return compound;
}

int convex_decomposition_hacd::GetNumMergedVertices()
{
   return mMergedVertices.size()/3;
}

int convex_decomposition_hacd::GetNumMergedIndices()
{
   return mMergedIndices.size();
}

void convex_decomposition_hacd::GetMergedVertices( float* vertices)
{
   assert(vertices != NULL);

   for(int i=0; i<mMergedVertices.size(); i++)
   {
      vertices[i] = mMergedVertices[i];
   }
}

void convex_decomposition_hacd::GetMergedIndices( int* indices)
{
   assert(indices != NULL);

   for(int i=0; i<mMergedIndices.size(); i++)
   {
      indices[i] = mMergedIndices[i];
   }
}
