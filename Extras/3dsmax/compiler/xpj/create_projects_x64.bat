rem generate VS project files
if not exist ..\vc8win64 mkdir ..\vc8win64
xpj3 -v 4 -t VC8 -p WIN64 -x pluginProjectsSDK281_x64.xpj
pause