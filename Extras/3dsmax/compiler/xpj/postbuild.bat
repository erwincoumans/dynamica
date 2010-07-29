echo Copying scripts to %1
mkdir %1\Scripts\physx
xcopy /Y /R /D  "..\..\scripts\startup\px_start.ms"  %1\Scripts\Startup
xcopy /Y /R /D  "..\..\scripts\physx\*.ms"           %1\Scripts\physx


:End

echo All done.