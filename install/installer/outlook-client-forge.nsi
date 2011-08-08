 ;
 ; Funambol is a mobile platform developed by Funambol, Inc.
 ; Copyright (C) 2003 - 2007 Funambol, Inc.
 ;
 ; This program is free software; you can redistribute it and/or modify it under
 ; the terms of the GNU Affero General Public License version 3 as published by
 ; the Free Software Foundation with the addition of the following permission
 ; added to Section 15 as permitted in Section 7(a): FOR ANY PART OF THE COVERED
 ; WORK IN WHICH THE COPYRIGHT IS OWNED BY FUNAMBOL, FUNAMBOL DISCLAIMS THE
 ; WARRANTY OF NON INFRINGEMENT  OF THIRD PARTY RIGHTS.
 ;
 ; This program is distributed in the hope that it will be useful, but WITHOUT
 ; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 ; FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 ; details.
 ;
 ; You should have received a copy of the GNU Affero General Public License
 ; along with this program; if not, see http://www.gnu.org/licenses or write to
 ; the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 ; MA 02110-1301 USA.
 ;
 ; You can contact Funambol, Inc. headquarters at 643 Bair Island Road, Suite
 ; 305, Redwood City, CA 94063, USA, or at email address info@funambol.com.
 ;
 ; The interactive user interfaces in modified source and object code versions
 ; of this program must display Appropriate Legal Notices, as required under
 ; Section 5 of the GNU Affero General Public License version 3.
 ;
 ; In accordance with Section 7(b) of the GNU Affero General Public License
 ; version 3, these Appropriate Legal Notices must retain the display of the
 ; "Powered by Funambol" logo. If the display of the logo is not reasonably
 ; feasible for technical reasons, the Appropriate Legal Notices must display
 ; the words "Powered by Funambol".
 ;

; customization params
!include "customization.ini"

!include UAC.nsh

; ------ defines ------
!define PRODUCT_NAME_EXE                        "FunambolClient.exe"
!define MICROSOFT_OUTLOOK                       "Microsoft Outlook"
!define MICROSOFT_OUTLOOK_CLASS_NAME            "rctrl_renwnd32"
!define PLUGIN_UI_CLASS_NAME                    "FunambolApp"

!define PRODUCT_UNINST_ROOT_KEY                 "HKLM"
!define PRODUCT_STARTMENU_REGVAL                "NSIS:StartMenuDir"
!define PLUGIN_REGKEY_CONTEXT                   "Software\${PLUGIN_ROOT_CONTEXT}"
!define ADDIN_REGKEY_CONTEXT                    "Software\Microsoft\Office\Outlook\Addins\FunambolAddin.Addin"
!define PRODUCT_DIR_REGKEY                      "Software\Microsoft\Windows\CurrentVersion\App Paths\${PRODUCT_NAME_EXE}"
!define OUTLOOK_DIR_REGKEY                      "Software\Microsoft\Windows\CurrentVersion\App Paths\OUTLOOK.exe"
!define PRODUCT_UNINST_KEY                      "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_NAME}"
!define MSMAPIAPPS_REGKEY_CONTEXT               "Software\Microsoft\Windows Messaging Subsystem\MSMapiApps"
!define SHELLFOLDERS_CONTEXT                    "Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders"
!define TELEPHONY_LOCATIONS_CONTEXT             "Software\Microsoft\Windows\CurrentVersion\Telephony\Locations"
!define PROPERTY_PATH                           "installDir"
!define PROPERTY_ADDIN_NAME                     "FileName"
!define PROPERTY_SWV                            "swv"
!define PROPERTY_SP                             "portal"
!define PROPERTY_CUSTOMER                       "Customer"
!define PROPERTY_DESCRIPTION                    "Description"
!define PROPERTY_FUNAMBOL_SWV                   "funambol_swv"

;
; MEDIAHUB properties
;
;!define PROPERTY_MEDIAHUB                       "MediaHub"
;!define PROPERTY_MEDIAHUB_TITLE                 "MediaHub Location"
;!define PROPERTY_MEDIAHUB_DESCRIPTION           "Select the MediaHub Folder Location."
;!define PROPERTY_MEDIAHUB_TEXT                  "Pictures, videos and files stored in the MediaHub folder will be kept in sync with your online account."
;!define PROPERTY_MEDIAHUB_FOLDER_ALERT          "NOTE: All pictures, videos, and files will now be downloaded to the designated folder above (instead of \Pictures or \MyPictures directory)."
;!define PROPERTY_MEDIAHUB_FOLDER                "MediaHub Folder"
;!define PROPERTY_MEDIAHUB_SELECT_FOLDER         "Choose a location for your MediaHub folder. A folder called 'MediaHub' will be created in the folder that you select."


; the mediaHubPath is created in the root context registry. At the start, the value is copied into
; the right spds\sources\picture, spds\sources\video and files and removed from the root context.
; the PROPERTY_PICTURE_MEDIAHUB_KEY is used as check for the upgrade
!define PROPERTY_MEDIAHUB_REG_KEY               "mediaHubPath"
!define PROPERTY_PICTURE_MEDIAHUB_KEY           "spds\sources\picture"


; Before v.10 the product name was "Funambol Outlook Client"
; We want to be able to upgrade the Client from versions < 10.0.0.
!define OLD_PRODUCT_NAME                        "Funambol Outlook Sync Client"
!define OLD_PRODUCT_NAME_EXE                    "OutlookPlugin.exe"
!define OLD_PRODUCT_DIR_REGKEY                  "Software\Microsoft\Windows\CurrentVersion\App Paths\${OLD_PRODUCT_NAME_EXE}"
!define OLD_PRODUCT_UNINST_KEY                  "Software\Microsoft\Windows\CurrentVersion\Uninstall\${OLD_PRODUCT_NAME}"
!define OLD_STARTMENU_CONTEXT                   "Funambol\Outlook Sync Client"
!define OLD_INSTALLDIR_CONTEXT                  "Funambol\Outlook Client"
!define OLD_PLUGIN_UI_TITLE                     "Funambol Outlook Sync Client"



; MUI 1.67 compatible ------
!include "MUI.nsh"
!include "FileFunc.nsh"

!include "MUI2.nsh"

;--------------------------------
;Language Selection Dialog Settings

  ;Remember the installer language
  !define MUI_LANGDLL_REGISTRY_ROOT "HKCU"
  !define MUI_LANGDLL_REGISTRY_KEY "${PLUGIN_REGKEY_CONTEXT}"
  !define MUI_LANGDLL_REGISTRY_VALUENAME "Installer Language"
    

BrandingText "${PRODUCT_NAME} ${PRODUCT_VERSION}"

; MUI Settings
!define MUI_ABORTWARNING                        ; Show a message box with a warning when the user wants to close the installer.
!define MUI_ABORTWARNING_CANCEL_DEFAULT         ; Set the Cancel button as the default button on the message box.
!define MUI_ICON                                "fileset\images\install.ico"
!define MUI_UNICON                              "fileset\images\uninstall.ico"

; Welcome page
!define MUI_WELCOMEFINISHPAGE_BITMAP            "${PRODUCT_WELCOME_BMP}"
!define MUI_WELCOMEPAGE_TITLE_3LINES            ; extra space since the application name is quite long
!insertmacro MUI_PAGE_WELCOME

; License page
!ifdef SHOW_LICENSE
    !define MUI_LICENSEPAGE_CHECKBOX
    !insertmacro MUI_PAGE_LICENSE                   "fileset\LICENSES\License.txt"
!endif

;
; Page to show a custom dialog form
;
Page custom nsDialogsPage nsDialogsPageLeave

; Directory page, the first two define check if the install dir is correct
!define MUI_DIRECTORYPAGE_VERIFYONLEAVE
!insertmacro MUI_PAGE_DIRECTORY

var ICONS_GROUP
!define MUI_STARTMENUPAGE_NODISABLE
!define MUI_STARTMENUPAGE_DEFAULTFOLDER         "${STARTMENU_CONTEXT}"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT         "${PRODUCT_UNINST_ROOT_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_KEY          "${PRODUCT_UNINST_KEY}"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME    "${PRODUCT_STARTMENU_REGVAL}"
!insertmacro MUI_PAGE_STARTMENU Application $ICONS_GROUP


; Instfiles page
!insertmacro MUI_PAGE_INSTFILES

Function finishpageaction
    SetShellVarContext all
    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_NAME_EXE}" 
FunctionEnd

; Finish page
!define MUI_FINISHPAGE_RUN                     "$INSTDIR\${PRODUCT_NAME_EXE}"
!define MUI_FINISHPAGE_TITLE_3LINES            ; extra space since the application name is quite long!define MUI_FINISHPAGE_RUN_FUNCTION             ExecAppFile
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_CHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT         $(CREATE_APP_DESKTOP_LINK_MM)
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION finishpageaction

;!ifdef FINISHPAGE_SHOW_README
;    !define MUI_FINISHPAGE_SHOWREADME               "$INSTDIR\Readme.txt"
;    !define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
;!endif

!insertmacro MUI_PAGE_FINISH

; Uninstaller pages
!define MUI_UNABORTWARNING
UninstPage custom un.RemoveUserData un.nsDialogsPageLeaveCheckbox          ; Custom page, to ask if deleting users files/settings.
!insertmacro MUI_UNPAGE_INSTFILES

; Language files

  !insertmacro MUI_LANGUAGE "English" ;first language is the default language
  !insertmacro MUI_LANGUAGE "French"
  !insertmacro MUI_LANGUAGE "German"
  !insertmacro MUI_LANGUAGE "SimpChinese"
  !insertmacro MUI_LANGUAGE "TradChinese"
  !insertmacro MUI_LANGUAGE "Italian"
  !insertmacro MUI_LANGUAGE "Russian"
  !insertmacro MUI_LANGUAGE "Arabic"


;
; Need to add these here otherwise cannot be read by the localizedMessages.ini
;
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_ENGLISH} "Create shortcut on desktop"
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_ITALIAN} "Crea collegamento sul desktop"      
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_FRENCH}  "Créer un raccourci sur le bureau"        
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_GERMAN}  "Verknüpfung auf Desktop erstellen"      
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_SIMPCHINESE} "在桌面上创建快捷方式" 
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_TRADCHINESE} "在桌面上创建快捷方式" 
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_RUSSIAN} "Создать ярлык на рабочем столе"     
LangString CREATE_APP_DESKTOP_LINK_MM                  ${LANG_ARABIC} "إنشاء اختصار على سطح المكتب"      

; MUI end ------


; add all the localized messages
!include "localizedMessages.ini" 
  

Name              "${PRODUCT_NAME} ${PRODUCT_VERSION}"                       ; PRODUCT_VERSION passed as parameter by build.xml
OutFile           "..\output\${FILE_NAME}-${PRODUCT_VERSION}.exe"
InstallDir        "$PROGRAMFILES\${INSTALLDIR_CONTEXT}"

InstallDirRegKey  HKLM "${PRODUCT_DIR_REGKEY}" ""
ShowInstDetails   show
ShowUnInstDetails show
Icon              "${MUI_ICON}"

RequestExecutionLevel user    /* RequestExecutionLevel REQUIRED! */

Function .OnInstFailed
    ;${UAC.Unload} ;Must call unload!
    
FunctionEnd


!include nsDialogs.nsh
!include LogicLib.nsh

Var Dialog
Var Label
Var GroupBox
Var DirRequest
Var Button
Var mediaHubFolderChoosen
Var mediaHubFolder
Var showMediaHubPanel
Var ttttt

Function nsDialogsPage

    ${If} $showMediaHubPanel == "NO"
		Abort ; exit and don't show anything
	${EndIf}

    nsDialogs::Create 1018
    Pop $Dialog

    ${If} $Dialog == error
		Abort
	${EndIf}

    GetFunctionAddress $0 OnBack
	nsDialogs::OnBack $0
	
    !insertmacro MUI_HEADER_TEXT "${PROPERTY_MEDIAHUB_TITLE}" "${PROPERTY_MEDIAHUB_DESCRIPTION}"
        
    ${NSD_CreateLabel} 0 0 100% 25u "${PROPERTY_MEDIAHUB_TEXT}"
    Pop $Label
	
	; if MediaHub foder changed, let's append an alert text.
	${If} $showMediaHubPanel == "FolderChanged"
        ${NSD_CreateLabel} 0 85u 100% 25u "${PROPERTY_MEDIAHUB_FOLDER_ALERT}"
	    Pop $Label
        CreateFont $1 "$(^Font)" 8 600
        SendMessage $Label ${WM_SETFONT} $1 0
    ${EndIf}
	
	${If} $mediaHubFolderChoosen == ""
	        StrCpy $mediaHubFolderChoosen "$DOCUMENTS"
	${EndIf}
	
	${NSD_CreateLabel} 3% 46u 70% 15u "$mediaHubFolderChoosen\${PROPERTY_MEDIAHUB}"
	Pop $DirRequest
	${NSD_OnChange} $DirRequest nsDialogsPageTextChange

	${NSD_CreateButton} 76% 43u 20% 15u "${PROPERTY_MEDIAHUB_BROWSE}"
	Pop $Button
	GetFunctionAddress $0 OnClick
	nsDialogs::OnClick $Button $0
	
    ${NSD_CreateGroupBox} 0 29u 100% 40u "${PROPERTY_MEDIAHUB_FOLDER}"
	Pop $GroupBox
 
    nsDialogs::Show

FunctionEnd

Function nsDialogsPageTextChange
        ;basically avoid to edit the field manually
        ;${NSD_SetText} $DirRequest "$mediaHubFolderChoosen\${PROPERTY_MEDIAHUB}"
        ;StrCpy $ttttt "$mediaHubFolderChoosen\${PROPERTY_MEDIAHUB}"
        ;${NSD_SetText} $ttttt "$mediaHubFolderChoosen\${PROPERTY_MEDIAHUB}"
        ;MessageBox MB_ICONSTOP "$ttttt"
        ;StrCpy $DirRequest $ttttt
        ;MessageBox MB_ICONSTOP "$DirRequest"
        ;${NSD_SetText} $DirRequest "$ttttt"
FunctionEnd


Function OnClick

	Pop $0 # HWND
        nsDialogs::SelectFolderDialog "${PROPERTY_MEDIAHUB_SELECT_FOLDER}" $mediaHubFolderChoosen
        Pop $R1
	
	${If} $R1 == "error"
	        Abort
	${EndIf}
	;MessageBox MB_ICONSTOP "$mediaHubFolderChoosen"
	StrCpy $mediaHubFolderChoosen $R1
	;MessageBox MB_ICONSTOP "$R1"
	StrLen $R2 $mediaHubFolderChoosen             ; remove the \ at the end
	;MessageBox MB_ICONSTOP "$R2"
	IntOp  $R3  $R2 - 1
	;MessageBox MB_ICONSTOP "$R3"
	StrCpy $R2 $mediaHubFolderChoosen "" $R3
	;MessageBox MB_ICONSTOP "$R2"
        ${If} $R2 == "\"
              StrCpy $mediaHubFolderChoosen $R1  -1
	${EndIf}
    ${NSD_SetText} $DirRequest "$mediaHubFolderChoosen\${PROPERTY_MEDIAHUB}"
    
FunctionEnd

Function OnBack
	${NSD_GetText} $DirRequest $0
FunctionEnd

Function nsDialogsPageLeave
	${NSD_GetText} $DirRequest $0
	StrCpy $mediaHubFolder $0
FunctionEnd

Function createMediaHubFolder

    ${DirState} "$mediaHubFolder" $R0
    ${If} $R0 == -1
	    StrCpy $R1 $mediaHubFolder
        CreateDirectory "$mediaHubFolder"

        ${DirState} "$mediaHubFolder" $R1
        ${If} $R1 == -1
           MessageBox MB_ICONSTOP "Error creating the MediaHub Folder. Please check it and try again."
           Abort
        ${EndIf}

	${EndIf}
	
	${If} $mediaHubFolder != ""
        WriteRegStr    HKCU  "${PLUGIN_REGKEY_CONTEXT}"  "${PROPERTY_MEDIAHUB_REG_KEY}" "$mediaHubFolder"
	${EndIf}
	
FunctionEnd


Function .OnInstSuccess
    ;${UAC.Unload} ;Must call unload!
    
FunctionEnd

Function ExecAppFile
    ;UAC::Exec '' '"$INSTDIR\${PRODUCT_NAME_EXE}"' '' '' 
    !insertmacro UAC_AsUser_ExecShell '' '"$INSTDIR\${PRODUCT_NAME_EXE}"' '' '' ''
FunctionEnd


; Check if OUTLOOK.EXE process is running
Function CheckMicrosoftApp
        ; First try to close Outlook automatically.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "I need to close ${MICROSOFT_OUTLOOK} to proceed with the installation of $(^Name). Ok?" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 2500                                           ; wait 2500 ms for Outlook closing

        ; If Outlook still running, ask to close it manually.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "Could not close ${MICROSOFT_OUTLOOK}. Please close it manually to proceed with the installation of $(^Name)." IDCANCEL Cancel
        
  checkProcess:
        ; Check if OUTLOOK.EXE is still running.
        push "OUTLOOK.EXE"
        processwork::existsprocess
        pop $1
        IntCmp $1 0 done          ; exit only when OUTLOOK.EXE process is not found
        
        ; If OUTLOOK.EXE still running, it's probably stuck: kill the process silently!
        push "OUTLOOK.EXE"
        processwork::KillProcess
        Sleep 500
        goto checkProcess

  done:
        Return
  Cancel:
        MessageBox MB_OK "${INSTALLATION_ABORTED}"
        Abort
FunctionEnd


; Check if "OUTLOOK.EXE" process is running (for uninstaller)
Function un.CheckMicrosoftApp
        ; First try to close Outlook automatically.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "${CLOSE_UNINSTALL_OLK}" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 2500                                           ; wait 2500 ms for Outlook closing

        ; If Outlook still running, ask to close it manually.
        FindWindow $0 "${MICROSOFT_OUTLOOK_CLASS_NAME}"
        IntCmp $0 0 checkProcess
        MessageBox MB_OKCANCEL "${CANNOT_CLOSE_UNINSTALL_OLK}" IDCANCEL Cancel

  checkProcess:
        ; Check if OUTLOOK.EXE is still running.
        push "OUTLOOK.EXE"
        processwork::existsprocess
        pop $1
        IntCmp $1 0 done          ; exit only when OUTLOOK.EXE process is not found

        ; If OUTLOOK.EXE still running, it's probably stuck: kill the process silently!
        push "OUTLOOK.EXE"
        processwork::KillProcess
        Sleep 500
        goto checkProcess

  done:
        Return
  Cancel:
        MessageBox MB_OK "${UNINSTALL_FAILED}"
        Abort
FunctionEnd



; Check if "FunambolClient.exe" is running
Function CheckFunClientApp
        ; First try to close Windows client automatically.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "${CLOSE_INSTALLATION}" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 500                                            ; wait 500 ms for plugin closing
  loop1:
        ; If client still running, ask to close it manually.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "${CANNOT_CLOSE_INSTALLATION}" IDCANCEL Cancel
        goto loop1
  done:
        Return
  Cancel:
        MessageBox MB_OK "${INSTALLATION_ABORTED}"
        Abort
FunctionEnd


; Check if "OutlookPlugin.exe" is running - for OLD Client versions (< 10.0.0)
Function CheckOldFunClientApp
        ; First try to close Outlook plugin automatically.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${OLD_PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "${CLOSE_INSTALLATION_OLD_PRODUCT}" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 500                                            ; wait 500 ms for plugin closing
  loop1:
        ; If plugin still running, ask to close it manually.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${OLD_PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "${CANNOT_CLOSE_INSTALLATION_OLD_PRODUCT}" IDCANCEL Cancel
        goto loop1
  done:
        Return
  Cancel:
        MessageBox MB_OK "${INSTALLATION_ABORTED}"
        Abort
FunctionEnd


; Check if "FunambolClient.exe" is running (for uninstaller)
Function un.CheckFunClientApp
        ; First try to close Windows client automatically.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "${CLOSE_UNINSTALL_PRODUCT}" IDCANCEL Cancel
        SendMessage $0 16  0 0  $R1 /TIMEOUT=1000            ; WM_CLOSE = 16, timeout = 1000 ms
        Sleep 500                                            ; wait 500 ms for plugin closing
  loop1:
        ; If client still running, ask to close it manually.
        FindWindow $0 "${PLUGIN_UI_CLASS_NAME}" "${PLUGIN_UI_TITLE}"
        IntCmp $0 0 done
        MessageBox MB_OKCANCEL "${CANNOT_CLOSE_UNINSTALL_PRODUCT}" IDCANCEL Cancel
        goto loop1
  done:
        Return
  Cancel:
        MessageBox MB_OK "${UNINSTALL_FAILED}"
        Abort
FunctionEnd



; Check if current user have Administrator rights to run the installer.
; If not, the installer will be aborted.
Function CheckUserRights
    uac_tryagain:
    !insertmacro UAC_RunElevated
    #MessageBox mb_TopMost "0=$0 1=$1 2=$2 3=$3"
    ${Switch} $0
    ${Case} 0
    	${IfThen} $1 = 1 ${|} Quit ${|} ;we are the outer process, the inner process has done its work, we are done
    	${IfThen} $3 <> 0 ${|} ${Break} ${|} ;we are admin, let the show go on
    	${If} $1 = 3 ;RunAs completed successfully, but with a non-admin user
    		MessageBox mb_IconExclamation|mb_TopMost|mb_SetForeground "This installer requires admin access, try again" /SD IDNO IDOK uac_tryagain IDNO 0
    	${EndIf}
    	;fall-through and die
    ${Case} 1223
    	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "${ADMIN_PRIVILIGES}"
    	Quit
    ${Case} 1062
    	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Logon service not running, aborting!"
    	Quit
    ${Default}
    	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Unable to elevate , error $0"
    	Quit
    ${EndSwitch}

    ;${UAC.I.Elevate.AdminOnly}
    ;!insertmacro UAC_RunElevated
    
    ; If user can write this, it's an Admin ;)
    ; NOTE that this registry is not being used for any logic, so we can dirty it.
    WriteRegStr  HKLM   "${PLUGIN_REGKEY_CONTEXT}"   "${PROPERTY_DESCRIPTION}" "${PRODUCT_NAME}"
    IfErrors 0 +3
    MessageBox MB_OK "You need Administrator rights to install ${PRODUCT_NAME}."
    Abort
      
FunctionEnd


; Check if current user have Administrator rights to run the uninstaller.
; If not, the uninstaller will be aborted.
Function un.CheckUserRights

    uac_tryagain:
    !insertmacro UAC_RunElevated
    #MessageBox mb_TopMost "0=$0 1=$1 2=$2 3=$3"
    ${Switch} $0
    ${Case} 0
    	${IfThen} $1 = 1 ${|} Quit ${|} ;we are the outer process, the inner process has done its work, we are done
    	${IfThen} $3 <> 0 ${|} ${Break} ${|} ;we are admin, let the show go on
    	${If} $1 = 3 ;RunAs completed successfully, but with a non-admin user
    		MessageBox mb_IconExclamation|mb_TopMost|mb_SetForeground "This installer requires admin access, try again" /SD IDNO IDOK uac_tryagain IDNO 0
    	${EndIf}
    	;fall-through and die
    ${Case} 1223
    	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "This installer requires admin privileges, aborting!"
    	Quit
    ${Case} 1062
    	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Logon service not running, aborting!"
    	Quit
    ${Default}
    	MessageBox mb_IconStop|mb_TopMost|mb_SetForeground "Unable to elevate , error $0"
    	Quit
    ${EndSwitch}


    ;${UAC.I.Elevate.AdminOnly}
    ;!insertmacro UAC_RunElevated
    
    
    ; If user can write this, it's an Admin ;)
    ; NOTE that this registry is not being used for any logic, so we can dirty it.
    WriteRegStr  HKLM   "${PLUGIN_REGKEY_CONTEXT}"   "${PROPERTY_DESCRIPTION}" "${PRODUCT_NAME} v.${PRODUCT_VERSION}"
    IfErrors 0 +3
    MessageBox MB_OK "You need Administrator rights to uninstall ${PRODUCT_NAME}."
    Abort

FunctionEnd



;
; Test if the application is already installed.
; If yes, manage the upgrade of the plugin.
;
Var tmp
Function CheckAppInstalled


       ; Check if the CUSTOMER name is the same of the one found on registry.
       ; In case of Funambol product, this value is empty.
       ; Note: the PLUGIN_REGKEY_CONTEXT must be the same also in branded clients, to make this check work!
       ReadRegStr $R8 HKLM "${PLUGIN_REGKEY_CONTEXT}" "${PROPERTY_CUSTOMER}"
       ReadRegStr $R7 HKLM "${PLUGIN_REGKEY_CONTEXT}" "${PROPERTY_SWV}"

       StrCmp $R8 "" +1 +2 ;if empty we control the swv field to check if no app installed
       StrCmp $R7 "" +2 +1
       StrCmp $R8 "${CUSTOMER}" +1 customerAbort
       
       ; Check the software version
       ReadRegStr $R0 HKLM "${PRODUCT_UNINST_KEY}" "UninstallString"
       StrCmp $R0 "" +3
       ReadRegStr $R1 HKLM "${PRODUCT_UNINST_KEY}" "DisplayVersion"         ; $R1 = installed version "x.y.z"
       Goto upgrade
       
       ; since v.10
       ; Also check the old product name "Outlook Client"
       ReadRegStr $R0 HKLM "${OLD_PRODUCT_UNINST_KEY}" "UninstallString"
       StrCmp $R0 "" done
       ReadRegStr $R1 HKLM "${OLD_PRODUCT_UNINST_KEY}" "DisplayVersion"     ; $R1 = installed version "x.y.z"
       
       
       ; ------------------- This is an UPGRADE ------------------
   upgrade:
   
       ; $9 (global) now contains the old installdir
       ; It's important because INSTDIR can be changed by the user, but we MUST delete the old files!
       StrCpy $9 $INSTDIR
   
       ;
       ; Compare installed version with current one. We suppose version string
       ; not longer than 12 chars, and in the format "x.y.z"
       ; Loop char by char and compare the version (major - minor - build).
       ;
       StrLen $R4 $R1                                                   ; $R4 = length of installed version
       StrLen $R5 ${PRODUCT_VERSION}                                    ; $R5 = length of current version
       
       Push 0
       Pop $R0                                                          ; $R0 = iterator of chars
   loop:
       StrCpy $R2 $R1 12 $R0                                            ; $R2 = installed version from offset $R0
       StrCpy $R3 ${PRODUCT_VERSION} 12 $R0                             ; $R3 = current version from offset $R0

       IntCmp $R2 $R3   0  newerVersion  olderVersion                   ; (installed v. = current v.)?

       IntOp $R0 $R0 + 1
       IntCmp $R0 $R4 sameVersion                                       ; out if one of the 2 strings has finished
       IntCmp $R0 $R5 sameVersion
       Goto loop


  ; 1. Upgrade to a newer version.
  newerVersion:
 
       ; installed version too old?
       ; NOTE: upgrades from a Funambol version < 8.0.0 is forbidden.
       StrCmp $R8 "" +1 +2                                              ; if customer avoid to check the "too old" version (for sure it isn't) and go with upgrade
       IntCmp $R1 8   0  tooOldVersion  0

       MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
                  "${PREVIOUS_VERSION_INSTALLED_UPGRADE}" \
                  IDCANCEL cancel

       StrCpy $R9 "uninstForUpgrade"     ; Cannot call now: user can cancel installation!


       ; Old installDir MUST be copied to $9 to correctly clean the old files! (see uninstForUpgrade function)
       ReadRegStr $9 HKLM "${PLUGIN_REGKEY_CONTEXT}" "${PROPERTY_PATH}"
       ;MessageBox MB_OK|MB_ICONEXCLAMATION "Installation dir in dollaro9 = $9"
       
       ; For versions < v10:
       ; If the old installDir folder ($9) is the old default one, let's keep installDir with the new naming default, otherwise use the old path ($9)
       StrCmp $9 "$PROGRAMFILES\${OLD_INSTALLDIR_CONTEXT}" +2 +1
       StrCpy $INSTDIR "$9"
       ;MessageBox MB_OK|MB_ICONEXCLAMATION "Instdir = $INSTDIR"
       
       
       ReadRegStr $tmp  HKCU "${PLUGIN_REGKEY_CONTEXT}\${PROPERTY_PICTURE_MEDIAHUB_KEY}" "${PROPERTY_MEDIAHUB_REG_KEY}"    ; Check if the mediaHub for pictures has value
       ${If} $tmp != ""
             StrCpy $showMediaHubPanel "NO"
       ${Else}
             StrCpy $showMediaHubPanel "FolderChanged"
       ${EndIf}
       
       Goto done


  ; 2. Reinstall the same version.
  sameVersion:
       MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
                  "${PROPERTY_SAME_VERSION_INSTALLED}" \
                  IDCANCEL cancel

       StrCpy $R9 "uninstForUpgrade"     ; Cannot call now: user can cancel installation!
       StrCpy $showMediaHubPanel "NO"
       Goto done


  ; 3. Downgrade to an older version -> avoid.
  olderVersion:
       MessageBox MB_OK|MB_ICONEXCLAMATION \
                  "${MORE_RECENT_VERSION_INSTALLED}"
       Abort


  ; 4. Upgrade from a version < v8 -> avoid.
  tooOldVersion:
       MessageBox MB_OK|MB_ICONEXCLAMATION \
                  "${PREVIOUS_VERSION_INSTALLED}"
       Abort

  ; 5 Customer abort
  customerAbort:
       MessageBox MB_OK \
                  "${DIFFERENT_VERSION_INSTALLED}"
       Abort
  done:
       Return
  cancel:
       Abort
FunctionEnd



; Uninstall plugin for an upgrade.
; Uregister DLLs and delete files required.
; Note: $9 (global) contains the old installdir (may be = $INSTDIR)
Function uninstForUpgrade

     ;
     StrCmp $9 "" +1 +2
     StrCpy $9 $INSTDIR
     
     ;MessageBox MB_OK|MB_ICONEXCLAMATION "Removing files from $9"

     ; Unregister DLLs.
     UnRegDLL "$9\Redemption.dll"
     UnRegDLL "$9\FunambolAddin.dll"

     ; Delete files from old installDir.
     Delete "$9\*.*"
     RMDir /r "$9"

     ; Delete application registered from System (version could be changed).
     DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
     DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"
     
     ; Addin need to be reinstalled for ALL users (replace menu/bars).
     Call resetUsersAddinState

     ;
     ; Versions < 10.
     ; TODO: better to do the following actions ONLY if $R1 is < 10.
     ; (now we're always doing it)
     ;
     DeleteRegKey HKLM "${OLD_PRODUCT_UNINST_KEY}"
     DeleteRegKey HKLM "${OLD_PRODUCT_DIR_REGKEY}"

     ; Delete old startMenu shortcuts
     SetShellVarContext all
     Delete "$SMPROGRAMS\${OLD_STARTMENU_CONTEXT}\*.*"
     RMDir  "$SMPROGRAMS\${OLD_STARTMENU_CONTEXT}"
     

     ; Delete startMenu shortcut to userguide (no more needed)
!ifndef SHOW_STARTMENU_USER_GUIDE
     SetShellVarContext all
     Delete "$SMPROGRAMS\$ICONS_GROUP\User Guide.lnk"
!endif


     ; NOTE:
     ; $R1 is the installed product version.
     ; Insert here code to manage upgrades from specific versions...
     ; (e.g. if paths of file names changed)

FunctionEnd


;
; Reset to 'installing' all users registry keys for Outlook Addin.
; Need to cycle on all users trees, as each user stores its keys under HKCU.
;
Function resetUsersAddinState

     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0
  loop:
     EnumRegKey  $R1  HKU  ""  $R0
     StrCmp $R1 "" done                                  ; empty string when finished

     WriteRegStr  HKU   "$R1\${ADDIN_REGKEY_CONTEXT}"   "State"   "installing"

     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd



;
; Install DLL into System.
;
Function installDll

      ; First, try to unregister previous dll (if any).
      UnRegDLL "$WINDIR\FunambolAddin.dll"
      UnRegDLL "$WINDIR\OutlookAddin.dll"
      UnRegDLL "$WINDIR\OutlookAddinRemover.dll"

      ; Register DLL, abort install on error.
      ClearErrors
      SetOutPath "$INSTDIR"
      RegDLL "$INSTDIR\FunambolAddin.dll"
      IfErrors errorDLL1
      
      ; Don't register Redemption.dll if OUTLOOK.exe is not installed.
      ; If Outlook is installed later, the DLL will be registered at runtime when client starts.
      ReadRegStr $R0 HKLM "${OUTLOOK_DIR_REGKEY}" ""
      StrCmp $R0 "" +3
      RegDLL "$INSTDIR\Redemption.dll"
      IfErrors errorDLL2

      Return

  errorDLL1:
      MessageBox MB_OK "Some error occurred registering FunambolAddin.dll. Installation failed."
      Goto end

  errorDLL2:
      MessageBox MB_OK "Some error occurred registering Redemption.dll. Installation failed."
      Goto end

  end:
      UnRegDLL "$INSTDIR\FunambolAddin.dll"
      UnRegDLL "$INSTDIR\Redemption.dll"
      
      Delete "$INSTDIR\*.*"
      SetOutPath "$WINDIR"
      RMDir /r "$INSTDIR"
      Quit
  
FunctionEnd


Function .onInit

      Call CheckUserRights
!insertmacro MUI_LANGDLL_DISPLAY

      Call CheckAppInstalled
!ifdef USE_OUTLOOK
      Call CheckMicrosoftApp
!endif
      Call CheckFunClientApp
      Call CheckOldFunClientApp
FunctionEnd


; --------------------------------- MAIN SECTION -----------------------------------
Section "MainSection" SEC01

      ; Uninstall previous version if necessary
      StrCmp $R9 "uninstForUpgrade"  0  +2
      Call uninstForUpgrade

      ; Check if in the instdir path there is some slash
      Push $INSTDIR
      Push "/"
      Call StrSlash
      Pop  $R0
      StrCpy $INSTDIR $R0

      ; Check if in the start menu path there is some slash
      Push $ICONS_GROUP
      Push "/"
      Call StrSlash
      Pop  $R0
      StrCpy $ICONS_GROUP $R0
  
  
      ; --- Extract files ---
      SetOutPath "$INSTDIR"
      SetOverwrite on
      File "fileset\*.*"
      
!ifdef USE_OUTLOOK
      SetOutPath "$INSTDIR\LICENSES\Redemption"
      File "fileset\LICENSES\*.*"
      File "fileset\LICENSES\Redemption\*.*"
!endif
      
      SetOutPath "$INSTDIR\docs\"
      File "fileset\docs\*.*"
      
      SetOutPath "$INSTDIR\images"
      File "fileset\images\*.*"
      
      SetOutPath "$INSTDIR"
      
      File /r "fileset\redist\*.*"


      ; --- Register dll ---
      ; If registration errors, installation fails
!ifdef USE_OUTLOOK
      Call installDll
!endif
  
      ; Delete the dll of some previous installation
      Delete "$WINDIR\FunambolAddin.dll"
      ; (These 3 could exist for dirty installation of plugins previous then 3.0 stable...)
      Delete "$WINDIR\winmainclientdll.dll"
      Delete "$WINDIR\OutlookAddin.dll"
      Delete "$WINDIR\OutlookAddinRemover.dll"

      ; Write the URL of userguide.url and website.url
!ifdef USER_GUIDE_LINK
      FileOpen $2 "$INSTDIR\userguide.url" w
      FileWrite $2 "[InternetShortcut]$\r$\nURL=${USER_GUIDE_LINK}"
      FileClose $2
!endif
      FileOpen $2 "$INSTDIR\website.url" w
      FileWrite $2 "[InternetShortcut]$\r$\nURL=${PRODUCT_WEB_SITE}"
      FileClose $2

      ; --- StartMenu shortcuts ---
      SetShellVarContext all
      !insertmacro MUI_STARTMENU_WRITE_BEGIN Application
      CreateDirectory "$SMPROGRAMS\$ICONS_GROUP"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\${PRODUCT_NAME}.lnk"               "$INSTDIR\${PRODUCT_NAME_EXE}" "" "" "" "" "" "Launch $(^Name)"
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\Uninstall.lnk"                     "$INSTDIR\uninst.exe"          "" "" "" "" "" "Uninstall $(^Name)"
!ifdef SHOW_STARTMENU_README
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\Readme.lnk"                        "$INSTDIR\Readme.txt"          "" "" "" "" "" "${PRODUCT_NAME} Readme"
!endif
!ifdef SHOW_STARTMENU_USER_GUIDE
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\User Guide.lnk"                    "$INSTDIR\userguide.url"       "" "" "" "" "" "${PRODUCT_NAME} User Guide"
!endif
!ifdef SHOW_STARTMENU_WEBSITE
      CreateShortCut  "$SMPROGRAMS\$ICONS_GROUP\${WEB_SITE_LINK_TITLE}.lnk"        "$INSTDIR\website.url"         "" "" "" "" "" "${WEB_SITE_LINK_TITLE}"
!endif
      
      !insertmacro MUI_STARTMENU_WRITE_END


      ; Write registry keys
      Call writeRegistry
      
      ; Create the MediaHub Folder only if previously set (skip during upgrades)
      ${If} $mediaHubFolder != ""
            ;MessageBox MB_ICONSTOP "$mediaHubFolder"
            Call createMediaHubFolder
            
            ; This is to use the "$DESKTOP" shell var for current user, instead of the public one
            SetShellVarContext current
            CreateShortCut  "$DESKTOP\${PROPERTY_MEDIAHUB}.lnk" "$mediaHubFolder" "" "$INSTDIR\images\MediaHubFolder.ico" "" "" "" ""
            SetShellVarContext all
      ${EndIf}

      ; check if telephony location is correctly set
      Call checkTelephonyLocation

SectionEnd



; Install application on System
Section -Post
     WriteUninstaller  "$INSTDIR\uninst.exe"
     WriteRegStr HKLM  "${PRODUCT_DIR_REGKEY}"  "" "$INSTDIR\${PRODUCT_NAME_EXE}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "DisplayName"      "$(^Name)"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "UninstallString"  "$INSTDIR\uninst.exe"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "DisplayIcon"      "$INSTDIR\${PRODUCT_NAME_EXE}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "DisplayVersion"   "${PRODUCT_VERSION}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "URLInfoAbout"     "${PRODUCT_WEB_SITE}"
     WriteRegStr HKLM  "${PRODUCT_UNINST_KEY}"  "Publisher"        "${PRODUCT_PUBLISHER}"
SectionEnd





; --------------------------------- SECTION Uninstall -----------------------------------

Section Uninstall

     ; No more delete scheduled task -> done ONLY if removing users' data.
     ; Delete "$WINDIR\Tasks\${PRODUCT_NAME}.job"

     ClearErrors
     SetOutPath "$INSTDIR"
     
     ; Unregister DLLs.
!ifdef USE_OUTLOOK
     UnRegDLL "$INSTDIR\Redemption.dll"
     IfErrors errorDLL1
     UnRegDLL "$INSTDIR\FunambolAddin.dll"
     IfErrors errorDLL2

     ; Copy Addin to WinDir and register it.
     ; (MUST keep it after uninstall, to be loaded at next Outlook startup
     ; and clean up buttons/bars - then it will unregister itself).
     CopyFiles /SILENT "$INSTDIR\FunambolAddin.dll" "$WINDIR\FunambolAddin.dll"

     CopyFiles /SILENT "$INSTDIR\${MSLIB_ATL}" "$WINDIR"          ; MUST copy also ATL library to WINDIR! (could not be installed).
     CopyFiles /SILENT "$INSTDIR\${MSLIB_MFC}" "$WINDIR"          ; this should not be necessary, but...
     CopyFiles /SILENT "$INSTDIR\${MSLIB_CRT}" "$WINDIR"          ; CRT is also required!
     
     RegDLL "$WINDIR\FunambolAddin.dll"
     IfErrors errorDLL3
!endif

     ; Delete files from installDir.
     Delete "$INSTDIR\*.*"

     ; Delete startMenu shortcuts
     SetShellVarContext all
     !insertmacro MUI_STARTMENU_GETFOLDER "Application" $ICONS_GROUP
     Delete "$SMPROGRAMS\$ICONS_GROUP\*.*"
     Delete  "$DESKTOP\${PRODUCT_NAME}.lnk"
     
     SetShellVarContext current
     Delete  "$DESKTOP\${PROPERTY_MEDIAHUB}.lnk"
     SetShellVarContext all
     
     ; Delete recursively empty folders created on install.
     StrCpy $R4 "$INSTDIR"
     ;MessageBox MB_OK|MB_ICONEXCLAMATION "Removing dir: $R4"
     RMDir /r "$R4"
     
  loop1:
     Push "$R4"
     Call un.GetLastStrPart
     Pop $R0
     StrCmp $R4 $R0 +5
     StrCmp "$PROGRAMFILES" $R0 +4
     StrCpy $R4 $R0
     RMDir "$R0"
     Goto loop1

     ; Delete recursively empty folders in start menu created on install.
     RMDir "$SMPROGRAMS\$ICONS_GROUP"
     StrCpy $R4 "$ICONS_GROUP"
  loop:
     Push "$R4"
     Call un.GetLastStrPart
     Pop $R0
     StrCmp $R4 $R0 +4
     StrCpy $R4 $R0
     RMDir "$SMPROGRAMS\$R0"
     Goto loop

     
     ; Delete registry keys
     Call un.deleteRegistry
     
     SetAutoClose true
     Return
     
  errorDLL1:
      MessageBox MB_OK "Could not unregister Redemption.dll. Uninstallation failed."
      Return

  errorDLL2:
      MessageBox MB_OK "Could not unregister FunambolAddin.dll. Uninstallation failed."
      Return

  errorDLL3:
      MessageBox MB_OK "Could not register FunambolAddin.dll. Uninstallation failed."
      Return
     
SectionEnd



Function un.onInit


!insertmacro MUI_UNGETLANGUAGE

     Call un.CheckUserRights
     
     ;!insertmacro MUI_INSTALLOPTIONS_EXTRACT "removeData.ini"
     ;MessageBox MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2 "Are you sure to remove $(^Name) and all its components?" IDYES +2
     ;Abort

!ifdef USE_OUTLOOK
     Call un.CheckMicrosoftApp
!endif
     Call un.CheckFunClientApp

FunctionEnd



Function un.onUninstSuccess
     HideWindow
     MessageBox MB_ICONINFORMATION|MB_OK "${PROGRAM_REMOVED}"
FunctionEnd

Var Chbox
Var Text
Var Checkbox_State
Function un.RemoveUserData

        nsDialogs::Create 1018
        Pop $Dialog

        ${If} $Dialog == error
		Abort
	${EndIf}


        ;!insertmacro MUI_HEADER_TEXT "Delete local settings" "Delete all local synchronization files and settings for all users."
        !insertmacro MUI_HEADER_TEXT "${DELETE_LOCAL_SETTINGS_TITLE}" "${DELETE_LOCAL_SETTINGS_BODY}"

        ${NSD_CreateCheckBox} 0 0 100% 25u "${CHECKBOX_REMOVE_SETTINGS}"
	Pop $Chbox

        ${NSD_Check} $Chbox
        
	;${If} $Checkbox_State == ${BST_CHECKED}
	;	${NSD_Check} $Chbox
	;${EndIf}
	
        nsDialogs::Show


FunctionEnd

Function un.nsDialogsPageLeaveCheckbox

	${NSD_GetState} $Chbox $Checkbox_State
	${If} $Checkbox_State == ${BST_CHECKED}
	
	  Call un.deleteUsersMediaHubDesktopIni
          Call un.deleteUsersFiles
          Call un.deleteUsersRegistry

          ; Delete scheduled task for all users.
          ; Tasks have the name: "Funambol Windows Client - <Username>.job"
          ; where 'Username' is the Windows current user that created it.
          Delete "$WINDIR\Tasks\${PRODUCT_NAME}*.job"

        ${EndIf}

FunctionEnd




; Display a custom page during uninstaller, to ask the user if deleting files/settings.
;Function un.RemoveUserData
;
 ;    !insertmacro MUI_HEADER_TEXT "Delete local settings" "Delete all local synchronization files and settings for all users."
 ;    !insertmacro MUI_INSTALLOPTIONS_DISPLAY_RETURN "removeData.ini"
;
;     Pop $R0      ; This is the return value: "success", "cancel", "back" or "error"
;     ;MessageBox MB_ICONINFORMATION|MB_OK "ret value = $R0"
;     StrCmp $R0 "success"  0 done
;
;     ; Read the 'State' value of checkbox: 0 = not selected.
;     ReadINIStr $R2 "$PLUGINSDIR\removeData.ini" "Field 1" "State"
;     IntCmp $R2 0 done
;
;     Call un.deleteUsersMediaHubDesktopIni
;     Call un.deleteUsersFiles
;     Call un.deleteUsersRegistry
;
;     ; Delete scheduled task for all users.
;     ; Tasks have the name: "Funambol Windows Client - <Username>.job"
;     ; where 'Username' is the Windows current user that created it.
;     Delete "$WINDIR\Tasks\${PRODUCT_NAME}*.job"
;
;  done:
;FunctionEnd



; Utility to get the last part of a string using '\' as the separator
Function un.GetLastStrPart
     Exch $R0
     Push $R1
     Push $R2
     StrLen $R1 $R0
     IntOp $R1 $R1 + 1
     StrCpy $R3 1
  loop:
     IntOp $R1 $R1 - 1
     StrCpy $R2 $R0 1 -$R3
     IntOp $R3 $R3 + 1
     StrCmp $R2 "" exit2
     StrCmp $R2 "\" exit1
     Goto loop
  exit1:
     IntOp $R3 $R3 - 1
     StrCpy $R0 $R0 -$R3
  exit2:
     Pop $R2
     Pop $R1
     Exch $R0
FunctionEnd


; Utility to transform a string: replace all '/' into '\'
Function StrSlash
     Exch $R3 ; $R3 = needle ("\" or "/")
     Exch
     Exch $R1 ; $R1 = String to replacement in (haystack)
     Push $R2 ; Replaced haystack
     Push $R4 ; $R4 = not $R3 ("/" or "\")
     Push $R6
     Push $R7 ; Scratch reg
     StrCpy $R2 ""
     StrLen $R6 $R1
     StrCpy $R4 "\"
     StrCmp $R3 "/" loop
     StrCpy $R4 "/"
  loop:
     StrCpy $R7 $R1 1
     StrCpy $R1 $R1 $R6 1
     StrCmp $R7 $R3 found
     StrCpy $R2 "$R2$R7"
     StrCmp $R1 "" done loop
found:
     StrCpy $R2 "$R2$R4"
     StrCmp $R1 "" done loop
done:
     StrCpy $R3 $R2
     Pop $R7
     Pop $R6
     Pop $R4
     Pop $R2
     Pop $R1
     Exch $R3
FunctionEnd


;
; Write registry keys.
; All client keys for synchronization are automatically created the first time
; by plugin at startup.
;
Function writeRegistry

!ifdef USE_OUTLOOK
     ; Register as program that uses Microsoft Outlook
     WriteRegStr   HKLM     "${MSMAPIAPPS_REGKEY_CONTEXT}"       "${PRODUCT_NAME_EXE}"           "${MICROSOFT_OUTLOOK}"

     ; Installation path and info used by Addin.
     ;(others are generated by DLL registration, see addin.rgs)
     WriteRegStr  HKLM      "${ADDIN_REGKEY_CONTEXT}"            "${PROPERTY_PATH}"              "$INSTDIR"
     WriteRegStr  HKLM      "${ADDIN_REGKEY_CONTEXT}"            "${PROPERTY_ADDIN_NAME}"        "FunambolAddin.dll"
!endif
     
     ; funambol_swv = Funambol Software version: it's important for the upgrade process!
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_SWV}"               "${PRODUCT_VERSION}"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_PATH}"              "$INSTDIR"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_DESCRIPTION}"       "${PRODUCT_NAME} v.${PRODUCT_VERSION}"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_CUSTOMER}"          "${CUSTOMER}"
     WriteRegStr  HKLM      "${PLUGIN_REGKEY_CONTEXT}"           "${PROPERTY_FUNAMBOL_SWV}"      "${FUNAMBOL_SWV}"

FunctionEnd


;
; Delete registry keys.
;
Function un.deleteRegistry

     ; Delete HKLM keys (values set on install)
     DeleteRegKey HKLM "${PLUGIN_REGKEY_CONTEXT}"

     ; Delete application registered from System
     DeleteRegKey HKLM "${PRODUCT_UNINST_KEY}"
     DeleteRegKey HKLM "${PRODUCT_DIR_REGKEY}"

FunctionEnd


;
; Delete all users registry keys.
; Need to cycle on all users trees, as each user stores its keys under HKCU.
; Keys are deleted ONLY if user selected it from uninstaller RemoveUsersData page.
;
Function un.deleteUsersRegistry

     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0
  loop:
     EnumRegKey  $R1  HKU  ""  $R0
     StrCmp $R1 "" done                                  ; empty string when finished

     DeleteRegKey  HKU  "$R1\${PLUGIN_REGKEY_CONTEXT}"   ; Delete users keys (concatenate strings) - plugin user's settings.
     ; Note: addin user's settings are NOT removed! The addin still need to be loaded to cleanup commandbar & menu.

     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd


;
; Delete all users files from 'Application Data' folder.
; Need to cycle on all users doc&settings folders.
; Files are deleted ONLY if user selected it from uninstaller RemoveUsersData page.
;
Function un.deleteUsersFiles

     ; Get 'Application Data' paths from each user's registry keys.
     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0                                                        ; $R0 = counter for users on this machine.
  loop:
     EnumRegKey  $R1  HKU  ""  $R0                                  ; $R1 = name of this user.
     StrCmp $R1 "" done                                             ; (empty string when finished -> done)

     ReadRegStr $R2  HKU "$R1\${SHELLFOLDERS_CONTEXT}" "AppData"    ; $R2 = path of 'Application Data' folder for this user.
     StrCmp $R2 "" next                                             ; (not found -> next user)
     StrCpy $R3 "$R2\${DATAFILES_CONTEXT}"                          ; $R3 = path of local files.
     Delete "$R3\*.*"
     RMDir /r "$R3"
     
     ; Delete recursively empty folders under 'Application Data'.
     loop1:
         Push "$R3"
         Call un.GetLastStrPart
         Pop $R4                                                    ; $R4 = path of parent folder.
         StrCmp $R3 $R4  next
         StrCmp $R2 $R4  next                                       ; (reached Application data folder -> next user)

         StrCpy $R3 $R4
         RMDir "$R4"
         Goto loop1

  next:
     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd

Function un.deleteUsersMediaHubDesktopIni

     ; Loop on each entry under HKU (user name).
     Push 1
     Pop $R0
  loop:
     EnumRegKey  $R1  HKU  ""  $R0
     StrCmp $R1 "" done                                  ; empty string when finished

     ReadRegStr $tmp  HKU "$R1\${PLUGIN_REGKEY_CONTEXT}\${PROPERTY_PICTURE_MEDIAHUB_KEY}" "${PROPERTY_MEDIAHUB_REG_KEY}"    ; Check if the mediaHub for pictures has value
     ${If} $tmp != ""
         Delete "$tmp\Desktop.ini"
     ${EndIf}
     
     IntOp $R0 $R0 + 1
     Goto loop

  done:
FunctionEnd

;
; Checks if there is at least one Telephony Location defined in the system.
; If not, we define a default one (US locale = area code 1).
;
; This is necessary on WinXP otherwise the phone numbers may lose the trailing "+"
; when they're added in Outlook using Outlook APIs (bug 6218), causing sync issues.
; It is safe: if a location is already defined, it does nothing.
;
Function checkTelephonyLocation

     EnumRegKey  $R1  HKLM  "${TELEPHONY_LOCATIONS_CONTEXT}"  0

     ; empty string means "no subkey is found", so we need to create one
     StrCmp $R1 "" 0 done
     
     WriteRegDWORD  HKLM  "${TELEPHONY_LOCATIONS_CONTEXT}"            "CurrentID"  1
     WriteRegDWORD  HKLM  "${TELEPHONY_LOCATIONS_CONTEXT}"            "NextID"     2
     
     WriteRegStr    HKLM  "${TELEPHONY_LOCATIONS_CONTEXT}\Location1"  "Name"       "Location"
     WriteRegStr    HKLM  "${TELEPHONY_LOCATIONS_CONTEXT}\Location1"  "AreaCode"   "1"
     WriteRegDWORD  HKLM  "${TELEPHONY_LOCATIONS_CONTEXT}\Location1"  "Country"    1
     
  done:
FunctionEnd



