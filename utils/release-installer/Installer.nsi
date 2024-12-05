; Insurgency Mod Installer Script
; Version 2

; ---------------------------------------
; Installer Configuration
!include "InstallConfig.nsh"

; ---------------------------------------
; General configuration
Name "Insurgency"
InstallDirRegKey HKLM "Software\SourceMods\Insurgency" "InstallDirectory"
SetCompressor /FINAL /SOLID lzma

; ---------------------------------------
; External References

; Include Modern UI
!include "MUI.nsh"

; Include custom utilities
!include "Utils.nsh"

; ---------------------------------------
; Interface configuration

; Additional init code
!define MUI_CUSTOMFUNCTION_GUIINIT InitInstaller

; Warn user on attemptin to abort instal.
!define MUI_ABORTWARNING

; Icons
!define MUI_ICON "art\game.ico"
!define MUI_UNICON "art\game.ico"

; Header Image (installer)
!define MUI_HEADERIMAGE
!define MUI_HEADERIMAGE_RIGHT
!define MUI_HEADERIMAGE_BITMAP "art\header.bmp"

; Welcome/Finish Page Image (installer)
!define MUI_WELCOMEFINISHPAGE_BITMAP "art\install_welcome.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP "art\uninstall_welcome.bmp"

; Finish page options
;  - Manual Link
!define MUI_FINISHPAGE_LINK $(FINISH_Link)
!define MUI_FINISHPAGE_LINK_LOCATION "http://insmod.net/manual"
;  - View ChangeLog option
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\ChangeLog.rtf"
!define MUI_FINISHPAGE_SHOWREADME_TEXT $(FINISH_Readme)
;  - Create Desktop Shortcut option
!define MUI_FINISHPAGE_RUN "null"
!define MUI_FINISHPAGE_RUN_TEXT $(FINISH_Run)
!define MUI_FINISHPAGE_RUN_FUNCTION CreateDesktopShortcut
;  - Don't auto-close the install progress page
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define NSIS_CONFIG_COMPONENTPAGE_ALTERNATIVE

; ---------------------------------------
; Language data and Version information
!ifdef PATCH
!include     "Patcher-Strings.nsh"
!else
!include     "Installer-Strings.nsh"
!endif
!insertmacro VERSION_INFO

; ---------------------------------------
; Installer Pages
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "license.rtf"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; Needed for English Language
!insertmacro MUI_LANGUAGE "English"

; ---------------------------------------
; Custom Installation Routines

Function InitInstaller
    Push $R0
    Push "Source"
    Call GetModInstallDir
    Pop $R0
    StrCmp $R0 "null" +1 +4
    MessageBox MB_OK $(ERROR_ModPath)
    StrCpy $INSTDIR "<your SoureMods directory>\insurgency"
    Goto +2
    StrCpy $INSTDIR "$R0\insurgency"
    Pop $R0
FunctionEnd

Function CreateDesktopShortcut
    Push $R0
    
    SetOutPath $INSTDIR
    Call GetSteamExe
    Pop $R0
    StrCmp $R0 "null" +3 +1 
    CreateShortcut "$DESKTOP\$(NAME_Shortcut)" $R0 '-applaunch 215 -game \"$INSTDIR\"' "$INSTDIR\resource\game.ico" 0
    Goto +2
    MessageBox MB_OK $(ERROR_Shortcut)
    
    Pop $R0
FunctionEnd

; ---------------------------------------
; Installer Sections

; Game files
Section $(NAME_SecBase) SecBase

    !insertmacro CORE_FILES
    
    ; Setup output path
    SetOutPath $INSTDIR
    ; Include ChangeLog
    # File "data\ChangeLog.rtf"
    ; Write installation information to the registry.
    WriteRegStr HKLM "Software\SourceMods\Insurgency" "InstallDirectory" "$INSTDIR"
    ; Create uninstaller and write uninstaller information to the registry.
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InsurgencyMod" "DisplayName" "Insurgency Mod"
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InsurgencyMod" "DisplayIcon" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr   HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InsurgencyMod" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InsurgencyMod" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InsurgencyMod" "NoRepair" 1
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

; Steam skins
SectionGroup $(NAME_SecSteamSkins) SecSteamSkins
Section $(NAME_SecSkinSand) SecSkinSand
    Push $R0
    
    Call GetSteamInstallDir
    Pop $R0
    StrCmp $R0 "null" +5 +1
    StrCpy $R0 "$R0\skins"
    SetOutPath $R0
    File /r "data\Insurgency - Sand"
    Goto +2
    MessageBox MB_OK "$(ERROR_Skin) $(DESC_SecSkinSand)"
    
    Pop $R0
SectionEnd
Section $(NAME_SecSkinUrban) SecSkinUrban
  Push $R0
 
  Call GetSteamInstallDir
  Pop $R0
  StrCmp $R0 "null" +5 +1
  StrCpy $R0 "$R0\skins"
  SetOutPath $R0
  File /r "data\Insurgency - Urban"
  Goto +2
  MessageBox MB_OK "$(ERROR_Skin) $(DESC_SecSkinUrban)"
  
  Pop $R0
SectionEnd
SectionGroupEnd

; ---------------------------------------
; Uninstaller Section

Section "Uninstall"

  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\InsurgencyMod"
  DeleteRegKey HKLM "Software\SourceMods\Insurgency"

  ; Remove desktop shortcut
  Delete "$DESKTOP\Insurgency Mod.lnk"

  ; Remove mod files
  RMDir /r $INSTDIR

SectionEnd

; ---------------------------------------
; Section Descriptions

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecBase} $(DESC_SecBase)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSteamSkins} $(DESC_SecSteamSkins)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSkinSand} $(DESC_SecSkinSand)
  !insertmacro MUI_DESCRIPTION_TEXT ${SecSkinUrban} $(DESC_SecSkinUrban)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

