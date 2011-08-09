/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2003 - 2011 Funambol, Inc.
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
#include "PIMSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"
#include "DateFilter.h"
#include "SettingsHelper.h"

#include "winmaincpp.h"
#include "utils.h"
#include "comutil.h"
#include "OutlookPlugin.h"
#include "UICustomization.h"

#include <string>

using namespace std;

static wstring outlookFolder;
static CPIMSettings* wndPIM;
static HANDLE handleThread;

IMPLEMENT_DYNCREATE(CPIMSettings, CDialog)

CPIMSettings::CPIMSettings(const int sourceType) : CDialog(CPIMSettings::IDD)
{
    type = sourceType;
    ssconf = NULL;
    handleThread = NULL;
}

CPIMSettings::~CPIMSettings()
{
    // clean stuff used in the select Outlook folder thread
    wndPIM = NULL;
    if (handleThread) {
        TerminateThread(handleThread, -1);
        CloseHandle(handleThread);
        handleThread = NULL;
    }
}

void CPIMSettings::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PIM_COMBO_SYNCTYPE,  lstSyncType);
    DDX_Control(pDX, IDC_PIM_EDIT_FOLDER,     editFolder);
    DDX_Control(pDX, IDC_PIM_CHECK_INCLUDE,   checkInclude);
    DDX_Control(pDX, IDC_PIM_BUT_SELECT,      butSelectFolder);
    DDX_Control(pDX, IDC_PIM_EDIT_REMOTE,     editRemote);
    DDX_Control(pDX, IDC_PIM_GROUP_DIRECTION, groupDirection);
    DDX_Control(pDX, IDC_PIM_GROUP_FOLDER,    groupFolder);
    DDX_Control(pDX, IDC_PIM_GROUP_ADVANCED,  groupAdvanced);
    DDX_Control(pDX, IDC_PIM_COMBO_FILTER,    lstFilter);
    DDX_Control(pDX, IDC_PIM_GROUP_FILTER,    groupFilter);
    DDX_Control(pDX, IDC_PIM_CHECK_SHARED,    checkShared);
}

BEGIN_MESSAGE_MAP(CPIMSettings, CDialog)
    ON_BN_CLICKED(IDC_PIM_BUTOK,       &CPIMSettings::OnBnClickedPIMButok)
    ON_BN_CLICKED(IDC_PIM_BUTCANCEL,   &CPIMSettings::OnBnClickedPIMButcancel)
    ON_BN_CLICKED(IDC_PIM_BUT_SELECT,  &CPIMSettings::OnBnClickedPIMButSelect)    
    ON_BN_CLICKED(IDC_PIM_CHECK_SHARED,&CPIMSettings::OnBnClickedCalendarCheckShared)
END_MESSAGE_MAP()


#ifdef _DEBUG
void CPIMSettings::AssertValid() const
{
	CDialog::AssertValid();
}

#ifndef _WIN32_WCE
void CPIMSettings::Dump(CDumpContext& dc) const
{
	CDialog::Dump(dc);
}
#endif
#endif //_DEBUG


// CPIMSettings message handlers

BOOL CPIMSettings::OnInitDialog() {

    CString s1;
    CDialog::OnInitDialog();

    string name;
    switch (type) {
        case SYNCSOURCE_CONTACTS:   name = CONTACT_;     break;
        case SYNCSOURCE_CALENDAR:   name = APPOINTMENT_; break;
        case SYNCSOURCE_TASKS:      name = TASK_;        break;
        case SYNCSOURCE_NOTES:      name = NOTE_;        break;
        default: return TRUE;
    }

    ssconf = ((OutlookConfig*)getConfig())->getSyncSourceConfig(name.c_str());
    if (!ssconf) {
        return TRUE;
    }
    
    editFolder.SetLimitText(EDIT_TEXT_MAXLENGTH);
    editRemote.SetLimitText(EDIT_TEXT_MAXLENGTH);
    
    // Load the syncmodes in the editbox/dropdown
    loadSyncModesBox();

    // load static string resources
    s1.LoadString(IDS_SYNC_DIRECTION);      SetDlgItemText(IDC_PIM_GROUP_DIRECTION, s1);
    s1.LoadString(IDS_CURRENT);             SetDlgItemText(IDC_PIM_STATIC_FOLDER, s1);
    s1.LoadString(IDS_INCLUDE_SUBFOLDERS);  SetDlgItemText(IDC_PIM_CHECK_INCLUDE, s1);
    s1.LoadString(IDS_SELECT_FOLDER);       SetDlgItemText(IDC_PIM_BUT_SELECT, s1);
    s1.LoadString(IDS_REMOTE_NAME);         SetDlgItemText(IDC_PIM_STATIC_REMOTE, s1);
    s1.LoadString(IDS_ADVANCED);            SetDlgItemText(IDC_PIM_GROUP_ADVANCED, s1);
    s1.LoadString(IDS_OK);                  SetDlgItemText(IDC_PIM_BUTOK, s1);
    s1.LoadString(IDS_CANCEL);              SetDlgItemText(IDC_PIM_BUTCANCEL, s1);
    s1.LoadString(IDS_DATA_FORMAT);         SetDlgItemText(IDC_PIM_STATIC_DATAFORMAT, s1);
    


    // source dependent resources
    if (type == SYNCSOURCE_CONTACTS) {
        s1.LoadString(IDS_CONTACTS_DETAILS);    SetWindowText(s1);
        s1.LoadString(IDS_CONTACTS_FOLDER);     SetDlgItemText(IDC_PIM_GROUP_FOLDER, s1);
        s1.LoadString(IDS_USE_VCARD);           SetDlgItemText(IDC_PIM_DATA_FORMAT, s1);
    }
    else if (type == SYNCSOURCE_CALENDAR) {
        s1.LoadString(IDS_CALENDAR_DETAILS);    SetWindowText(s1);
        s1.LoadString(IDS_CALENDAR_FOLDER);     SetDlgItemText(IDC_PIM_GROUP_FOLDER, s1);
        s1.LoadString(IDS_USE_VCAL);            SetDlgItemText(IDC_PIM_DATA_FORMAT, s1);
        s1.LoadString(IDS_SHARED);              SetDlgItemText(IDC_PIM_CHECK_SHARED, s1);
        s1.LoadString(IDS_EVENT_FILTER);        SetDlgItemText(IDC_PIM_GROUP_FILTER, s1);
        s1.LoadString(IDS_SYNC_PAST_EVENTS);    SetDlgItemText(IDC_PIM_STATIC_FILTER, s1);

        s1.LoadString(IDS_DATE_FILTER_NONE);            lstFilter.AddString(s1);
        s1.LoadString(IDS_DATE_FILTER_LAST_WEEK);       lstFilter.AddString(s1);
        s1.LoadString(IDS_DATE_FILTER_LAST_2_WEEKS);    lstFilter.AddString(s1);
        s1.LoadString(IDS_DATE_FILTER_LAST_MONTH);      lstFilter.AddString(s1);
        s1.LoadString(IDS_DATE_FILTER_LAST_3_MONTHS);   lstFilter.AddString(s1);
        s1.LoadString(IDS_DATE_FILTER_LAST_6_MONTHS);   lstFilter.AddString(s1);
        s1.LoadString(IDS_DATE_FILTER_ALL);             lstFilter.AddString(s1);

        lstFilter.SetCurSel(getDateFilterIndex(getConfig()->getAppointmentsDateFilter().getRelativeLowerDate()));
    }
    else if (type == SYNCSOURCE_TASKS) {
        s1.LoadString(IDS_TASKS_DETAILS);      SetWindowText(s1);
        s1.LoadString(IDS_TASKS_FOLDER);       SetDlgItemText(IDC_PIM_GROUP_FOLDER, s1);
        s1.LoadString(IDS_USE_VCAL);           SetDlgItemText(IDC_PIM_DATA_FORMAT, s1);
    }
    else if (type == SYNCSOURCE_NOTES) {
        s1.LoadString(IDS_NOTES_DETAILS);      SetWindowText(s1);
        s1.LoadString(IDS_NOTES_FOLDER);       SetDlgItemText(IDC_PIM_GROUP_FOLDER, s1);
        s1.LoadString(IDS_USE_SIF);            SetDlgItemText(IDC_PIM_DATA_FORMAT, s1);
    }
    else {
        return TRUE;
    }

    // Set dropdown-lists initial position
    lstSyncType.SetCurSel(getSyncTypeIndex(ssconf->getSync()));
    

    
    // Get folder path.
    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* olFolder = toWideChar(ssconf->getProperty(PROPERTY_FOLDER_PATH));
    s1 = olFolder;
    delete [] olFolder;

    try {
        if(s1 == "") {
            WCHAR* wname = toWideChar(name.c_str());
            s1 = getDefaultFolderPath(wname).data();
            delete [] wname;
        }
    }
    catch (...){
        // an exception occured while trying to get the default folder
        EndDialog(-1);
    }
    SetDlgItemText(IDC_PIM_EDIT_FOLDER, s1);

    bool err;
    if(ssconf->getBoolProperty(PROPERTY_USE_SUBFOLDERS, &err)) {
        checkInclude.SetCheck(BST_CHECKED);
    }

    // Note: use 'toWideChar' because we need UTF-8 conversion.
    WCHAR* remName = toWideChar(ssconf->getURI());
    s1 = remName;
    delete [] remName;
    SetDlgItemText(IDC_PIM_EDIT_REMOTE, s1);

    if (s1.Right(wcslen(SHARED_SUFFIX)).Compare(SHARED_SUFFIX) == 0) {
        checkShared.SetCheck(BST_CHECKED);
    }

    //
    // Apply customizations
    // ********************
    //
    bool shared             = UICustomization::shared;
    bool forceUseSubfolders = UICustomization::forceUseSubfolders;
    bool hideDataFormats    = UICustomization::hideDataFormats;
    bool hideAllAdvanced    = !SHOW_ADVANCED_SETTINGS;

    // calendar filtering (only for appointments)
    if (type == SYNCSOURCE_CALENDAR) {
        if (UICustomization::lockCalendarFilter) {
            int lockFilterIndex = UICustomization::lockCalendarFilterValue;
            CComboBox * item = (CComboBox*)GetDlgItem(IDC_PIM_COMBO_FILTER);
            item->EnableWindow(false);
            item->SetCurSel(lockFilterIndex);
        }
    }
    else {
        GetDlgItem(IDC_PIM_GROUP_FILTER)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PIM_STATIC_FILTER)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PIM_COMBO_FILTER)->ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        groupFilter.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        moveItem(this, &groupAdvanced, 0, dy);
        moveItem(this, &editRemote,    0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_DATA_FORMAT),       0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_STATIC_REMOTE),     0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_STATIC_DATAFORMAT), 0, dy);
        moveItem(this, &checkShared,                          0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_BUTOK),             0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_BUTCANCEL),         0, dy);

        setWindowHeight(this, GetDlgItem(IDC_PIM_BUTOK));
    }

    if (forceUseSubfolders) {
        checkInclude.SetCheck(BST_CHECKED);
        checkInclude.ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        checkInclude.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        resizeItem(GetDlgItem(IDC_PIM_GROUP_FOLDER), 0, dy);

        moveItem(this, &groupAdvanced, 0, dy);
        moveItem(this, &editRemote,    0, dy);
        moveItem(this, &checkShared,   0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_DATA_FORMAT),       0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_STATIC_REMOTE),     0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_STATIC_DATAFORMAT), 0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_BUTOK),             0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_BUTCANCEL),         0, dy);
        if (type == SYNCSOURCE_CALENDAR) {
            moveItem(this, GetDlgItem(IDC_PIM_GROUP_FILTER),  0, dy);
            moveItem(this, GetDlgItem(IDC_PIM_STATIC_FILTER), 0, dy);
            moveItem(this, GetDlgItem(IDC_PIM_COMBO_FILTER),  0, dy);
        }

        setWindowHeight(this, GetDlgItem(IDC_PIM_BUTOK));
    }

    if (hideAllAdvanced) {
        groupAdvanced.ShowWindow(SW_HIDE);
        editRemote.ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PIM_STATIC_REMOTE)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PIM_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PIM_DATA_FORMAT)->ShowWindow(SW_HIDE);

        CRect rect;
        groupAdvanced.GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 10);

        moveItem(this, GetDlgItem(IDC_PIM_BUTOK),     0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_BUTCANCEL), 0, dy);

        setWindowHeight(this, GetDlgItem(IDC_PIM_BUTOK));
    } 
    else if (hideDataFormats) {
        GetDlgItem(IDC_PIM_STATIC_DATAFORMAT)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_PIM_DATA_FORMAT)->ShowWindow(SW_HIDE);

        // Resize things
        CRect rect;
        GetDlgItem(IDC_PIM_STATIC_DATAFORMAT)->GetClientRect(&rect);
        int dy = -1 * (rect.Height() + 5);

        resizeItem(&groupAdvanced, 0, dy);

        moveItem(this, GetDlgItem(IDC_PIM_BUTOK),     0, dy);
        moveItem(this, GetDlgItem(IDC_PIM_BUTCANCEL), 0, dy);

        setWindowHeight(this, GetDlgItem(IDC_PIM_BUTOK));
    }

    // Shared folders (only for appointments)
    if (type == SYNCSOURCE_CALENDAR && shared) {
        editRemote.EnableWindow(false);
    } else {
        GetDlgItem(IDC_PIM_CHECK_SHARED)->ShowWindow(SW_HIDE);
    } 


    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupDirection.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupFolder.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupAdvanced.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupFilter.m_hWnd,L" ",L" ");
    }

    // Accessing Outlook, could be no more in foreground
    SetForegroundWindow();

    wndPIM = this;
    return FALSE;
}

void CPIMSettings::OnBnClickedPIMButok()
{
    // OK Button
    if(saveSettings(false)){
        CDialog::OnOK();
    }
}

void CPIMSettings::OnBnClickedPIMButcancel()
{
    // Never read from winreg, will save when 'OK' is clicked on SyncSettings.
    //getConfig()->read();
    CDialog::OnCancel();
}


bool CPIMSettings::saveSettings(bool saveToDisk)
{
    CString remoteName, outlookFolder, syncType;
    CString s1;
    _bstr_t bst;


    GetDlgItemText(IDC_PIM_EDIT_REMOTE, remoteName);
    GetDlgItemText(IDC_PIM_EDIT_FOLDER, outlookFolder);

    // change values
    if(remoteName == ""){
        // remote name is empty
        s1.LoadString(IDS_ERROR_SET_REMOTE_NAME);
        wsafeMessageBox(s1);
        return false;
    }

    if (UICustomization::showWarningOnChangeFromOneWay) {
        int currentSyncType = getSyncTypeIndex(ssconf->getSync());
        int newSyncType = lstSyncType.GetCurSel();
        if (checkOneWayToTwoWay(currentSyncType, newSyncType)) {
           return false;
        }
    }

    // sync source enabled
    ssconf->setSync(getSyncTypeName(lstSyncType.GetCurSel()));

    // Date Filter
    if (type == SYNCSOURCE_CALENDAR) {
        int filterPos = lstFilter.GetCurSel();
        DateFilter& df = getConfig()->getAppointmentsDateFilter();
        df.setRelativeLowerDate(getDateFilterValue(filterPos));
    }


    // Note: use 'toMultibyte' which uses charset UTF-8.
    //       (when writing to winreg, toWideChar is then called)
    char* olFolder = toMultibyte(outlookFolder.GetBuffer());
    if (olFolder) {
        // If folder has changed, clear anchors
        if (UICustomization::clearAnchorsOnFolderChange) {
            const char * original = ssconf->getProperty(PROPERTY_FOLDER_PATH);
            if (strcmp(original, olFolder) != 0) {
                ssconf->setLast(0);
                ssconf->setLongProperty(PROPERTY_SYNC_END, 0);
            }
        }
        
        ssconf->setProperty(PROPERTY_FOLDER_PATH, olFolder);
        delete [] olFolder;
    }

    if(checkInclude.GetCheck() == BST_CHECKED){
        ssconf->setBoolProperty(PROPERTY_USE_SUBFOLDERS, true);
    } else {
        ssconf->setBoolProperty(PROPERTY_USE_SUBFOLDERS, false);
    }

    StringBuffer remName;
    remName.convert(remoteName.GetBuffer());
    if (!remName.null()) {
        ssconf->setURI(remName.c_str());
    }   

    // Never save to winreg, will save when 'OK' is clicked on SyncSettings.
    //if(saveToDisk)
    //    ((OutlookConfig*)getConfig())->save();

    return true;
}

int pickFolderPIM(LPVOID lpv) {

    int type = (int)lpv;
    wstring name;
    switch (type) {
        case SYNCSOURCE_CONTACTS:   name = CONTACT;     break;
        case SYNCSOURCE_CALENDAR:   name = APPOINTMENT; break;
        case SYNCSOURCE_TASKS:      name = TASK;        break;
        case SYNCSOURCE_NOTES:      name = NOTE;        break;
        default: return 1;
    }

    CString s1;
    try {
        outlookFolder = pickOutlookFolder(name.c_str());
    }
    catch (...) {
        printLog("Exception thrown by pickOutlookFolder", LOG_DEBUG);
        outlookFolder = L"";
    }

    if (wndPIM) {      // dialog could have been closed...
        if (outlookFolder != L""){
            s1 = outlookFolder.c_str();
            wndPIM->SetDlgItemText(IDC_PIM_EDIT_FOLDER, s1);
        }
        wndPIM->SetForegroundWindow();
        wndPIM->EndModalState();
    }
    return 0;
}

void CPIMSettings::OnBnClickedPIMButSelect() {
    outlookFolder = L"";
    BeginModalState();
    handleThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)pickFolderPIM, (LPVOID)type, 0, NULL);
}



void CPIMSettings::OnBnClickedCalendarCheckShared() {
    long editId = IDC_PIM_EDIT_REMOTE;

    CString currentValue;
    GetDlgItemText(editId, currentValue);
    CString warningMessage;
    warningMessage.LoadString(IDS_UNCHECK_SHARED);

    CString newValue = processSharedCheckboxClick(CALENDAR_REMOTE_NAME,
        checkShared.GetCheck() != 0, currentValue, warningMessage);

    SetDlgItemText(editId, newValue);
}

void CPIMSettings::loadSyncModesBox()
{
    // TODO: use a switch on sourceName when refactoring
    int editBoxResourceID = IDC_PIM_EDIT_SYNCTYPE;
    int comboBoxResourceID = IDC_PIM_COMBO_SYNCTYPE;

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
