;JulianaNFC install script

!define UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\JulianaNFC"

;--------------------------------
; Include MultiUser

  !define MULTIUSER_MUI
  !define MULTIUSER_EXECUTIONLEVEL Highest
  !define MULTIUSER_INSTALLMODE_COMMANDLINE
  !define MULTIUSER_INSTALLMODE_INSTDIR "JulianaNFC"
  !include MultiUser.nsh

;--------------------------------
; Include ModernUI

  !include MUI2.nsh

  !define MUI_ICON "..\..\code\main.ico"

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
; Basics

  Name "JulianaNFC"

  OutFile "JulianaNFC.installer.exe"

;--------------------------------
; Pages

  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MULTIUSER_PAGE_INSTALLMODE

  !define MUI_COMPONENTSPAGE_NODESC
  !insertmacro MUI_PAGE_COMPONENTS

  !insertmacro MUI_PAGE_DIRECTORY

  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\JulianaNFC"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Language

  !insertmacro MUI_LANGUAGE Dutch

;--------------------------------
; MultiUser stuff

  Function .onInit
    !insertmacro MULTIUSER_INIT
  FunctionEnd

  Function un.onInit
    !insertmacro MULTIUSER_UNINIT
  FunctionEnd

;--------------------------------
; Sections

  Section "JulianaNFC (verplicht)"

    ; ReadOnly
    SectionIn RO

    SetOutPath $INSTDIR

    File "..\..\build\Release-Win32\JulianaNFC.exe"

    File "..\..\build\Release-Win32\LIBEAY32.dll"
    File "..\..\build\Release-Win32\SSLEAY32.dll"
    File "..\..\build\Release-Win32\websockets.dll"

    !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

      CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
      CreateShortcut "$SMPROGRAMS\$StartMenuFolder\JulianaNFC.lnk" "$INSTDIR\JulianaNFC.exe"
      CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

    !insertmacro MUI_STARTMENU_WRITE_END

    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayIcon" "$INSTDIR\JulianaNFC.exe"
    WriteRegStr SHCTX "${UNINST_KEY}" "DisplayName" "JulianaNFC kaartlezer"
    WriteRegStr SHCTX "${UNINST_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr SHCTX "${UNINST_KEY}" "Publisher" "Cas Ebbers"
    WriteRegStr SHCTX "${UNINST_KEY}" "UninstallString" "$\"$INSTDIR\uninstall.exe$\" /$MultiUser.InstallMode"
    WriteRegDWORD SHCTX "${UNINST_KEY}" "NoModify" 1
    WriteRegDWORD SHCTX "${UNINST_KEY}" "NoRepair" 1
    WriteUninstaller "$INSTDIR\uninstall.exe"

  SectionEnd

  Section "Automatisch opstarten"

    CreateShortCut "$SMSTARTUP\JulianaNFC.lnk" "$INSTDIR\JulianaNFC.exe"

  SectionEnd

;--------------------------------
; Uninstaller

  Section "Uninstall"

    DeleteRegKey SHCTX "${UNINST_KEY}"
    DeleteRegKey SHCTX "Software\JulianaNFC"

    Delete "$INSTDIR\JulianaNFC.exe"
    Delete "$INSTDIR\LIBEAY32.dll"
    Delete "$INSTDIR\SSLEAY32.dll"
    Delete "$INSTDIR\websockets.dll"
    Delete "$INSTDIR\uninstall.exe"
    RMDir "$INSTDIR"

    !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

    Delete "$SMPROGRAMS\$StartMenuFolder\JulianaNFC.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
    RMDir "$SMPROGRAMS\$StartMenuFolder"

    Delete "$SMSTARTUP\JulianaNFC.lnk"

  SectionEnd
