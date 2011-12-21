; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply
; prompts the user asking them where to install, and drops a copy of "MyProg.exe"
; there.

;--------------------------------

; The location of the module file
Var MODULE_FILE_LOCATION
!define MAYA_VERSION "2011-x64"
!define DYNAMICA_VERSION "2.80"
;Remember to change PROGRAMFILES64 to PROGRAMFILES for 32bit installers!
;And choose the appropriate amd64 or x86 msvcp*.dll and msvcr*.dll version


Function .onInit	
	Push $R0
	StrCpy $MODULE_FILE_LOCATION "$DOCUMENTS\maya\${MAYA_VERSION}\modules"
  ReadEnvStr $R0 "MAYA_APP_DIR"
   StrCmp $R0 "" done
   	StrCpy $MODULE_FILE_LOCATION "$R0\maya\${MAYA_VERSION}\modules"
   done:
   Pop $R0   
   StrCpy $INSTDIR "$PROGRAMFILES64\DynamicaBullet${DYNAMICA_VERSION}ForMaya${MAYA_VERSION}\"
FunctionEnd

; The name of the installer
Name "Dynamica Bullet ${DYNAMICA_VERSION} physics plugin for ${MAYA_VERSION}"

; The file to write
OutFile "DynamicaBullet${DYNAMICA_VERSION}ForMaya${MAYA_VERSION}.exe"

; The default installation directory
InstallDir "$PROGRAMFILES64\DynamicaBullet${MAYA_VERSION}\"

UninstPage uninstConfirm
UninstPage instfiles

; The text to prompt the user to enter a directory
DirText "This will install Dynamica Bullet For Maya. Choose destination directory"

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important
;Create Dynamica directories
;CreateDirectory "$INSTDIR\dll"
CreateDirectory "$INSTDIR\doc"	
CreateDirectory "$INSTDIR\doc\images"
CreateDirectory "$INSTDIR\scenes\"
CreateDirectory "$INSTDIR\icons"
CreateDirectory "$INSTDIR\plug-ins"	
CreateDirectory "$INSTDIR\scripts"	

;SetOutPath "$INSTDIR\dll"
;File "dll\*.dll"
SetOutPath "$INSTDIR\doc"
File "doc\*.*"
;SetOutPath "$INSTDIR\doc\images"
;File "doc\images\*.*"
SetOutPath "$INSTDIR\scenes"
File "scenes\*.*"
SetOutPath "$INSTDIR\icons"
File "icons\*.*"
SetOutPath "$INSTDIR\plug-ins"
File "..\..\lib\release\plug-ins\BulletMayaPlugin.mll"
;File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll"
;File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll"

;File "F:\sdks\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcp100.dll"
;File "F:\sdks\Microsoft Visual Studio 10.0\VC\redist\x64\Microsoft.VC100.CRT\msvcr100.dll"

;File "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcp90.dll"
;File "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcr90.dll"

File "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.CRT\msvcp90.dll"
File "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\amd64\Microsoft.VC90.CRT\msvcr90.dll"

SetOutPath "$INSTDIR\scripts"
File "scripts\*.*"
SetOutPath	$MODULE_FILE_LOCATION
File "BulletDynamica.6_module"

FileOpen $0 $MODULE_FILE_LOCATION\BulletDynamica.6_module a
FileSeek $0 0 END
FileWrite $0 "${DYNAMICA_VERSION} $INSTDIR$\n"
FileClose $0

CreateDirectory "$SMPROGRAMS\Dynamica Bullet"
CreateShortCut "$SMPROGRAMS\Dynamica Bullet\Documentation.lnk" "$INSTDIR\doc\index.html"
CreateShortCut "$SMPROGRAMS\Dynamica Bullet\Examples.lnk" "$INSTDIR\scenes\"
CreateShortCut "$SMPROGRAMS\Dynamica Bullet\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
ExecShell "open" "$INSTDIR\doc\index.html"

WriteUninstaller $INSTDIR\Uninstall.exe
SectionEnd ; end the section
 
Section "Uninstall"
	ClearErrors
	MessageBox MB_YESNO "Uninstall Bullet for MAYA?" IDNO end
	
	Delete "$MODULE_FILE_LOCATION\DynamicaBullet.6_module"
	RMDir /r "$SMPROGRAMS\Dynamica Bullet\"
	RMDir /r "$INSTDIR"
	end:
SectionEnd