/*
 * Funambol is a mobile platform developed by Funambol, Inc.
 * Copyright (C) 2003 - 2009 Funambol, Inc.
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
#include "VideosSettings.h"
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
static int getVideosSyncTypeID(const char* syncType) {

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


IMPLEMENT_DYNCREATE(CVideosSettings, CDialog)

CVideosSettings::CVideosSettings() : CDialog(CVideosSettings::IDD) {

    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(VIDEO_);
    if (!ssconf) {
        printLog("Config not found for source videos!", LOG_ERROR);
    }
}

CVideosSettings::~CVideosSettings() {}

void CVideosSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_VIDEOS_COMBO_SYNCTYPE,  lstSyncType);
    DDX_Control(pDX, IDC_VIDEOS_EDIT_SYNCTYPE,   editSyncType);
    DDX_Control(pDX, IDC_VIDEOS_EDIT_FOLDER,     editFolder);
    DDX_Control(pDX, IDC_VIDEOS_BUT_SELECT,      butSelectFolder);    
    DDX_Control(pDX, IDC_VIDEOS_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_VIDEOS_GROUP_FOLDER,    groupFolder);
   
}

BEGIN_MESSAGE_MAP(CVideosSettings, CDialog)
    ON_BN_CLICKED(IDC_VIDEOS_OK,         &CVideosSettings::OnBnClickedVideosOk)
    ON_BN_CLICKED(IDC_VIDEOS_CANCEL,     &CVideosSettings::OnBnClickedVideosCancel)
    ON_BN_CLICKED(IDC_VIDEOS_BUT_SELECT, &CVideosSettings::OnBnClickedVideosButSelect)
    ON_CBN_SELCHANGE(IDC_VIDEOS_COMBO_SYNCTYPE, &CVideosSettings::OnCbnSelchangeVideosComboSynctype)
END_MESSAGE_MAP()


#ifdef _DEBUG
void CVideosSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CVideosSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG



BOOL CVideosSettings::OnInitDialog() {

    if (!ssconf) return FALSE;

    bool showAdvanced = true;
    CString s1;
    s1.LoadString(IDS_VIDEOS_DETAILS);
    SetWindowText(s1);
    CDialog::OnInitDialog();

    editSyncType.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editFolder.SetLimitText  (EDIT_TEXT_MAXLENGTH);    

    // Load the syncmodes in the editbox/dropdown
    loadSyncModesBox(VIDEO_);

    // load string resources
    s1.LoadString(IDS_SYNC_DIRECTION);      SetDlgItemText(IDC_VIDEOS_GROUP_DIRECTION,    s1);
    s1.LoadString(IDS_VIDEOS_FOLDER);       SetDlgItemText(IDC_VIDEOS_GROUP_FOLDER,       s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_VIDEOS_BUT_SELECT,         s1);
    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_VIDEOS_OK,                 s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_VIDEOS_CANCEL,             s1);

    // Sync type
    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
    OnCbnSelchangeVideosComboSynctype();

    // Sync type
    int id = getVideosSyncTypeID(ssconf->getSync());
    
    s1.LoadString(id);
    SetDlgItemText(IDC_VIDEOS_EDIT_SYNCTYPE, s1);

    // Videos folder path
    StringBuffer path = ssconf->getProperty(PROPERTY_MEDIAHUB_PATH);
    if (path.empty()) {
        // If empty, set the default path for videos (shell folder)
        path = getDefaultMyDocumentsPath();
        path.append("\\");
        path.append(MEDIA_HUB_DEFAULT_FOLDER);        
        ssconf->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());
    }
    WCHAR* wpath = toWideChar(path.c_str());
    s1 = wpath;
    delete [] wpath;
    SetDlgItemText(IDC_VIDEOS_EDIT_FOLDER, s1);
    
    

    butSelectFolder.EnableWindow(FALSE);


    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if (((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupDirection.m_hWnd, L" ", L" ");
        pfnSetWindowTheme (groupFolder.m_hWnd,    L" ", L" ");
        
    }
    GetDlgItem(IDC_VIDEOS_OK)->SetFocus();
    
    return FALSE;
}


void CVideosSettings::OnBnClickedVideosOk()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    }
}

void CVideosSettings::OnBnClickedVideosCancel()
{
    // will save when 'OK' is clicked on SyncSettings.
    CDialog::OnCancel();
}

bool CVideosSettings::saveSettings(bool saveToDisk) {

    if (!ssconf) return FALSE;

    CString videosPath;
    CString s1;
    _bstr_t bst;

    GetDlgItemText(IDC_VIDEOS_EDIT_FOLDER, videosPath);

    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* path = toMultibyte(videosPath.GetBuffer());
    if (path) {
        ssconf->setProperty(PROPERTY_MEDIAHUB_PATH, path);
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


void CVideosSettings::OnBnClickedVideosButSelect() {

    if (!ssconf) return;

    // Get the default browse folder to the current path of videos
    StringBuffer path = ssconf->getProperty(PROPERTY_MEDIAHUB_PATH);
    WCHAR* defaultPath = toWideChar(path.c_str());

    CString caption;
    caption.LoadString(IDS_SELECT_VIDEOS_FOLDER);

    // Open the browse for folder window (modal)
    wstring newPath;
    if ( browseFolder(newPath, defaultPath, caption.GetBuffer(), GetSafeHwnd()) ) {
        // Update the UI label and save the new path
        SetDlgItemText(IDC_VIDEOS_EDIT_FOLDER, newPath.c_str());
        path.convert(newPath.c_str());
        ssconf->setProperty(PROPERTY_MEDIAHUB_PATH, path.c_str());
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

bool CVideosSettings::browseFolder(wstring& folderpath, const WCHAR* defaultFolder, const WCHAR* szCaption, const HWND hOwner) {

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

void CVideosSettings::loadSyncModesBox(const char* sourceName)
{
    OutlookConfig* config = getConfig();
    SyncSourceConfig* ssconf = config->getSyncSourceConfig(sourceName);
    if (!ssconf) return;

    // TODO: use a switch on sourceName when refactoring
    int editBoxResourceID = IDC_VIDEOS_EDIT_SYNCTYPE;
    int comboBoxResourceID = IDC_VIDEOS_COMBO_SYNCTYPE;

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
        SetDlgItemText(editBoxResourceID, s1);
    }
}

void CVideosSettings::OnCbnSelchangeVideosComboSynctype()
{
    // Supported data format
    StringBuffer supportedData;
    
    CString ss(" "), ss1;        
    ss1.LoadString(IDS_SUPPORTED_FORMAT);
    ss.Append(ss1);
    
    CString And;
    And.LoadString(IDS_STRING_AND);
    StringBuffer and(" ");
    and.append(ConvertToChar(And));
    and.append(" ");
       
    
    StringBuffer data = ssconf->getProperty(PROPERTY_EXTENSION);    
    if (data.empty() == false) {
       
        supportedData = ConvertToChar(ss);
        
        StringBuffer data = ssconf->getProperty(PROPERTY_EXTENSION);    
        data.upperCase();
        supportedData.append(data);
        
        int val = supportedData.rfind(",.");
        if (val != StringBuffer::npos) {
            supportedData.replace(",.", and.c_str(), val);
        }
        supportedData.replaceAll(",.",", ");
        supportedData.replaceAll(".","");    
        supportedData.append(".");
    }
   
    /*
    CString s2;
    s2.LoadString(IDS_MEDIA_HUB_VIDEO_MAX_SIZE);
    StringBuffer s, sss;    
    s = ConvertToChar(s2);
    sss.sprintf(s.c_str(), (int)SAPI_MAX_VIDEO_SIZE/1024/1024);
    supportedData.append(" ");
    supportedData.append(sss);
    */
    CString s2;
    s2.LoadString(IDS_MEDIA_HUB_VIDEO_MAX_SIZE);
    WCHAR tmp[1024];
    wsprintf(tmp, s2.GetBuffer(), (int)SAPI_MAX_VIDEO_SIZE/1024/1024);
    wstring w1 = tmp;
    WCHAR* tmp2 = toWideChar(supportedData.c_str());
    wstring w2 = tmp2;
    delete [] tmp2;

    w2.append(L" ");
    w2.append(w1);

    CString suppData(w2.c_str()); //supportedData;

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
            s1.LoadString(IDS_TWO_WAY_LABEL_VIDEO_SUMMARY);
            s1.Append(suppData);
            SetDlgItemText(IDC_VIDEOS_SYNC_DIRECTION_LABEL, s1);
            break;
        case 1:
            s1.LoadString(IDS_DOWNLOAD_ONLY_LABEL_VIDEO_SUMMARY);        
            s1.Append(suppData);
            SetDlgItemText(IDC_VIDEOS_SYNC_DIRECTION_LABEL, s1);
            break;
        case 2:
            s1.LoadString(IDS_UPLOAD_ONLY_LABEL_VIDEO_SUMMARY);
            s1.Append(suppData);
            SetDlgItemText(IDC_VIDEOS_SYNC_DIRECTION_LABEL, s1);
            break;
    }
}
