/*
Bullet Continuous Collision Detection and Physics Library Maya Plugin

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

Modified by Roman Ponomarev <rponom@gmail.com>
01/27/2010 : Replaced COLLADA export with Bullet binary export
*/

#include "bulletExport.h"
#include "solver.h"
#include "solver_impl.h"

#include "rigidBodyNode.h"
#include "constraint/nailConstraintNode.h"
#include "constraint/hingeConstraintNode.h"
#include "constraint/sliderConstraintNode.h"
#include "constraint/sixdofConstraintNode.h"

#if defined (_WIN32)
#define strcasecmp stricmp
#elif defined  (OSMac_)
extern "C" int strcasecmp (const char *, const char *);

#endif

#define NO_SMOOTHING_GROUP      -1
#define INITIALIZE_SMOOTHING    -2
#define INVALID_ID              -1

//////////////////////////////////////////////////////////////

MString BulletTranslator::fExtension = "bullet";

//////////////////////////////////////////////////////////////

void* BulletTranslator::creator()
{
    return new BulletTranslator();
}

//////////////////////////////////////////////////////////////

MStatus BulletTranslator::reader ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode)
{
    fprintf(stderr, "Bullet Physics import is not available yet\n");
    return MS::kFailure;
}



MStatus BulletTranslator::writer ( const MFileObject& file,
                                const MString& options,
                                FileAccessMode mode )

{
    MStatus status;
    
    MString mname = file.fullName(), unitName;
   
//just pass in the filename

#if defined (OSMac_)
	char fname[256];//MAXPATHLEN];
	strcpy (fname, file.fullName().asChar());
//	fp = fopen(fname,"wb");//MAYAMACTODO
#else
    const char *fname = mname.asChar();
  //  fp = fopen(fname,"w");
#endif

shared_ptr<solver_impl_t> solv = solver_t::get_solver();




	MStatus stat;
//	MItDag dagIterator( MItDag::kBreadthFirst, MFn::kInvalid, &stat);
	MItDag dagIterator( MItDag::kDepthFirst, MFn::kInvalid, &stat);
   	if (stat != MS::kSuccess)
	{
        std::cout << "Failure in DAG iterator setup" << std::endl;
   	}
	for ( ;!dagIterator.isDone(); dagIterator.next()) 
	{
   	    MDagPath dagPath;
   	    stat = dagIterator.getPath( dagPath );
   		if(stat != MS::kSuccess)
		{
			std::cout << "Failure in getting DAG path" << std::endl;
		}
		// skip over intermediate objects
		MFnDagNode dagNode( dagPath, &stat );
		if (dagNode.isIntermediateObject()) 
		{
			continue;
		}
		if(!dagPath.hasFn(MFn::kDependencyNode))
		{
			continue;
		}
		MObject mObj = dagNode.object(&stat);
		if(stat != MS::kSuccess)
		{
			std::cout << "Failure in getting MObject" << std::endl;
		}
        MFnDependencyNode fnNode(mObj, &stat);
		if(stat != MS::kSuccess)
		{
			std::cout << "Failure in getting dependency node" << std::endl;
		}

		if(fnNode.typeId() == rigidBodyNode::typeId) 
		{

		}
        if(fnNode.typeId() == rigidBodyNode::typeId) 
		{
			rigidBodyNode *rbNode = static_cast<rigidBodyNode*>(dagNode.userNode()); 
			
			rbNode->rigid_body()->register_name(solv.get(),rbNode->name().asChar());

		}
        if(fnNode.typeId() == nailConstraintNode::typeId) 
		{
			nailConstraintNode *ncNode = static_cast<nailConstraintNode*>(dagNode.userNode()); 
			ncNode->register_name(solv.get(),ncNode->name().asChar());
		}
        if(fnNode.typeId() == hingeConstraintNode::typeId) 
		{
			hingeConstraintNode *hcNode = static_cast<hingeConstraintNode*>(dagNode.userNode()); 
			std::cout << hcNode->name();
			
		}
        if(fnNode.typeId() == sliderConstraintNode::typeId) 
		{
			sliderConstraintNode *scNode = static_cast<sliderConstraintNode*>(dagNode.userNode()); 
		}
        if(fnNode.typeId() == sixdofConstraintNode::typeId) 
		{
			sixdofConstraintNode *sdNode = static_cast<sixdofConstraintNode*>(dagNode.userNode()); 
		}
	}




	solv->export_bullet_file(fname);

	return status;

}
//////////////////////////////////////////////////////////////

bool BulletTranslator::haveReadMethod () const
{
    return true;
}
//////////////////////////////////////////////////////////////

bool BulletTranslator::haveWriteMethod () const
{
    return true;
}
//////////////////////////////////////////////////////////////

MString BulletTranslator::defaultExtension () const
{
//    return MString("bullet");
	return fExtension;
}

MString BulletTranslator::filter() const
{
	//return "*.bullet;*.dae";
	return "*.bullet";
}

//////////////////////////////////////////////////////////////

MPxFileTranslator::MFileKind BulletTranslator::identifyFile (
                                        const MFileObject& fileName,
                                        const char* buffer,
                                        short size) const
{
    const char * name = fileName.name().asChar();
    int   nameLength = strlen(name);
    
    if ((nameLength > 7) && !strcasecmp(name+nameLength-7, ".bullet"))
        return kCouldBeMyFileType;
    else
        return kNotMyFileType;
}
//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////
MStatus BulletTranslator::exportSelected( )
{
	MStatus status;
	MString filename;


	// Create an iterator for the active selection list
	//
	MSelectionList slist;
	MGlobal::getActiveSelectionList( slist );
	MItSelectionList iter( slist );

	if (iter.isDone()) {
	    fprintf(stderr,"Error: Nothing is selected.\n");
	    return MS::kFailure;
	}

	
	return status;
}

//////////////////////////////////////////////////////////////

MStatus BulletTranslator::exportAll( )
{
	MStatus status = MS::kSuccess;

	return status;
}
