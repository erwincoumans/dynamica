#ifndef DYN_DSOFTBODYCMD_H
#define DUN_DSOFTBODYCMD_H

#include <maya/MPxCommand.h>

#include <maya/MArgDatabase.h>
#include <maya/MDagModifier.h>
#include <maya/MSelectionList.h>

class dSoftBodyCmd :
	public MPxCommand
{
public:
	dSoftBodyCmd(void);
	virtual ~dSoftBodyCmd(void);
	static void *creator();
	static MSyntax syntax();
	
	MStatus doIt(const MArgList &i_mArgList);
	MStatus redoIt();
	MStatus undoIt();
	bool isUndoable() const { return true ; }
	bool hasSyntax() const { return true; }

	static MString typeName;

protected:
	MArgDatabase *m_argDatabase;
	MDagModifier *m_dagModifier;

	MSelectionList m_undoSelectionList;

};

#endif
