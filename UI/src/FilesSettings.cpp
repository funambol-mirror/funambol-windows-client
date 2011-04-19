/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2010 Funambol, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License version 3 as published by
 * the Free Software Foundation with the addition of the following permission 
 * added to Section 15 as permitted in Section 7(a): FOR ANY PART OF THE COVERED
 * WORK IN WHICH THE COPYRIGHT IS OWNED BY FUNAMBOL, FUNAMBOL DISCLAIMS THE 
 * WARRANTY OF NON INFRINGEMENT  OF THIRD PARTY RIGHTS.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Affero General Public License 
 * along with this program; if not, see http://www.gnu.org/licenses or write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 * 
 * You can contact Funambol, Inc. headquarters at 643 Bair Island Road, Suite 
 * 305, Redwood City, CA 94063, USA, or at email address info@funambol.com.
 * 
 * The interactive user interfaces in modified source and object code versions
 * of this program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU Affero General Public License version 3.
 * 
 * In accordance with Section 7(b) of the GNU Affero General Public License
 * version 3, these Appropriate Legal Notices must retain the display of the
 * "Powered by Funambol" logo. If the display of the logo is not reasonably 
 * feasible for technical reasons, the Appropriate Legal Notices must display
 * the words "Powered by Funambol".
 */


#include "stdafx.h"
#include "resource.h"
#include "FilesSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"

#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"

#include "shlobj.h"     // to browse for folder

using namespace std;

// used to exchange the default browse folder (between BrowseCallbackProc and browseFolder)
static wstring defaultBrowseFolder;

/**
 * Returns the resource ID of the sync-direction text to show, given the synctype.
 */
static int getFilesSyncTypeID(const char* syncType) {

    int ret = IDS_SYNCTYPE2;    // default

    if (!strcmp(syncType, SYNC_MODE_TWO_WAY)) {
        ret = IDS_SYNCTYPE1;
    }
    else if (!strcmp(syncType, SYNC_MODE_ONE_WAY_FROM_SERVER) ||
        !strcmp(syncType, SYNC_MODE_SMART_ONE_WAY_FROM_SERVER)) {
        ret = IDS_SYNCTYPE2;
    }
    else if (!strcmp(syncType, SYNC_MODE_ONE_WAY_FROM_CLIENT) ||
             !strcmp(syncType, SYNC_MODE_SMART_ONE_WAY_FROM_CLIENT)) {
        ret = IDS_SYNCTYPE3;
    }
    return ret;
}


IMPLEMENT_DYNCREATE(CFilesSettings, CDialog)

CFilesSettings::CFilesSettings() : CDialog(CFilesSettings::IDD) {

    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(FILES_);
    if (!ssconf) {
        printLog("Config not found for source files!", LOG_ERROR);
    }
}

CFilesSettings::~CFilesSettings() {}

void CFilesSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_FILES_COMBO_SYNCTYPE,  lstSyncType);
    DDX_Control(pDX, IDC_FILES_EDIT_SYNCTYPE,   editSyncType);
    DDX_Control(pDX, IDC_FILES_EDIT_FOLDER,     editFolder);
    DDX_Control(pDX, IDC_FILES_BUT_SELECT,      butSelectFolder);    
    DDX_Control(pDX, IDC_FILES_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_FILES_GROUP_FOLDER,    groupFolder);
   
}

BEGIN_MESSAGE_MAP(CFilesSettings, CDialog)
    ON_BN_CLICKED(IDC_FILES_OK,         &CFilesSettings::OnBnClickedFilesOk)
    ON_BN_CLICKED(IDC_FILES_CANCEL,     &CFilesSettings::OnBnClickedFilesCancel)
    ON_BN_CLICKED(IDC_FILES_BUT_SELECT, &CFilesSettings::OnBnClickedFilesButSelect)
    ON_CBN_SELCHANGE(IDC_FILES_COMBO_SYNCTYPE, &CFilesSettings::OnCbnSelchangeFilesComboSynctype)
END_MESSAGE_MAP()


#ifdef _DEBUG
void CFilesSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CFilesSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG



BOOL CFilesSettings::OnInitDialog() {

    if (!ssconf) return FALSE;

    bool showAdvanced = true;
    CString s1;
    s1.LoadString(IDS_FILES_DETAILS);
    SetWindowText(s1);
    CDialog::OnInitDialog();

    editSyncType.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editFolder.SetLimitText  (EDIT_TEXT_MAXLENGTH);    
        
    // Load the syncmodes in the editbox/dropdown
    loadSyncModesBox(FILES_);

    // load string resources
    s1.LoadString(IDS_SYNC_DIRECTION);      SetDlgItemText(IDC_FILES_GROUP_DIRECTION,    s1);
    s1.LoadString(IDS_FILES_FOLDER);        SetDlgItemText(IDC_FILES_GROUP_FOLDER,       s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_FILES_BUT_SELECT,         s1);    
    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_FILES_OK,                 s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_FILES_CANCEL,             s1);

     // Sync type
    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
    OnCbnSelchangeFilesComboSynctype();

    // Sync type
    //int id = getFilesSyncTypeID(ssconf->getSync());
    //s1.LoadString(id);
    //SetDlgItemText(IDC_FILES_EDIT_SYNCTYPE, s1);

    // Files folder path
    StringBuffer path = ssconf->getCommonConfig()->getProperty(PROPERTY_MEDIAHUB_PATH);
    
    if (path.empty()) {
        // If empty, set the default path for files (shell folder)
       // If empty, set the default path for pictures (shell folder)
        path = getDefaultMyDocumentsPath();
        path.append("\\");
        path.append(MEDIA_HUB_DEFAULT_FOLDER);        
        ssconf->getCommonConfig()->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());
    }
    WCHAR* wpath = toWideChar(path.c_str());
    s1 = wpath;
    delete [] wpath;
    SetDlgItemText(IDC_FILES_EDIT_FOLDER, s1);
       

    
    butSelectFolder.EnableWindow(FALSE);
    
    

    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if (((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupDirection.m_hWnd, L" ", L" ");
        pfnSetWindowTheme (groupFolder.m_hWnd,    L" ", L" ");
        
    }
    GetDlgItem(IDC_FILES_OK)->SetFocus();
    
    return FALSE;
}


void CFilesSettings::OnBnClickedFilesOk()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    }
}

void CFilesSettings::OnBnClickedFilesCancel()
{
    // will save when 'OK' is clicked on SyncSettings.
    CDialog::OnCancel();
}

bool CFilesSettings::saveSettings(bool saveToDisk) {

    if (!ssconf) return FALSE;

    CString filesPath;
    CString s1;
    _bstr_t bst;

    GetDlgItemText(IDC_FILES_EDIT_FOLDER, filesPath);
   

    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* path = toMultibyte(filesPath.GetBuffer());
    if (path) {
        ssconf->getCommonConfig()->setProperty(PROPERTY_MEDIAHUB_PATH, path);
        delete [] path;    
    }    

    // Never save to winreg, will save when 'OK' is clicked on SyncSettings.
    //if(saveToDisk)
    //    ((OutlookConfig*)getConfig())->save();
    if (lstSyncType.IsWindowVisible()) {
        ssconf->setSync(getSyncTypeName(lstSyncType.GetCurSel()));
    }
    return true;
}


void CFilesSettings::OnBnClickedFilesButSelect() {

    if (!ssconf) return;

    // Get the default browse folder to the current path of files
    StringBuffer path = ssconf->getCommonConfig()->getProperty(PROPERTY_MEDIAHUB_PATH);
    WCHAR* defaultPath = toWideChar(path.c_str());

    CString caption;
    caption.LoadString(IDS_SELECT_FILES_FOLDER);
    
    // Open the browse for folder window (modal)
    wstring newPath;
    if ( browseFolder(newPath, defaultPath, caption.GetBuffer(), GetSafeHwnd()) ) {
        // Update the UI label and save the new path
        SetDlgItemText(IDC_FILES_EDIT_FOLDER, newPath.c_str());
        path.convert(newPath.c_str());
        ssconf->getCommonConfig()->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());
    }

    delete [] defaultPath;
}


/// Callback fuction for the 'browse for folder' window. Sets the default folder.
static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {

    if (uMsg == BFFM_INITIALIZED) {
        SendMessage(hwnd, BFFM_SETSELECTION, TRUE, (LPARAM)(defaultBrowseFolder.c_str()));
    }
    return 0;
}

bool CFilesSettings::browseFolder(wstring& folderpath, const WCHAR* defaultFolder, const WCHAR* szCaption, const HWND hOwner) {

    bool retVal = false;

    // The BROWSEINFO struct tells the shell how it should display the dialog.
    BROWSEINFO bi;
    memset(&bi, 0, sizeof(bi));

    bi.ulFlags   = BIF_USENEWUI | BIF_VALIDATE;
    bi.hwndOwner = hOwner;
    bi.lpszTitle = szCaption;

    if (defaultFolder) {
        // The callback function will set this folder as the default one for browse
        defaultBrowseFolder = defaultFolder;
        bi.lpfn = BrowseCallbackProc;
    }

    // must call this if using BIF_USENEWUI
    OleInitialize(NULL);

    // Show the dialog and get the itemIDList for the selected folder.
    LPITEMIDLIST pIDL = SHBrowseForFolder(&bi);

    if (pIDL) {
        // Create a buffer to store the path, then get the path.
        WCHAR buffer[_MAX_PATH] = {'\0'};

        if (SHGetPathFromIDList(pIDL, buffer) != 0) {
            folderpath = buffer;
            retVal = true;
        }
        // free the item id list
        CoTaskMemFree(pIDL);
    }

    OleUninitialize();
    return retVal;
}

void CFilesSettings::loadSyncModesBox(const char* sourceName)
{
    OutlookConfig* config = getConfig();
    WindowsSyncSourceConfig* ssconf = config->getSyncSourceConfig(sourceName);
    if (!ssconf) return;

    // TODO: use a switch on sourceName when refactoring
    int editBoxResourceID = IDC_FILES_EDIT_SYNCTYPE;
    int comboBoxResourceID = IDC_FILES_COMBO_SYNCTYPE;

    CEdit* editbox = (CEdit*)GetDlgItem(editBoxResourceID);
    CComboBox* combobox = (CComboBox*)GetDlgItem(comboBoxResourceID);
    if (!combobox || !editbox) return;

    //
    // Load the syncmodes in the editbox/dropdown
    //
    CString s1 = "";
    StringBuffer syncModes(ssconf->getSyncModes());
    if (syncModes.find(SYNC_MODE_TWO_WAY) != StringBuffer::npos) {
        s1.LoadString(IDS_SYNCTYPE1);
        combobox->AddString(s1);
    }
    if (syncModes.find(SYNC_MODE_ONE_WAY_FROM_SERVER) != StringBuffer::npos ||
        syncModes.find(SYNC_MODE_SMART_ONE_WAY_FROM_SERVER) != StringBuffer::npos) {
        s1.LoadString(IDS_SYNCTYPE2);
        combobox->AddString(s1);
    }
    if (syncModes.find(SYNC_MODE_ONE_WAY_FROM_CLIENT) != StringBuffer::npos ||
        syncModes.find(SYNC_MODE_SMART_ONE_WAY_FROM_CLIENT) != StringBuffer::npos) {
        s1.LoadString(IDS_SYNCTYPE3);
        combobox->AddString(s1);
    }

    if (combobox->GetCount() > 1) {
        // More than 1 syncmode available: use the dropdown box
        editbox->ShowWindow(SW_HIDE);
        combobox->ShowWindow(SW_SHOW);
    }
    else {
        // Only 1 syncmode available: use the editbox
        editbox->ShowWindow(SW_SHOW);
        combobox->ShowWindow(SW_HIDE);
        s1.Insert(0, L" "); // just add a space to avoid to be too close to the border
        SetDlgItemText(editBoxResourceID, s1);
    }
}

void CFilesSettings::OnCbnSelchangeFilesComboSynctype()
{
    // Supported data format
    StringBuffer supportedData;
    /*
    CString ss(" "), ss1;        
    ss1.LoadString(IDS_SUPPORTED_FORMAT);
    ss.Append(ss1);
    
    CString And;
    And.LoadString(IDS_STRING_AND);
    StringBuffer and(" ");
    and.append(ConvertToChar(And));
    and.append(" ");
       
    
    StringBuffer data = ssconf->getCommonConfig()->getProperty(PROPERTY_EXTENSION);    
    if (data.empty() == false) {
       
        supportedData = ConvertToChar(ss);
        
        StringBuffer data = ssconf->getCommonConfig()->getProperty(PROPERTY_EXTENSION);    
        data.upperCase();
        supportedData.append(data);
        
        int val = supportedData.rfind(",.");
        if (val != StringBuffer::npos) {
            supportedData.replace(",.", and.c_str(), val);
        }
        supportedData.replaceAll(",.",", ");
        supportedData.replaceAll(".","");    
        
    }
    */

    CString s2;
    s2.LoadString(IDS_MEDIA_HUB_FILE_MAX_SIZE);
    StringBuffer s, sss;    
    s = ConvertToChar(s2);
    sss.sprintf(s.c_str(), (int)SAPI_MAX_FILE_SIZE/1024/1024);
    supportedData.append(" ");
    supportedData.append(sss);

    CString suppData = supportedData;

    int index = 0;
    if (lstSyncType.GetCount() > 1) {
        index = lstSyncType.GetCurSel();
    } else {
        // Fixed, 1 synctype only, get from config.
        index = getSyncTypeIndex(ssconf->getSync());
    }

    CString s1;
    switch (index) {
        case 0:
            s1.LoadString(IDS_TWO_WAY_LABEL_FILE_SUMMARY);
            s1.Append(suppData);
            SetDlgItemText(IDC_FILES_SYNC_DIRECTION_LABEL, s1);
            break;
        case 1:
            s1.LoadString(IDS_DOWNLOAD_ONLY_LABEL_FILE_SUMMARY);        
            s1.Append(suppData);
            SetDlgItemText(IDC_FILES_SYNC_DIRECTION_LABEL, s1);
            break;
        case 2:
            s1.LoadString(IDS_UPLOAD_ONLY_LABEL_FILE_SUMMARY);
            s1.Append(suppData);
            SetDlgItemText(IDC_FILES_SYNC_DIRECTION_LABEL, s1);
            break;
    }
}