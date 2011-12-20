#include "dSoftBodyCmd.h"
#include "softBodyNode.h"


#include <maya/MPxCommand.h>
#include <maya/MGlobal.h>
#include <maya/MSyntax.h>
#include <maya/MArgDatabase.h>
#include <maya/MDagModifier.h>
#include <maya/MString.h>

#include <iostream>

MString dSoftBodyCmd::typeName("dSoftBody");

dSoftBodyCmd::dSoftBodyCmd(void)
: m_argDatabase(0),
	m_dagModifier(0)
{
}

dSoftBodyCmd::~dSoftBodyCmd(void)
{
  if (m_argDatabase) {
    delete m_argDatabase;
  }

  if (m_dagModifier) {
    delete m_dagModifier;
  }
}

void *dSoftBodyCmd::creator()
{
	return new dSoftBodyCmd;
}

MStatus dSoftBodyCmd::doIt(const MArgList &args)
{
	MStatus stat;
	std::cout << "Invoking dSoftBodyCmd::doIt" << std::endl;
    m_argDatabase = new MArgDatabase(syntax(), args, &stat);
    if (stat == MS::kFailure) {
	return stat;
    }
    return redoIt();
}

MStatus dSoftBodyCmd::undoIt()
{
  MGlobal::setActiveSelectionList(m_undoSelectionList);

  if (m_dagModifier) {
      m_dagModifier->undoIt();
      delete m_dagModifier;
      m_dagModifier = 0;
  }

  return MS::kSuccess;
}


MStatus dSoftBodyCmd::redoIt()
{
    MGlobal::getActiveSelectionList(m_undoSelectionList);

    MString name;
    if (m_argDatabase->isFlagSet("-name")) 
	{
		m_argDatabase->getFlagArgument("-name", 0, name);
    }
    if (!name.length()) 
	{
		name = "dSoftBody";
    }

	
    m_dagModifier = new MDagModifier;
	
	// when creating the softbody node, we parent it with a transform
	// node named "dSoftBodyX" for X an integer
    
	MObject parentObj = m_dagModifier->createNode("transform");
    m_dagModifier->renameNode(parentObj, name + "#");
    m_dagModifier->doIt();

	MObject dSoftBodyObj = m_dagModifier->createNode(SoftBodyNode::typeId, parentObj);
    // m_dagModifier->renameNode(dSoftBodyObj, name);
	m_dagModifier->doIt();
	// rename the soft body node itself to be of the form
	// "dSoftBodyShapeX"
	
	std::string dSoftBodyName = MFnDependencyNode(parentObj).name().asChar();
	// determine position to insert string "shape" into the name
	std::string::size_type pos = dSoftBodyName.find_last_not_of("0123456789");    
	dSoftBodyName.insert(pos + 1, "Shape");
	m_dagModifier->renameNode(dSoftBodyObj, dSoftBodyName.c_str());
    m_dagModifier->doIt();

    // connect the solver attribute to the Dynamica solver	
	MPlug plgSolver(dSoftBodyObj, SoftBodyNode::ia_solver);
	MSelectionList slist;
    slist.add("dSolver1");
    MObject solverObj;
    if(slist.length() != 0) 
	{
        slist.getDependNode(0, solverObj);
	// comment this
		
		MPlug plgSoftBodies = MFnDependencyNode(solverObj).findPlug("rigidBodies", false);
		m_dagModifier->connect(plgSoftBodies, plgSolver);
		m_dagModifier->doIt();
    }

  //  MGlobal::select(parentObj, MGlobal::kReplaceList);

    setResult(MFnDependencyNode(dSoftBodyObj).name());
    return MS::kSuccess;
}

MSyntax dSoftBodyCmd::syntax()
{
    MSyntax syntax;
    syntax.enableQuery(false);
    syntax.enableEdit(false);

    syntax.addFlag("-n", "-name", MSyntax::kString);
  //  syntax.addFlag("-fn", "-filename", MSyntax::kString);
   // syntax.addFlag("-col", "-color", MSyntax::kString);
   // syntax.addFlag("-dia", "-diameter", MSyntax::kDouble);

    return syntax;
}
