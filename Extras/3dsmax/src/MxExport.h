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

#ifndef MX_EXPORT_H
#define MX_EXPORT_H
#if 0
void SaveScene(NxScene* scene, const char* pFilename, NXU::NXU_FileType type)
{
	if ( scene )
	{
		NXU::NxuPhysicsCollection *c = NULL;
		c = NXU::extractCollectionSDK(&scene->getPhysicsSDK());
		bool saveDefaults = false;
#if NX_SDK_VERSION_NUMBER >= 272
		saveDefaults = PxFunctions::mSetting_savedefaults!=0;
#endif

      // Assign user-properties to joints.
      // Because of limitations of the NxuStream library we do it
      // from the outside...
      if ( c->mCurrentScene )
      {
         for ( int i = 0; i < c->mCurrentScene->mJoints.size(); i++ )
         {
            NXU::NxJointDesc *nxuJoint = c->mCurrentScene->mJoints[i];
         
            NxArray<MxObject*> mxArray;
            MxPluginData::getObjectsFromName( nxuJoint->name, mxArray );

            if ( mxArray.size() == 0 )
               continue;
            MxObject *mxObj = mxArray[0];

            INode *inode = mxObj->getNode();
            MSTR data;
            inode->GetUserPropBuffer( data );

            // Not sure if getGlobalString is really safe,
            // or will leak memory, etc..?
            nxuJoint->mUserProperties = NXU::getGlobalString( data );
         }
      }

		NXU::saveCollection(c, pFilename, type, true, saveDefaults); //saving defaults, there are problems with not doing so in the current version of the NxuStream2 (2.7.0).
		NXU::releaseCollection(c);
	}
}
#endif
#endif //MX_EXPORT_H
