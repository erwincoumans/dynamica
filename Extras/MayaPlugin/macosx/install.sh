
## Change this if you want to change the installation directory
export MAYA_PLUG_IN_PATH=/Users/Shared/Autodesk/maya/2012
mkdir -p ${MAYA_PLUG_IN_PATH}
mkdir -p ${MAYA_PLUG_IN_PATH}/plug-ins
mkdir -p ${MAYA_PLUG_IN_PATH}/scripts
mkdir -p ${MAYA_PLUG_IN_PATH}/icons

cp -f  dynamicaMayaPlugin.bundle ${MAYA_PLUG_IN_PATH}/plug-ins      
cp -f scripts/*.mel ${MAYA_PLUG_IN_PATH}/scripts        
cp -f icons/*.xpm ${MAYA_PLUG_IN_PATH}/icons  




