SetCompressor /SOLID /FINAL lzma

;--------------------------------
!define VersionName      "1.0.6 Beta"
!define PureName         "PhysX Plugin for 3ds Max"
!define ApplicationName  "${PureName}"

!define UXTVersion       "1.06.0000.0000"
!define UXTRegName       "PhysX_Plugin_for_3ds_Max"
!define UXTRegPath       "SOFTWARE\NVIDIA Corporation\Registration\${ApplicationName}"
!define UXTDLLNAME       "nvRegDev"
!define UXTDLL           "nvRegDev.dll"
!define UXTFUNC          "Register"
!define UXTEXE           "nvRegDevWin32.exe"

!define InstallerFile    "NVIDIA_PhysX_Plugin_for_3ds_Max_${VersionName}"
!define InstallerName    "${PureName} ${VersionName}"
!define StartMenuName    "NVIDIA Corporation\PhysX Plugin for 3ds Max ${VersionName}"
!define UninstallerName  "Uninstall ${InstallerName}"
!define LicensePath	     "..\doc\license.txt"
!define RegistryKey      "PhysXForMax"
!define RegistryPath     "Software\NVIDIA\${RegistryKey}"
!define TargetPath       "$PROGRAMFILES\NVIDIA Corporation\${InstallerName}"
!define PicPath          "pics"

!define MUI_COMPONENTSPAGE_SMALLDESC ;No value
!define MUI_ICON         "${PicPath}\Install01.ico"
!define MUI_UNICON       "${PicPath}\UnInstall.ico"

!define SDK273Name       "2.7.3B1"
!define SDK280Name       "2.8.0FC10"
!define SDK281Name       "2.8.1FC5"

;!define RegStartMenu     "StartMenu"

!include "MUI2.nsh"
!include "StrFunc.nsh"
# Declare used functions
${StrCase}
${StrClb}
${StrIOToNSIS}
${StrLoc}
${StrNSISToIO}
${StrRep}
${StrStr}
${StrStrAdv}
${StrTok}
${StrTrimNewLines}
${StrSort}

${UnStrCase}
${UnStrClb}
${UnStrIOToNSIS}
${UnStrLoc}
${UnStrNSISToIO}
${UnStrRep}
${UnStrStr}
${UnStrStrAdv}
${UnStrTok}
${UnStrTrimNewLines}
${UnStrSort}


Name         "${InstallerName}"
OutFile      "${InstallerFile}.exe"
BrandingText " " ; PhysXPluginDev@nvidia.com

;--------------------------------
;Request application privileges for Windows Vista
RequestExecutionLevel user
AutoCloseWindow false
XPStyle     on

WindowIcon  off
;--------------------------------
; variables
Var Max9
Var Max2008
Var Max2009
Var MaxOtherVersion

Var UXTIni
;Var UXTName
Var UXTOrg
Var UXTEmail
Var UXTAcceptEmail

;--------------------------------
Var SDKIni
Var SDKVersion
Var StartMenuFolder
;--------------------------------
;Interface Configuration

  !define MUI_HEADERIMAGE
  !define MUI_HEADERIMAGE_BITMAP "${PicPath}\PhysXbyNV_Black_small.bmp" ; optional
  !define MUI_ABORTWARNING

;--------------------------------
;Pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE       "${LicensePath}"
  Page custom ShowSDKPage LeaveSDKPage ;"Please choose correct PhysX SDK version that you are using."
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM"    ;HKCU
  ;!define MUI_STARTMENUPAGE_REGISTRY_KEY "${RegistryPath}" 
  ;!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "${StartMenuName}"
  ;!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  Page custom ShowUXTPage LeaveUXTPage ;"Please choose correct PhysX SDK version that you are using."
  !insertmacro MUI_PAGE_INSTFILES
  ;Page custom ShowUsagePage "" ;"Please choose correct PhysX SDK version that you are using."
  ;!define MUI_FINISHPAGE_TITLE "Important note"
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English" ;first language is the default language

;--------------------------------
;Reserve Files
  
  ;If you are using solid compression, files that are required before
  ;the actual installation should be stored first in the data block,
  ;because this will make your installer start faster.
  
;--------------------------------

;--------------------------------
;Installer Sections

;--------------------------------
;Installer Sections
Function ShowUXTPage
  !insertmacro MUI_HEADER_TEXT  "NVIDIA Developer Tools Registeration" "Please fill out the following information to register this free product."
  InstallOptions::initDialog /NOUNLOAD "$UXTIni"
  ; In this mode InstallOptions returns the window handle so we can use it
  Pop $0 ; get  window handle 
  ; Now show the dialog and wait for it to finish
  InstallOptions::show
  ; Finally fetch the InstallOptions status value (we don't care what it is though)
  Pop $0
FunctionEnd

Function LeaveUXTPage
  ; At this point the user has either pressed Next or one of our custom buttons
  ; We find out which by reading from the INI file
  ReadINIStr $0 $UXTIni "Settings" "State"
  ReadINIStr $R1 $UXTIni "Field 2" "State"
  ReadINIStr $R2 $UXTIni "Field 4" "State"
  ReadINIStr $R3 $UXTIni "Field 6" "State"
  ReadINIStr $R7 $UXTIni "Field 8" "State"
  ;messagebox MB_ICONQUESTION|MB_OK $0
checkName:
  ${StrRep} $R0 $R1 " " ""
  StrCmp $R0 "" +1 CheckOrg
  MessageBox MB_OK "Please input your name."
  Goto notvalid
CheckOrg:
  ${StrRep} $R0 $R2 " " ""
  StrCmp $R0 "" +1 checkSubscribe
  MessageBox MB_OK "Please input your organization."
  Goto notvalid
  
checkSubscribe:
  StrCmp $R7 "Yes, I would like to subscribe." checkemail notcheckemail
  
checkemail:
  StrCpy $R7 "1"
  ${StrRep} $R0 $R3 " " ""
  StrCmp $R0 "" +1 uxtReg
  MessageBox MB_OK "Please input your email address."
  Goto notvalid
  
notcheckemail:
  StrCpy $R7 "0"
  StrCmp $R1 "" notvalid uxtReg
  StrCmp $R2 "" notvalid uxtReg
  
uxtReg:
  ; try to do regeistration
  ClearErrors
  StrCpy $R4 "${UXTRegName}"
  StrCpy $R5 "${UXTVersion}"
  StrCpy $R6 "Install"

  InitPluginsDir
  ; try with SYSDIR
  SetOutPath $SYSDIR
  File ${UXTDLL}
  System::Call '${UXTDLLNAME}::${UXTFUNC}(t r11., t r12., t r13., t r14., t r15., t r16., t r17.) i .r0'
  ;MessageBox MB_ICONQUESTION|MB_OK "$SYSDIR\${UXTDLL}, ${UXTDLLNAME}::${UXTFUNC}('$R1', '$R2', '$R3', '$R4', '$R5', '$R6', '$R7') i .r0 return = $0"
  ;
  File ${UXTEXE}
  ;ExecWait '"$SYSDIR\${UXTEXE}" "$R1" "$R2" "$R3" "$R4" "$R5" "$R6" "$R7"' $0
  ;MessageBox MB_ICONQUESTION|MB_OK '"$SYSDIR\${UXTEXE}" "$R1" "$R2" "$R3" "$R4" "$R5" "$R6" "$R7" , return = $0'
  ;
  
  ${If} $0 == -1
    MessageBox MB_OK "Name is too long or too short. Please retry."
  ${ElseIf} $0 == -2
    MessageBox MB_OK "Organization is too long or too short. Please retry."
  ${ElseIf} $0 == -3
    MessageBox MB_OK "Email format is wrong. Please retry."
  ${ElseIf} $0 == -4
    MessageBox MB_OK "Product name is wrong. DEBUG INSTALLER SCRIPT."
  ${ElseIf} $0 == -5
    MessageBox MB_OK "Product version is in wrong format. DEBUG INSTALLER SCRIPT."
  ${ElseIf} $0 == -6
    MessageBox MB_OK "Install type format is wrong. DEBUG INSTALLER SCRIPT."
  ${ElseIf} $0 == -7
    MessageBox MB_OK "Subscribe value is wrong. DEBUG INSTALLER SCRIPT."
  ${ElseIf} $0 == 0
    ;MessageBox MB_OK "Fail to connect to license server."
    Goto validate
  ${Else}
    ;MessageBox MB_OK "Registration is successful. Thank you!"
    Goto validate
  ${EndIf}
  
notvalid:  
  ;System::Call "user32::GetWindowText(i $HWNDPARENT,t .r2,i ${NSIS_MAX_STRLEN}) i .r1"
  ;MessageBox MB_ICONQUESTION|MB_OK "test system::Call user32::GetWindowText($HWNDPARENT, $2, ${NSIS_MAX_STRLEN}) $1"
  Abort ; Return to the page
  ;ReadINIStr $0 $INI "Settings" "State"

validate:
  WriteRegStr HKLM "${UXTRegPath}" "SubscribeValue" $R7
  WriteRegStr HKLM "${UXTRegPath}" "RegistrationVer" "001"
  WriteRegStr HKLM "${UXTRegPath}" "Name" $R1
  WriteRegStr HKLM "${UXTRegPath}" "Organization" $R2
  WriteRegStr HKLM "${UXTRegPath}" "Email" $R3
  WriteRegStr HKLM "${UXTRegPath}" "ProductName" "${UXTRegName}"
  WriteRegStr HKLM "${UXTRegPath}" "ProductVersion" "${UXTVersion}"

  ;Goto notvalid
FunctionEnd	

Function ShowSDKPage
  !insertmacro MUI_HEADER_TEXT  "PhysX SDK Version" "Please select the PhysX SDK version that you are using."
  InstallOptions::initDialog /NOUNLOAD "$SDKIni"
  ; In this mode InstallOptions returns the window handle so we can use it
  Pop $0 ; get  window handle 
  ; Now show the dialog and wait for it to finish
  InstallOptions::show
  ; Finally fetch the InstallOptions status value (we don't care what it is though)
  Pop $0
FunctionEnd

Function LeaveSDKPage
  ; At this point the user has either pressed Next or one of our custom buttons
  ; We find out which by reading from the INI file
  ReadINIStr $0 $SDKIni "Settings" "State"
  ;MessageBox MB_ICONQUESTION|MB_OK "$0"
  StrCmp $0 0 validate  ; Next button?
  ;MessageBox MB_ICONQUESTION|MB_OK "$0 drop list"
  StrCmp $0 3 droplist 
  ;MessageBox MB_ICONQUESTION|MB_OK "$0 abort"
  Abort ; Return to the page
  ;ReadINIStr $0 $INI "Settings" "State"

droplist:
  ; Make the DirRequest field depend on the droplist
  ReadINIStr $0 "$SDKIni" "Field 3" "State"
  ;Strcpy $SDKVersion $0
  ;MessageBox MB_ICONQUESTION|MB_OK "$0"
  Abort ; Return to the page

validate:
  ReadINIStr $0 "$SDKIni" "Field 3" "State"
  Strcpy $SDKVersion $0
  
FunctionEnd	
	
Section "!Plugin Core" SectCommon  ; 
  SectionIn RO	; read only
  
  WriteRegStr HKLM "${RegistryPath}" "InstallPath"      $INSTDIR
  WriteRegStr HKLM "${RegistryPath}" "StartMenuFolder"  "$StartMenuFolder"
  WriteRegStr HKLM "${RegistryPath}" "Max9Path"         $Max9
  WriteRegStr HKLM "${RegistryPath}" "Max2008Path"      $Max2008
  WriteRegStr HKLM "${RegistryPath}" "Max2009Path"      $Max2009
  ;MessageBox MB_ICONQUESTION|MB_OK "$StartMenuFolder"
  
  SetOutPath "$INSTDIR\"
  WriteUninstaller "${UninstallerName}.exe" 

  SetOutPath "$INSTDIR\Max\Documents"
  File /a "Max\Documents\PhysX_Max_User_Guide.pdf"
  File /a "Max\Documents\Release Notes.htm"
  
  ;Create shortcuts
  ;!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
  CreateDirectory "$StartMenuFolder"
  CreateShortCut "$StartMenuFolder\PhysX Plugin User Guide.lnk" "$INSTDIR\Max\Documents\PhysX_Max_User_Guide.pdf"
  CreateShortCut "$StartMenuFolder\Release Notes.lnk" "$INSTDIR\Max\Documents\Release Notes.htm"
  CreateShortCut "$StartMenuFolder\${UninstallerName}.lnk" "$INSTDIR\${UninstallerName}.exe"
  ;!insertmacro MUI_STARTMENU_WRITE_END
  
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${RegistryKey}" "DisplayName" "${StartMenuName}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${RegistryKey}" "UninstallString" "$INSTDIR\${UninstallerName}.exe"
SectionEnd

SectionGroup "3ds Max" SectMax
Section "Max 9" SectMax9
  StrCmp $Max9 "" 0 BeginMax9
  Strcpy $Max9 "$INSTDIR\Max9"      ; this is for manuallly installation

BeginMax9:
  SetOutPath $Max9
  File NxTetra.dll
  
  StrCmp $SDKVersion "${SDK273Name}" 0 Check280
    SetOutPath $Max9\plugins
    File /a "273\max9\pxplugin.dlm"

    SetOutPath "$Max9\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max9\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver
 
Check280:
  StrCmp $SDKVersion "${SDK280Name}" 0 Check281
    SetOutPath $Max9\plugins
    File /a "280\max9\pxplugin.dlm"

    SetOutPath "$Max9\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max9\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver

Check281:
  StrCmp $SDKVersion "${SDK281Name}" 0 CheckOver
    SetOutPath $Max9\plugins
    File /a "281\max9\pxplugin.dlm"

    SetOutPath "$Max9\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max9\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver

CheckOver:

SectionEnd


Section "Max 2008" SectMax2008
  StrCmp $Max2008 "" 0 BeginMax2008
  Strcpy $Max2008 "$INSTDIR\Max2008"      ; this is for manuallly installation

BeginMax2008:
  SetOutPath $Max2008
  File NxTetra.dll
  
  StrCmp $SDKVersion "${SDK273Name}" 0 Check280
    SetOutPath $Max2008\plugins
    File /a "273\Max2008\pxplugin.dlm"

    SetOutPath "$Max2008\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max2008\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver
  
Check280:
  StrCmp $SDKVersion "${SDK280Name}" 0 Check281
    SetOutPath $Max2008\plugins
    File /a "280\Max2008\pxplugin.dlm"

    SetOutPath "$Max2008\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max2008\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver

Check281:
  StrCmp $SDKVersion "${SDK281Name}" 0 CheckOver
    SetOutPath $Max2008\plugins
    File /a "281\Max2008\pxplugin.dlm"

    SetOutPath "$Max2008\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max2008\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver

CheckOver:

SectionEnd


Section "Max 2009" SectMax2009
  StrCmp $Max2009 "" 0 BeginMax2009
  Strcpy $Max2009 "$INSTDIR\Max2009"      ; this is for manuallly installation

BeginMax2009:
  SetOutPath $Max2009
  File NxTetra.dll
  
  StrCmp $SDKVersion "${SDK273Name}" 0 Check280
    SetOutPath $Max2009\plugins
    File /a "273\Max2009\pxplugin.dlm"

    SetOutPath "$Max2009\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max2009\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver
  
Check280:
  StrCmp $SDKVersion "${SDK280Name}" 0 Check281
    SetOutPath $Max2009\plugins
    File /a "280\Max2009\pxplugin.dlm"

    SetOutPath "$Max2009\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max2009\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver

Check281:
  StrCmp $SDKVersion "${SDK281Name}" 0 CheckOver
    SetOutPath $Max2009\plugins
    File /a "281\Max2009\pxplugin.dlm"

    SetOutPath "$Max2009\Scripts\physx"
    File /a /r "..\scripts\physx\*.ms"

    SetOutPath "$Max2009\Scripts\startup"
    File /a "..\Scripts\startup\px_startup.ms"
	
	Goto CheckOver

CheckOver:

SectionEnd
SectionGroupEnd

Section "Samples" 
  SetOutPath "$INSTDIR\Max\Samples"
  File /a /r "Max\Samples\*.max"
  
  CreateDirectory "$StartMenuFolder\Samples"
  CreateShortCut "$StartMenuFolder\Samples\ballbounce.lnk" "$INSTDIR\Max\Samples\ballbounce.max"
  CreateShortCut "$StartMenuFolder\Samples\clothBall.lnk" "$INSTDIR\Max\Samples\clothBall.max"
  CreateShortCut "$StartMenuFolder\Samples\clothTearing.lnk" "$INSTDIR\Max\Samples\clothTearing.max"
  CreateShortCut "$StartMenuFolder\Samples\frictionramp.lnk" "$INSTDIR\Max\Samples\frictionramp.max"
  ;CreateShortCut "$StartMenuFolder\Samples\lincolnlogs.lnk" "$INSTDIR\Max\Samples\lincolnlogs.max"
  CreateShortCut "$StartMenuFolder\Samples\mousehead.lnk" "$INSTDIR\Max\Samples\mousehead.max"
  CreateShortCut "$StartMenuFolder\Samples\simpleJoint.lnk" "$INSTDIR\Max\Samples\simpleJoint.max"
  CreateShortCut "$StartMenuFolder\Samples\SeeSawFun.lnk" "$INSTDIR\Max\Samples\SeeSawFun.max"
  CreateShortCut "$StartMenuFolder\Samples\windmill.lnk" "$INSTDIR\Max\Samples\windmill.max"
  CreateShortCut "$StartMenuFolder\Samples\fountain.lnk" "$INSTDIR\Max\Samples\fountain.max"
  CreateShortCut "$StartMenuFolder\Samples\softfrog.lnk" "$INSTDIR\Max\Samples\softfrog.max"
  CreateShortCut "$StartMenuFolder\Samples\metalBarrel.lnk" "$INSTDIR\Max\Samples\metalBarrel.max"
  CreateShortCut "$StartMenuFolder\Samples\antigravity.lnk" "$INSTDIR\Max\Samples\antigravity.max"
SectionEnd

Function .onInit
	;check to prevent multiple instance
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "PluginInstallMutex") i .r1 ?e'
		Pop $R0
		StrCmp $R0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
		Abort
	; Check to see if 3dsmax is running.
	FindWindow $0 "3dsMAX" ""
	StrCmp $0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION "Cannot install while Autodesk 3ds Max is running. Please close the application and try again."
		Abort
   ;Strcpy $SDKVersion ""
  ;!insertmacro MUI_LANGDLL_DISPLAY
  
  SetShellVarContext all
  ;MessageBox MB_OK|MB_ICONEXCLAMATION "$SMPROGRAMS" 0 0
  ;SectionSetFlags ${SectMax} 32

  ReadRegStr $0 HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${RegistryKey}" "UninstallString"
  StrCmp $0 "" ContinueInstall 0
    MessageBox MB_YESNO|MB_ICONQUESTION "Another version of the tool is already installed, and must be removed to install this version.  Would you like to remove it?  If not, the installer will close." IDNO QUITINSTALL 
	;MessageBox MB_ICONQUESTION|MB_OK "$0"
	ExecWait '"$0" /S'
  
Goto ContinueInstall
QUITINSTALL:
	  Quit

ContinueInstall:
  Call GetPaths
  
  InitPluginsDir
  GetTempFileName $UXTIni $PLUGINSDIR
  File /oname=$UXTIni "UXT.ini"
  GetTempFileName $SDKIni $PLUGINSDIR
  File /oname=$SDKIni "SDKVersions.ini"
	
ENDOFINIT:
  ;System::Call 'keernel32::CreateMutexA(i 0, i 0, t "3dsmaxPluginMutex") i .r1 ?e'
  ;Pop $R0

FunctionEnd


Function GetPaths
  ClearErrors
  Strcpy $Max9    ""
  Strcpy $Max2008 ""
  Strcpy $Max2009 ""

  ReadRegStr $Max9    HKLM "SOFTWARE\Autodesk\3dsMax\9.0\MAX-1:409"  "Installdir"
  ReadRegStr $Max2008 HKLM "SOFTWARE\Autodesk\3dsMax\10.0\MAX-1:409" "Installdir"
  ReadRegStr $Max2009 HKLM "SOFTWARE\Autodesk\3dsMax\11.0\MAX-1:409" "Installdir"
  
Japanese:
  ReadRegStr $MaxOtherVersion    HKLM "SOFTWARE\Autodesk\3dsMax\9.0\MAX-1:411"  "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max9 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\10.0\MAX-1:411" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2008 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\11.0\MAX-1:411" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2009 $MaxOtherVersion
  
French:
  ReadRegStr $MaxOtherVersion    HKLM "SOFTWARE\Autodesk\3dsMax\9.0\MAX-1:40C"  "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max9 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\10.0\MAX-1:40C" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2008 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\11.0\MAX-1:40C" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2009 $MaxOtherVersion

German:
  ReadRegStr $MaxOtherVersion    HKLM "SOFTWARE\Autodesk\3dsMax\9.0\MAX-1:407"  "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max9 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\10.0\MAX-1:407" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2008 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\11.0\MAX-1:407" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2009 $MaxOtherVersion
  
Chinese:
  ReadRegStr $MaxOtherVersion    HKLM "SOFTWARE\Autodesk\3dsMax\9.0\MAX-1:804"  "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max9 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\10.0\MAX-1:804" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2008 $MaxOtherVersion

  ReadRegStr $MaxOtherVersion HKLM "SOFTWARE\Autodesk\3dsMax\11.0\MAX-1:804" "Installdir"
  Strcmp "$MaxOtherVersion" "" +2 0
  Strcpy $Max2009 $MaxOtherVersion
  
  StrCmp $Max9 "" 0 ThereIsMaxMayaXSI
	StrCmp $Max2008 "" 0 ThereIsMaxMayaXSI
		StrCmp $Max2009 "" 0 ThereIsMaxMayaXSI
			MessageBox MB_OK|MB_ICONEXCLAMATION "Setup cannot find your 3D Studio Max installation. Please install 3D Studio Max before installing the NVIDIA PhysX plug-in for Autodesk 3ds Max." 
			Abort

ThereIsMaxMayaXSI:

  Strcpy $StartMenuFolder "$SMPROGRAMS\${StartMenuName}"

  Strcpy $INSTDIR "${TargetPath}"
  Strcmp "$Max9" "" 0 +2
  SectionSetFlags ${SectMax9} 0

  Strcmp "$Max2008" "" 0 +2
  SectionSetFlags ${SectMax2008} 0

  Strcmp "$Max2009" "" 0 +2
  SectionSetFlags ${SectMax2009} 0

  ;MessageBox MB_ICONQUESTION|MB_OK "max9 path $Max9"
  ;MessageBox MB_ICONQUESTION|MB_OK "max2008 path $Max2008"
  ;MessageBox MB_ICONQUESTION|MB_OK "max2009 path $Max2009"
  
FunctionEnd


;--------------------------------
;Descriptions

  ;Language strings
  ;LangString DESC_SectCommon  ${LANG_ENGLISH} "This part is necessary for all."
  ;LangString DESC_SectMax9    ${LANG_ENGLISH} "If you want to install Plugin for 3ds Max 9 install, you need choose this."
  ;LangString DESC_SectMax2008 ${LANG_ENGLISH} "If you want to install Plugin for 3ds Max 2008 install, you need choose this."
  ;LangString DESC_SectMax2009 ${LANG_ENGLISH} "If you want to install Plugin for 3ds Max 2009 install, you need choose this."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SectCommon}  "This part is necessary for all." ;$(DESC_SectCommon)
    !insertmacro MUI_DESCRIPTION_TEXT ${SectMax9}    "If you want to install Plugin for 3ds Max 9 install, you need choose this." ;$(DESC_SectMax9)
    !insertmacro MUI_DESCRIPTION_TEXT ${SectMax2008} "If you want to install Plugin for 3ds Max 2008 install, you need choose this." ;$(DESC_SectMax2008)
    !insertmacro MUI_DESCRIPTION_TEXT ${SectMax2009} "If you want to install Plugin for 3ds Max 2009 install, you need choose this.";$(DESC_SectMax2009)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

UninstallText "Uninstall ${InstallerName}"
;UninstallIcon "${UnInstallIcon}"

Section "Uninstall"

RemoveMax9:  
  Strcmp "$Max9" "" RemoveMax2008 0
  Delete   "$Max9\NxTetra.dll"
  Delete   "$Max9\plugins\pxplugin.dlm"
  Delete   "$Max9\Scripts\startup\px_startup.ms"
  RMDir /r "$Max9\Scripts\physx"

RemoveMax2008:
  Strcmp "$Max2008" "" RemoveMax2009 0
  Delete   "$Max2008\NxTetra.dll"
  Delete   "$Max2008\plugins\pxplugin.dlm"
  Delete   "$Max2008\Scripts\startup\px_startup.ms"
  RMDir /r "$Max2008\Scripts\physx"

RemoveMax2009:
  Strcmp "$Max2009" "" RemoveCore 0
  Delete   "$Max2009\NxTetra.dll"
  Delete   "$Max2009\plugins\pxplugin.dlm"
  Delete   "$Max2009\Scripts\startup\px_startup.ms"
  RMDir /r "$Max2009\Scripts\physx"
  
RemoveCore:
  RMDir  /r "$INSTDIR\Max2009"
  RMDir  /r "$INSTDIR\Max2008"
  RMDir  /r "$INSTDIR\Max9"
  Delete "$INSTDIR\NxTetra.dll"
  RMDir  /r "$INSTDIR\Max\Documents"
  RMDir  /r "$INSTDIR\Max\Samples"
  RMDir  /r "$INSTDIR\Max"
  Delete "$INSTDIR\${UninstallerName}.exe"
  ;RMDir  /r "$INSTDIR"
  
  ;!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  ReadRegStr $StartMenuFolder HKLM "${RegistryPath}" "StartMenuFolder"  
  ;MessageBox MB_ICONQUESTION|MB_OK "$StartMenuFolder"
  Delete "$StartMenuFolder\Samples\*.lnk"		; /REBOOTOK 
  Delete "$StartMenuFolder\*.lnk"		; /REBOOTOK 
  RMDir  /r "$StartMenuFolder\Samples"		; /REBOOTOK 
  RMDir  /r "$StartMenuFolder"	
  DeleteRegKey /ifempty HKLM "${RegistryPath}"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${RegistryKey}"
 
  ; un-regiester uxt
  ; check uxt version
  ReadRegStr $0  HKLM "${UXTRegPath}" "RegistrationVer"
  StrCmp $0 "001" +1 UXT_IGNORE

  ReadRegStr $R1 HKLM "${UXTRegPath}" "Name"
  ReadRegStr $R2 HKLM "${UXTRegPath}" "Organization"
  ReadRegStr $R3 HKLM "${UXTRegPath}" "Email"
  ReadRegStr $R4 HKLM "${UXTRegPath}" "ProductName"
  ReadRegStr $R5 HKLM "${UXTRegPath}" "ProductVersion"
  StrCpy $R6 "Uninstall"
  ReadRegStr $R7 HKLM "${UXTRegPath}" "SubscribeValue"
  

  ; try with SYSDIR
  SetOutPath $SYSDIR
  ;File ${UXTDLL}
  System::Call "${UXTDLLNAME}::${UXTFUNC}(t r11., t r12., t r13., t r14., t r15., t r16., t r17.) i .r0"
  ;MessageBox MB_ICONQUESTION|MB_OK "$SYSDIR\${UXTDLL} ${UXTDLLNAME}::${UXTFUNC}('$R1', '$R2', '$R3', '$R4', '$R5', '$R6', '$R7') i .r0 return = $0"
  ;ExecWait '"$SYSDIR\${UXTEXE}" "$R1" "$R2" "$R3" "$R4" "$R5" "$R6" "$R7"' $0
  ;MessageBox MB_ICONQUESTION|MB_OK '"$SYSDIR\${UXTEXE}" "$R1" "$R2" "$R3" "$R4" "$R5" "$R6" "$R7" , return = $0'
  Delete "$SYSDIR\${UXTEXE}"
  Delete "$SYSDIR\${UXTDLL}"
  
  DeleteRegKey /ifempty HKLM "${UXTRegPath}"
  
  ;MessageBox MB_ICONQUESTION|MB_OK "uninstall UXT"
UXT_IGNORE:
  ;MessageBox MB_ICONQUESTION|MB_OK "Ignore UXT"
SectionEnd


;--------------------------------
;Uninstaller Functions
Function un.GetPaths
  ClearErrors
  Strcpy $Max9    ""
  Strcpy $Max2008 ""
  Strcpy $Max2009 ""

  ReadRegStr $Max9    HKLM "${RegistryPath}" "Max9Path"
  ReadRegStr $Max2008    HKLM "${RegistryPath}" "Max2008Path"
  ReadRegStr $Max2009    HKLM "${RegistryPath}" "Max2009Path"
  ;ReadRegStr $Max9    HKLM "${RegistryPath}" "Max9Path"
  ;ReadRegStr $Max9    HKLM "SOFTWARE\Autodesk\3dsMax\9.0\MAX-1:409"  "Installdir"
  ;ReadRegStr $Max2008 HKLM "SOFTWARE\Autodesk\3dsMax\10.0\MAX-1:409" "Installdir"
  ;ReadRegStr $Max2009 HKLM "SOFTWARE\Autodesk\3dsMax\11.0\MAX-1:409" "Installdir"

  ;Strcpy $INSTDIR $TargetPath
  ;MessageBox MB_ICONQUESTION|MB_OK "$INSTDIR"
FunctionEnd

Function un.onInit
	;check to prevent multiple instance
	System::Call 'kernel32::CreateMutexA(i 0, i 0, t "PluginUnInstallMutex") i .r1 ?e'
		Pop $R0
		StrCmp $R0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION "The installer is already running."
		Abort
	; Check to see if 3dsmax is running.
	FindWindow $0 "3dsMAX" ""
	StrCmp $0 0 +3
		MessageBox MB_OK|MB_ICONEXCLAMATION "Cannot install while 3dsMax is running. Please close 3dsMax."
		Abort

  ;!insertmacro MUI_UNGETLANGUAGE
  Call un.GetPaths

FunctionEnd