; Required Headers
!include "WordFunc.nsh"
!insertmacro WordReplace

; GetModInstallDir
;
; Gets the installation directory for Half-Life or Source Engine mods.
;
; Returns on top of stack:
;   The installation directory
;   or
;   "null"
;
; Usage:
;   Push "Source" ; or "GoldSrc"
;   Call GetModInstallDir
;   Pop $R0

Function GetModInstallDir
    Exch $R0
    ClearErrors
    StrCmp $R0 "Source" +1 +3
    ReadRegStr $R0 HKCU "Software\Valve\Steam" "SourceModInstallPath" 
    Goto +3
    StrCmp $R0 "GoldSrc" +1 +3
    ReadRegStr $R0 HKCU "Software\Valve\Steam" "SourceModInstallPath" 
    IfErrors +2
    Goto +2
    StrCpy $R0 "null"
    ${WordReplace} $R0 "/" "\" "+" $R0
    Push $R0
FunctionEnd

; GetSteamInstallDir
;
; Gets the installation directory for Steam.
;
; Returns on top of stack:
;   The installation directory
;   or
;   "NULL"
;
; Usage:
;   Call GetSteamInstallDir
;   Pop $R0
Function GetSteamInstallDir
  Push $R0
  ClearErrors
  ReadRegStr $R0 HKCU "Software\Valve\Steam" "SteamPath"
  IfErrors +2
  Goto +2
  StrCpy $R0 "null"
  ${WordReplace} $R0 "/" "\" "+" $R0
  Exch $R0
FunctionEnd

; GetSteamExe
;
; Gets the installation directory for Steam.
;
; Returns on top of stack:
;   The installation directory
;   or
;   "NULL"
;
; Usage:
;   Call GetSteamExe
;   Pop $R0
Function GetSteamExe
  Push $R0
  ClearErrors
  ReadRegStr $R0 HKCU "Software\Valve\Steam" "SteamExe"
  IfErrors +2
  Goto +2
  StrCpy $R0 "null"
  ${WordReplace} $R0 "/" "\" "+" $R0
  Exch $R0
FunctionEnd