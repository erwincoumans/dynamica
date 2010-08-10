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


#include "MxUtils.h"

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