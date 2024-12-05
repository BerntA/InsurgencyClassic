;
; Insurgency Mod Installer
; Global strings
;

; ---------------------------------------
; English

LoadLanguageFile "${NSISDIR}\Contrib\Language files\English.nlf"
!define "LANGUAGEFILE_English_USED"

; Section Name
LangString NAME_SecBase         ${LANG_ENGLISH} "Game Files"
LangString NAME_SecSteamSkins   ${LANG_ENGLISH} "Steam Skins"
LangString NAME_SecSkinSand     ${LANG_ENGLISH} "Insurgency - Sand"
LangString NAME_SecSkinUrban    ${LANG_ENGLISH} "Insurgency - Urban"

; Section Descriptions
LangString DESC_SecBase         ${LANG_ENGLISH} "Game Files"
LangString DESC_SecSteamSkins   ${LANG_ENGLISH} "Steam Skins"
LangString DESC_SecSkinSand     ${LANG_ENGLISH} "Insurgency - Sand"
LangString DESC_SecSkinUrban    ${LANG_ENGLISH} "Insurgency - Urban"

; Error Messages
LangString ERROR_ModPath        ${LANG_ENGLISH} "Could not find mod install path.  Verify your steam installation or manually enter an install path."
LangString ERROR_Skin           ${LANG_ENGLISH} "Could not install skin: "
LangString ERROR_Shortcut       ${LANG_ENGLISH} "Could not create desktop shortcut."

; Installer Options
LangString FINISH_Readme        ${LANG_ENGLISH} "Show ChangeLog"
LangString FINISH_Run           ${LANG_ENGLISH} "Create desktop shortcut"
LangString FINISH_Link          ${LANG_ENGLISH} "Read the manual"

; Filename for desktop shortciut.
LangString NAME_Shortcut        ${LANG_ENGLISH} "Play Insurgency - Modern Infantry Combat"

; Version Information Strings
VIAddVersionKey /LANG=${LANG_ENGLISH} "ProductName" "Insurgency"
VIAddVersionKey /LANG=${LANG_ENGLISH} "CompanyName" "Insurgency Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "LegalCopyright" "©2002-2007 Insurgency Team"
VIAddVersionKey /LANG=${LANG_ENGLISH} "FileVersion" "2.0"

; ---------------------------------------

