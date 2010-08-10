; example1.nsi
;
; This script is perhaps one of the simplest NSIs you can make. All of the
; optional settings are left to their default settings. The installer simply
; prompts the user asking them where to install, and drops a copy of "MyProg.exe"
; there.

;--------------------------------

; The name of the installer
Name "Bullet 2.77 physics plugin for Autodesk 3ds Max 2009"

; The file to write
OutFile "Bullet2.77_3dsMax2009Plugin_32bit.exe"


; The default installation directory
InstallDir "$PROGRAMFILES\Autodesk\3ds Max 2009\"

UninstPage uninstConfirm
UninstPage instfiles

; The text to prompt the user to enter a directory
DirText "This will install Bullet 2.77 for Autodesk 3ds Max. Choose destination directory"

;--------------------------------

; The stuff to install
Section "" ;No components page, name is not important
;Create plugin directories

CreateDirectory "$INSTDIR\plugins"	
CreateDirectory "$INSTDIR\scripts\"
CreateDirectory "$INSTDIR\scripts\startup"
CreateDirectory "$INSTDIR\scripts\physx"	


SetOutPath "$INSTDIR\scripts\startup"
File "scripts\startup\px_startup.ms"
SetOutPath "$INSTDIR\scripts\physx"
File "scripts\physx\*.*"
SetOutPath "$INSTDIR\plugins"
File "bin\release\281\2009\pxplugin.dlm"

;File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcp80.dll"
;File "C:\Program Files\Microsoft Visual Studio 8\VC\redist\x86\Microsoft.VC80.CRT\msvcr80.dll"

File "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcp90.dll"
File "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT\msvcr90.dll"


CreateDirectory "$DOCUMENTS\Bullet3dsMaxPlugin\"
CreateDirectory "$DOCUMENTS\Bullet3dsMaxPlugin\docs"
CreateDirectory "$DOCUMENTS\Bullet3dsMaxPlugin\examples"
SetOutPath	"$DOCUMENTS\Bullet3dsMaxPlugin\docs\"
File "doc\3ds Max Plugin User Guide.pdf"
	
;FileOpen $0 $DOCUMENTS\maya\modules\BulletDynamica.6_module a
;FileSeek $0 0 END
;FileWrite $0 "$INSTDIR$\n"
;FileClose $0

CreateDirectory "$SMPROGRAMS\Bullet3dsMaxPlugin"
CreateShortCut  "$SMPROGRAMS\Bullet3dsMaxPlugin\Documentation.lnk" "$DOCUMENTS\Bullet3dsMaxPlugin\docs\3ds Max Plugin User Guide.pdf"
CreateShortCut  "$SMPROGRAMS\Bullet3dsMaxPlugin\Examples.lnk" "$DOCUMENTS\Bullet3dsMaxPlugin\examples\"
CreateShortCut  "$SMPROGRAMS\Bullet3dsMaxPlugin\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
;ExecShell "open" "$INSTDIR\doc\index.html"

WriteUninstaller $INSTDIR\Uninstall.exe
SectionEnd ; end the section

Section "Uninstall"
	ClearErrors
	MessageBox MB_YESNO "Uninstall Bullet for 3ds Max?" IDNO end
		
	RMDir /r "$DOCUMENTS\Bullet3dsMaxPlugin\"
	Delete "$INSTDIR\plugins\pxplugin.dlm"
	Delete "$INSTDIR\scripts\startup\px_startup.ms"
	RMDir /r "$INSTDIR\scripts\physx\"
	RMDir /r "$SMPROGRAMS\Bullet3dsMaxPlugin\"
	end:
SectionEnd