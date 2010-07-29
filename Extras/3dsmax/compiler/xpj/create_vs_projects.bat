
rem generate VS project files
if not exist ..\vc8win32 mkdir ..\vc8win32

xpj3 -v 4 -t VC8 -p WIN32 -d COPY_COUNT=1 -d SDK_VERSION=281 -d PHYSX_SDK_PATH="c:/Program Files/NVIDIA Corporation/NVIDIA PhysX SDK/v2.8.1" -d MAX_9_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 9 SDK" -d MAX_2008_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 2008 SDK" -d MAX_2009_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 2009 SDK" -x maxplugin.xpj

xpj3 -v 4 -t VC8 -p WIN32 -d COPY_COUNT=1 -d SDK_VERSION=280 -d PHYSX_SDK_PATH="c:/Program Files/NVIDIA Corporation/NVIDIA PhysX SDK/v2.8.0" -d MAX_9_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 9 SDK" -d MAX_2008_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 2008 SDK" -d MAX_2009_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 2009 SDK" -x maxplugin.xpj

xpj3 -v 4 -t VC8 -p WIN32 -d COPY_COUNT=1 -d SDK_VERSION=273 -d PHYSX_SDK_PATH="c:/Program Files/NVIDIA Corporation/NVIDIA PhysX SDK/v2.7.3" -d MAX_9_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 9 SDK" -d MAX_2008_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 2008 SDK" -d MAX_2009_SDK="C:/majun/projects/3DMaxPlugin/maxSDK/3ds Max 2009 SDK" -x maxplugin.xpj
pause