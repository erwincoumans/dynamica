#ifndef MX_EXPORT_H
#define MX_EXPORT_H

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

#endif //MX_EXPORT_H
