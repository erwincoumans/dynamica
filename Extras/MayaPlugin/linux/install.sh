#! /bin/sh

## Change this if you want to change the installation directory
MAYA_PLUG_IN_PATH=/usr/autodesk/maya2012-x64

cp -f  DynamicaMayaPlugin.so  ${MAYA_PLUG_IN_PATH}/bin/plug-ins      
cp -f scripts/*.mel ${MAYA_PLUG_IN_PATH}/scripts        
cp -f icons/*.xpm ${MAYA_PLUG_IN_PATH}/icons  




