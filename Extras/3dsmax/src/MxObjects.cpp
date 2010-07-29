#include <NxPhysics.h>
#include <max.h>
#include "MxObjects.h"

NxU32 MxObject::m_currentID = 0;


MxPhysXObjectRegsiter     MxObject::PhysXObjects;

void MxPhysXObjectRegsiter::clear()
{
	physXNodes.clear();
	maxNodes.clear();
	quickTable.clear();
}

void LoadChildren(ccMaxNodeContainer& container, INode* node)
{
	container.push_back(node);
	int number = node->NumberOfChildren();
	for(int i = 0; i <number; i++) {
		INode* child = node->GetChildNode(i);
		LoadChildren(container, child);
	}
}

void MxPhysXObjectRegsiter::loadMaxNodes()
{
	clear();
	INode* cur = GetCOREInterface()->GetRootNode();
	LoadChildren(maxNodes, cur);
}

void MxPhysXObjectRegsiter::registerObject(MxObject * obj)
{
	if(maxNodes.size() < 1)
		loadMaxNodes();
	INode* node = obj->getNode();
	if(node)
		quickTable[node] = obj;
}

void MxPhysXObjectRegsiter::unregisterObject(MxObject * obj)
{
	INode* node = obj->getNode();
	if(node)
		quickTable[node] = NULL;
}

void MxPhysXObjectRegsiter::arrange()
{
	physXNodes.clear();
	for(ccMaxNodeContainer::iterator it = maxNodes.begin(); it != maxNodes.end(); ++it) {
		ccPhysXMapContainer::iterator pit = quickTable.find(*it);
		if(pit != quickTable.end()) {
			physXNodes.push_back(pit->second);
		}
	}
}