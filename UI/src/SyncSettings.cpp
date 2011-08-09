/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2003 - 2007 Funambol, Inc.
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

// SyncSettings.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "SyncSettings.h"
#include "MainSyncFrm.h"
#include "ClientUtil.h"

#include "LeftView.h"
#include "PIMSettings.h"
#include "PicturesSettings.h"
#include "VideosSettings.h"
#include "FilesSettings.h"
#include "UICustomization.h"
#include "SettingsHelper.h"

#include "customization.h"
#include "winmaincpp.h"
#include "utils.h"
#include "OutlookPlugin.h"


// Values for the scheduler
static StringBuffer minutesString = SCHEDULED_MINUTES_VALUES;
static StringBuffer hoursString   = SCHEDULED_HOURS_VALUES;
static ArrayList minutesA, hoursA;
static int defaultPosition = 0;
static bool initialized = false;

bool hasDefault(StringBuffer* value) {  
    if (value->find("fault") != StringBuffer::npos) {
        return true;
    }
    return false;
}

StringBuffer removeDefaultString(StringBuffer& value) {
    StringBuffer ret = value;
    size_t pos = value.find("(");
    if (pos != StringBuffer::npos) {
        ret = value.substr(0, pos);
    }
    return ret;
}

void populateArrays() {
    
    ArrayList tmp;
    StringBuffer* el; 
    StringBuffer value, completeValue;
    int res = 0, minSize = 0, hourSize = 0, totalSize = 0;

    // safe check to remove element that are number <1 and >59
    if (!minutesString.empty()) {
    minutesString.split(tmp, ",");    
    for (el = (StringBuffer *)tmp.front(); el; el = (StringBuffer *)tmp.next() ) {
        completeValue = el->c_str();
        value = removeDefaultString(completeValue);
        res = atoi(value.c_str());
        if (res >= 1 && res <= 59) {
            minutesA.add(completeValue);
        }
    }
    }

    tmp.clear();
    if (!hoursString.empty()) {
    hoursString.split(tmp, ",");
    for (el = (StringBuffer *)tmp.front(); el; el = (StringBuffer *)tmp.next() ) {
        completeValue = el->c_str();
        value = removeDefaultString(completeValue);
        res = atoi(value.c_str());
        if (res >= 1 && res <= 24) {
            hoursA.add(completeValue);
        }
    }
    }
    minSize = minutesA.size();
    hourSize = hoursA.size();
    totalSize = minSize + hourSize;

    for (int i = 0; i < totalSize; i++) {
        if (i < minSize) {
            if (hasDefault((StringBuffer*)minutesA.get(i))) {
                StringBuffer v = removeDefaultString(*((StringBuffer*)minutesA.get(i)));
                minutesA.removeElementAt(i);
                minutesA.add(i, v);
                if (defaultPosition == 0) {
                    defaultPosition = i;
                }
            }
        } else {
            if (hasDefault((StringBuffer*)hoursA.get(i-minSize))) {
                StringBuffer v = removeDefaultString(*((StringBuffer*)hoursA.get(i-minSize)));
                hoursA.removeElementAt(i-minSize);                
                hoursA.add(i-minSize, v);
                if (defaultPosition == 0) {
                    defaultPosition = i;
                }
            }
        }
    }
}

int getSchedulerPosition(int minutes) {

    int minSize = minutesA.size();
    int hourSize = hoursA.size();
    int totalSize = minSize + hourSize;
    StringBuffer* s = NULL;
    int res = 0, position = 0; 

    for (int i = 0; i < totalSize; i++) {
        if (i < minSize) {
            s = (StringBuffer*)minutesA.get(i);
            res = atoi(s->c_str());
            if (minutes <= res) {
                position = i;
                break;
            }            
        } else {
            s = (StringBuffer*)hoursA.get(i-minSize);
            res = atoi(s->c_str());
            res = res * 60;
            if (minutes <= res) {
                position = i;
                break;
            }    
        }
    }
    return position;
}

int getSchedulerMinutes(int position) {
    
    int res = 0;
    StringBuffer* s = NULL;
    if (position < minutesA.size()) {
        s = (StringBuffer*)minutesA.get(position);
        res = atoi(s->c_str());
    } else {
        s = (StringBuffer*)hoursA.get(position - minutesA.size());
        res = atoi(s->c_str());
        res = res * 60;
    }
    return res;
}

bool isMediaHubSet() {

    CMainSyncFrame syncFrame;
    int res = syncFrame.OnCheckMediaHubFolder(0,0);
    
    if (res != IDOK) {
        return false;
    }
    return true;
}


// CSyncSettings
IMPLEMENT_DYNCREATE(CSyncSettings, CFormView)

CSyncSettings::CSyncSettings() : CFormView(CSyncSettings::IDD) {}

CSyncSettings::~CSyncSettings() {}

void CSyncSettings::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SYNC_CHECK_CONTACTS, checkContacts);
    DDX_Control(pDX, IDC_SYNC_CHECK_CALENDAR, checkCalendar);
    DDX_Control(pDX, IDC_SYNC_CHECK_TASKS,    checkTasks);
    DDX_Control(pDX, IDC_SYNC_CHECK_NOTES,    checkNotes);
    DDX_Control(pDX, IDC_SYNC_CHECK_PICTURES, checkPictures);
    DDX_Control(pDX, IDC_SYNC_CHECK_VIDEOS,   checkVideos);
    DDX_Control(pDX, IDC_SYNC_CHECK_FILES,    checkFiles);

    DDX_Control(pDX, IDC_SYNC_BUT_CONTACTS, butContacts);
    DDX_Control(pDX, IDC_SYNC_BUT_CALENDAR, butCalendar);
    DDX_Control(pDX, IDC_SYNC_BUT_TASKS,    butTasks);
    DDX_Control(pDX, IDC_SYNC_BUT_NOTES,    butNotes);
    DDX_Control(pDX, IDC_SYNC_BUT_PICTURES, butPictures);
    DDX_Control(pDX, IDC_SYNC_BUT_VIDEOS,   butVideos);
    DDX_Control(pDX, IDC_SYNC_BUT_FILES,    butFiles);

    DDX_Control(pDX, IDC_SCHEDULER_CHECK_ENABLED, checkEnabled);
    DDX_Control(pDX, IDC_SCHEDULER_COMBO_VALUE,   comboSchedulerValue);
    DDX_Control(pDX, IDC_SYNC_CHECK_ENCRYPTION,   checkEncryption);
    DDX_Control(pDX, IDC_SYNC_CHECK_OUTLOOK_OPEN, checkAttach);
    DDX_Control(pDX, IDC_SYNC_GROUP_ITEMS,        groupItems);
    DDX_Control(pDX, IDC_SCHEDULER_GROUP,         groupScheduler);
    DDX_Control(pDX, IDC_SECURITY_GROUP,          groupSecurity);
}

BEGIN_MESSAGE_MAP(CSyncSettings, CFormView)
    ON_MESSAGE( WM_INITDIALOG, OnInitForm ) 
    ON_BN_CLICKED(IDC_SYNC_CHECK_CONTACTS, &CSyncSettings::OnBnClickedSyncCheckContacts)
    ON_BN_CLICKED(IDC_SYNC_CHECK_CALENDAR, &CSyncSettings::OnBnClickedSyncCheckCalendar)
    ON_BN_CLICKED(IDC_SYNC_CHECK_TASKS,    &CSyncSettings::OnBnClickedSyncCheckTasks)
    ON_BN_CLICKED(IDC_SYNC_CHECK_NOTES,    &CSyncSettings::OnBnClickedSyncCheckNotes)
    ON_BN_CLICKED(IDC_SYNC_CHECK_PICTURES, &CSyncSettings::OnBnClickedSyncCheckPictures)
    ON_BN_CLICKED(IDC_SYNC_CHECK_VIDEOS,   &CSyncSettings::OnBnClickedSyncCheckVideos)
    ON_BN_CLICKED(IDC_SYNC_CHECK_FILES,    &CSyncSettings::OnBnClickedSyncCheckFiles)
    ON_BN_CLICKED(IDC_SYNC_OK,             &CSyncSettings::OnBnClickedSyncOk)
    ON_BN_CLICKED(IDC_SYNC_CANCEL,         &CSyncSettings::OnBnClickedSyncCancel)
    ON_BN_CLICKED(IDC_SYNC_BUT_CONTACTS,   &CSyncSettings::OnBnClickedSyncButContacts)
    ON_BN_CLICKED(IDC_SYNC_BUT_CALENDAR,   &CSyncSettings::OnBnClickedSyncButCalendar)
    ON_BN_CLICKED(IDC_SYNC_BUT_TASKS,      &CSyncSettings::OnBnClickedSyncButTasks)
    ON_BN_CLICKED(IDC_SYNC_BUT_NOTES,      &CSyncSettings::OnBnClickedSyncButNotes)
    ON_BN_CLICKED(IDC_SYNC_BUT_PICTURES,   &CSyncSettings::OnBnClickedSyncButPictures)
    ON_BN_CLICKED(IDC_SYNC_BUT_VIDEOS,     &CSyncSettings::OnBnClickedSyncButVideos)
    ON_BN_CLICKED(IDC_SYNC_BUT_FILES,      &CSyncSettings::OnBnClickedSyncButFiles)
    ON_WM_NCPAINT()
    ON_BN_CLICKED(IDC_SCHEDULER_CHECK_ENABLED,  &CSyncSettings::OnBnClickedSchedulerCheckEnabled)
    ON_CBN_SELCHANGE(IDC_SCHEDULER_COMBO_VALUE, &CSyncSettings::OnCbnSelchangeSchedulerComboValue)
    ON_BN_CLICKED(IDC_SYNC_CHECK_OUTLOOK_OPEN,  &CSyncSettings::OnBnClickedSyncCheckOutlookOpen)
END_MESSAGE_MAP()


// CSyncSettings diagnostics

#ifdef _DEBUG
void CSyncSettings::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CSyncSettings::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG



// CSyncSettings message handlers
LRESULT CSyncSettings::OnInitForm(WPARAM, LPARAM) {

    CFormView::OnInitialUpdate();
    CString s1;
    
    // Load static objects labels
    s1.LoadString(IDS_ITEMS);                   SetDlgItemText(IDC_SYNC_GROUP_ITEMS,        s1);
    s1.LoadString(IDS_SCHEDULER);               SetDlgItemText(IDC_SCHEDULER_GROUP,         s1);
    s1.LoadString(IDS_SYNC_SYNCHRONIZE_EVERY);  SetDlgItemText(IDC_SCHEDULER_CHECK_ENABLED, s1);
    s1.LoadString(IDS_SYNC_ENABLE_ENCRYPTION);  SetDlgItemText(IDC_SYNC_CHECK_ENCRYPTION,   s1);
    s1.LoadString(IDS_REQUIRE_OUTLOOK_OPEN);    SetDlgItemText(IDC_SYNC_CHECK_OUTLOOK_OPEN, s1);
    s1.LoadString(IDS_OK);                      SetDlgItemText(IDC_SYNC_OK,                 s1);
    s1.LoadString(IDS_CANCEL);                  SetDlgItemText(IDC_SYNC_CANCEL,             s1);
    s1.LoadString(IDS_SECURITY);                SetDlgItemText(IDC_SYNC_GROUP_SECURITY,     s1);
    s1.LoadString(IDS_SYNC_ENABLE_ENCRYPTION);  SetDlgItemText(IDC_SYNC_CHECK_ENCRYPTION,   s1);



    CRect formRect, groupRect;
    GetClientRect(&formRect);
    CPoint posGroup = getRelativePosition(&groupItems, this);
    groupItems.GetClientRect(&groupRect);

    int x_space_left    = (int)(groupRect.Width()  * 0.05);
    int y_space         = (int)(groupRect.Height() * 0.07);
    int y_space_top     = (int)(groupRect.Height() * 0.04);
    int y_space_bottom  = (int)(groupRect.Height() * 0.08);
    int y_space_groups  = (int)(groupRect.Height() * 0.04);
    int x_checkbox      = posGroup.x + x_space_left;
    int y               = posGroup.y + y_space_top;     // <-- this is the y position of each item, topdown

    int checkbox_width  = (int)(groupRect.Width() * 0.70) - x_space_left;
    int x_button        = x_checkbox + checkbox_width + 1;
    int button_width    = (int)(groupRect.Width() * 0.98) - x_button;

    CWnd* lastWnd = NULL;   // for TAB order: this is the last Window drawed

    //
    // Sources group
    // ------------- 
    hideAllSources();
    const ArrayList& sources = getConfig()->getSourcesVisible();
    for (int i=0; i<sources.size(); i++) 
    {
        y += y_space;

        StringBuffer* sourceName = (StringBuffer*)sources.get(i);
        if (!sourceName) continue;

        SyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(sourceName->c_str());
        if (!ssc) continue;

        // add a separator (not the first one)
        if (i > 0) {
            CWnd* sep = getSeparator(i);
            if (!sep) continue;

            CRect sepRect;
            sep->GetClientRect(&sepRect);
            int x_sep = posGroup.x + (groupRect.Width() - sepRect.Width()) / 2;  // center on x axis
            sep->SetWindowPos(&CWnd::wndTop, x_sep, y, NULL, NULL, SWP_SHOWWINDOW | SWP_NOSIZE);
            y += y_space;
        }

        // CHECKBOX & BUTTON
        CButton* checkbox = getCheckbox(*sourceName);   if (!checkbox) continue;
        CButton* button    = getButton (*sourceName);   if (!button)   continue;

        CRect boxRect, butRect;
        checkbox->GetClientRect(&boxRect);
        button->GetClientRect(&butRect);

        // to center on y axis
        int y_checkbox = y - boxRect.Height() / 2;
        int y_button   = y - butRect.Height() / 2;

        checkbox->SetWindowPos(lastWnd,  x_checkbox, y_checkbox, checkbox_width, boxRect.Height(), SWP_SHOWWINDOW);
        button->SetWindowPos  (checkbox, x_button,   y_button,   button_width,   butRect.Height(), SWP_SHOWWINDOW);
        lastWnd = button;

        s1 = composeCheckboxText(sourceName->c_str());   checkbox->SetWindowText(s1);
        s1.LoadString(IDS_DETAILS);                        button->SetWindowText(s1);

        // enable/disable
        if (ssc->isAllowed()) {
            if (ssc->isEnabled()) {
                checkbox->SetCheck(BST_CHECKED);
            } else {
                button->EnableWindow(FALSE);
            }
        }
        else {
            checkbox->EnableWindow(FALSE);
            button->EnableWindow(FALSE);
        }
    }

    // resize the sources groupbox dinamically
    y += y_space_bottom;
    int group_heigth = y - posGroup.y;
    groupItems.SetWindowPos(&CWnd::wndTop, NULL, NULL, groupRect.Width(), group_heigth, SWP_SHOWWINDOW | SWP_NOMOVE);


    //
    // Scheduler group
    // ---------------
    saveScheduler = false;
    if (initialized == false) {
        populateArrays();
        initialized = true;
    }
    if (minutesA.size() == 0 && hoursA.size() == 0) {
        // scheduler is NOT visible (hide all)
        checkEnabled.EnableWindow(FALSE);
        checkEnabled.ShowWindow(SW_HIDE);
        comboSchedulerValue.ShowWindow(SW_HIDE);
        groupScheduler.ShowWindow(SW_HIDE);
        checkAttach.ShowWindow(SW_HIDE);
    } 
    else {
        // scheduler is visible
        loadSchedulerData();

        // move items
        y += y_space_groups;
        CPoint schedPos = getRelativePosition(&groupScheduler, this);
        int dy = -(schedPos.y - y);
        moveItem(this, &groupScheduler,      0, dy);
        moveItem(this, &checkEnabled,        0, dy, lastWnd);
        moveItem(this, &comboSchedulerValue, 0, dy, &checkEnabled);
        lastWnd = &comboSchedulerValue;

        // attach option
        if (UICustomization::attachOption) {
            // attach option is visible
            if (getConfig()->getWindowsDeviceConfig().getAttach()) {
                checkAttach.SetCheck(BST_CHECKED);
            } else {
                checkAttach.SetCheck(BST_UNCHECKED);
            }
            moveItem(this, &checkAttach, 0, dy, lastWnd);
            lastWnd = &checkAttach;
        } 
        else {
            // attach option is NOT visible
            checkAttach.SetCheck(BST_UNCHECKED);
            checkAttach.ShowWindow(SW_HIDE);
            resizeItem(&groupScheduler, 0, -17);
        }

        CRect schedRect;
        groupScheduler.GetClientRect(&schedRect);
        y += schedRect.Height();
    }

    
    //
    // Security group
    // --------------
    if (!ENABLE_ENCRYPTION_SETTINGS) {
        // Encryption check is NOT visible
        checkEncryption.SetCheck(BST_UNCHECKED);
        checkEncryption.EnableWindow(FALSE);
        checkEncryption.ShowWindow(FALSE);
        groupSecurity.ShowWindow(FALSE);
    }
    else {
        // Encryption check is visible
        if( (strcmp(getConfig()->getSyncSourceConfig(CONTACT_)->getEncryption(),"") != 0) ||
            (strcmp(getConfig()->getSyncSourceConfig(APPOINTMENT_)->getEncryption(),"") != 0) ||
            (strcmp(getConfig()->getSyncSourceConfig(TASK_)->getEncryption(),"") != 0) ||
            (strcmp(getConfig()->getSyncSourceConfig(NOTE_)->getEncryption(),"") != 0) ) {
            checkEncryption.SetCheck(BST_CHECKED);
        }
        else {
            checkEncryption.SetCheck(BST_UNCHECKED);
        }

        // move items
        y += y_space_groups;
        CPoint secPos = getRelativePosition(&groupSecurity, this);
        int dy = -(secPos.y - y);
        moveItem(this, &groupSecurity,   0, dy);
        moveItem(this, &checkEncryption, 0, dy, lastWnd);
        lastWnd = &checkEncryption;

        CRect secRect;
        groupSecurity.GetClientRect(&secRect);
        y += secRect.Height();
    }



    // disable windows xp theme, otherwise any color setting for groupbox
    // will be overriden by the theme settings
    if(((COutlookPluginApp*)AfxGetApp())->hLib){
        PFNSETWINDOWTHEME pfnSetWindowTheme =
            (PFNSETWINDOWTHEME)GetProcAddress(((COutlookPluginApp*)AfxGetApp())->hLib, "SetWindowTheme");
        pfnSetWindowTheme (groupItems.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupScheduler.m_hWnd,L" ",L" ");
        pfnSetWindowTheme (groupSecurity.m_hWnd,L" ",L" ");
    }

    return 0;
}

void CSyncSettings::loadSchedulerData() {

    CString s1, sched;
    s1.LoadString(IDS_MINUTES);
    StringBuffer val;
    int minutes = 0;

    for (int i = 0; i < minutesA.size(); i++) {
        val = ((StringBuffer*)(minutesA.get(i)))->c_str();
        sched = val.c_str();     sched += " ";     sched += s1;
        comboSchedulerValue.AddString(sched);
    }
    
    s1.LoadString(IDS_HOUR);
    for (int i = 0; i < hoursA.size(); i++) {
        val = ((StringBuffer*)(hoursA.get(i)))->c_str();
        if (val == "1") s1.LoadString(IDS_HOUR);
        else            s1.LoadString(IDS_HOURS);
        sched = val.c_str();     sched += " ";     sched += s1;
        comboSchedulerValue.AddString(sched);
    }

    if(!getScheduler(&minutes)){
        // schedule job NOT active
        checkEnabled.SetCheck(BST_UNCHECKED);
        comboSchedulerValue.EnableWindow(FALSE);
        int pos = defaultPosition;
        comboSchedulerValue.SetCurSel(pos);
    }
    else{
        // schedule job is active
        checkEnabled.SetCheck(BST_CHECKED);
        comboSchedulerValue.EnableWindow(TRUE);
        int pos = getSchedulerPosition(minutes);
        comboSchedulerValue.SetCurSel(pos);
        if (getSchedulerMinutes(pos) != minutes) {
            // Scheduler time was not exactly this one (manually modified?)
            saveScheduler = true;
        }
    }
}


bool CSyncSettings::saveSettings(bool saveToDisk)
{    
    //
    // Set enabled/disabled flag for all sources
    //
    const ArrayList& sources = getConfig()->getSourcesVisible();
    for (int i=0; i<sources.size(); i++) {
        StringBuffer* sourceName = (StringBuffer*)sources.get(i);
        if (!sourceName) continue;
        
        saveEnabledCheck(*sourceName);
    }


    //
    // Save scheduler settings
    //
    CString msg;
    int minutes=0;

    // if scheduler values haven't changed, do not save it again
    if (saveScheduler) {
        // check scheduler values
        if (checkEnabled.GetCheck() == BST_UNCHECKED) {
            setScheduler(false, 0);
        }
        else {
            // scheduler enabled
            int pos = comboSchedulerValue.GetCurSel();
            minutes = getSchedulerMinutes(pos);

            // save scheduler settings
            if (setScheduler(true, minutes) != 0) {
                msg.LoadString(IDS_SCHEDULER_CANNOT_SCHEDULE);
                wsafeMessageBox(msg);
            }
        }
    }
    saveScheduler = false;

    //
    // Save encryption, global property 
    //
    for (unsigned int i=0; i<getConfig()->getSyncSourceConfigsCount(); i++) {
        SyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(i);
        if (!ssc) continue;

        // Only PIM sources!
        // (cannot DES a largeObject read chunk by chunk via input stream)
        if ( !isPIMSource(ssc->getName()) ) {
            continue;
        }

        if (checkEncryption.GetCheck()) {
            ssc->setEncryption("des");
            ssc->setEncoding("");
        } 
        else {
            ssc->setEncryption("");
            
            // For SIF types: the encoding is always b64
            StringBuffer type(ssc->getType());
            if (type.ifind("sif") == StringBuffer::npos) {
                ssc->setEncoding("b64");
            } else {
                ssc->setEncoding("bin");
            }
        }
    }

    if (saveAttach) {
        bool attach = (checkAttach.GetCheck() == BST_CHECKED);
        getConfig()->getWindowsDeviceConfig().setAttach(attach);
    }

    //
    // SAVE TO DISK!
    //
    if (saveToDisk) {
        getConfig()->save();
    }
    return true;
}


void CSyncSettings::saveEnabledCheck(const StringBuffer& sourceName) {

    SyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(sourceName.c_str());
    if (!ssc) return;

    CButton* checkbox = getCheckbox(sourceName);
    if (!checkbox) return;

    bool enabled = (checkbox->GetCheck() == BST_CHECKED);
    ssc->setIsEnabled(enabled);
}



void CSyncSettings::OnBnClickedSyncCheckContacts()
{
    if(checkContacts.GetCheck() == BST_UNCHECKED){
        butContacts.EnableWindow(FALSE);
    }
    else{
        butContacts.EnableWindow(TRUE);
    }
}

void CSyncSettings::OnBnClickedSyncCheckCalendar()
{
    if(checkCalendar.GetCheck() == BST_UNCHECKED){
        butCalendar.EnableWindow(FALSE);
    }
    else{
        butCalendar.EnableWindow(TRUE);
    }
}

void CSyncSettings::OnBnClickedSyncCheckTasks()
{
    if(checkTasks.GetCheck() == BST_UNCHECKED){
        butTasks.EnableWindow(FALSE);
    }
    else{
        butTasks.EnableWindow(TRUE);
    }
}

void CSyncSettings::OnBnClickedSyncCheckNotes()
{
    if(checkNotes.GetCheck() == BST_UNCHECKED)
        butNotes.EnableWindow(FALSE);
    else
        butNotes.EnableWindow(TRUE);
}

void CSyncSettings::OnBnClickedSyncCheckPictures()
{
    if(checkPictures.GetCheck() == BST_UNCHECKED)
        butPictures.EnableWindow(FALSE);
    else
        butPictures.EnableWindow(TRUE);
}

void CSyncSettings::OnBnClickedSyncCheckVideos()
{
    if(checkVideos.GetCheck() == BST_UNCHECKED)
        butVideos.EnableWindow(FALSE);
    else
        butVideos.EnableWindow(TRUE);
}

void CSyncSettings::OnBnClickedSyncCheckFiles()
{
    if(checkFiles.GetCheck() == BST_UNCHECKED)
        butFiles.EnableWindow(FALSE);
    else
        butFiles.EnableWindow(TRUE);
}

void CSyncSettings::OnBnClickedSyncOk()
{
    // OK Button: save settings to disk
    saveSettings();

    // return to main screen
    ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
}

void CSyncSettings::OnBnClickedSyncCancel()
{
    // CANCEL button: restore original settings
    getConfig()->read();

    // return to main screen
    ((CMainSyncFrame*)AfxGetMainWnd())->pConfigFrame->DoCancel();
}

void CSyncSettings::OnBnClickedSyncButContacts()
{
    CPIMSettings wndContacts(SYNCSOURCE_CONTACTS);
    INT_PTR result = wndContacts.DoModal();

    // Update the UI checkbox
    CString s1 = composeCheckboxText(CONTACT_);
    SetDlgItemText(IDC_SYNC_CHECK_CONTACTS, s1);
}

void CSyncSettings::OnBnClickedSyncButCalendar()
{
    CPIMSettings wndCalendar(SYNCSOURCE_CALENDAR);
    INT_PTR result = wndCalendar.DoModal();

    // Update the UI checkbox
    CString s1 = composeCheckboxText(APPOINTMENT_);
    SetDlgItemText(IDC_SYNC_CHECK_CALENDAR, s1);
}

void CSyncSettings::OnBnClickedSyncButTasks()
{
    CPIMSettings wndTasks(SYNCSOURCE_TASKS);
    INT_PTR result = wndTasks.DoModal();

    // Update the UI checkbox
    CString s1 = composeCheckboxText(TASK_);
    SetDlgItemText(IDC_SYNC_CHECK_TASKS, s1);
}

void CSyncSettings::OnBnClickedSyncButNotes()
{
    CPIMSettings wndNotes(SYNCSOURCE_NOTES);
    INT_PTR result = wndNotes.DoModal();

    // Update the UI checkbox
    CString s1 = composeCheckboxText(NOTE_);
    SetDlgItemText(IDC_SYNC_CHECK_NOTES, s1);
}


void CSyncSettings::OnBnClickedSyncButPictures()
{
    if (isMediaHubSet() == false) {
        return;
    }
    CPicturesSettings wndPictures;
    INT_PTR result = wndPictures.DoModal();

    // Update the UI checkbox
    CString s1 = composeCheckboxText(PICTURE_);
    SetDlgItemText(IDC_SYNC_CHECK_PICTURES, s1);
}

void CSyncSettings::OnBnClickedSyncButVideos()
{
    if (isMediaHubSet() == false) {
        return;
    }
    CVideosSettings wndVideos;
    INT_PTR result = wndVideos.DoModal();

    // Update the UI checkbox
    CString s1 = composeCheckboxText(VIDEO_);
    SetDlgItemText(IDC_SYNC_CHECK_VIDEOS, s1);
}

void CSyncSettings::OnBnClickedSyncButFiles()
{
    if (isMediaHubSet() == false) {
        return;
    }
    CFilesSettings wndFiles;
    INT_PTR result = wndFiles.DoModal();

      // Update the UI checkbox
    CString s1 = composeCheckboxText(FILES_);
    SetDlgItemText(IDC_SYNC_CHECK_FILES, s1);
}



void CSyncSettings::OnNcPaint()
{
    CFormView::OnNcPaint();
    CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));   
}

void CSyncSettings::OnBnClickedSchedulerCheckEnabled()
{
    if(checkEnabled.GetCheck() == BST_UNCHECKED){
        comboSchedulerValue.EnableWindow(FALSE);
        checkAttach.EnableWindow(FALSE);
    }
    else{
        comboSchedulerValue.EnableWindow(TRUE);
        int pos = defaultPosition;
        comboSchedulerValue.SetCurSel(pos);
        checkAttach.EnableWindow(TRUE);
    }

    saveScheduler = true;
}



BOOL CSyncSettings::PreTranslateMessage(MSG* pMsg){
    bool bProcessed =false;
    if(pMsg->message == WM_KEYDOWN){
        if(pMsg->wParam == VK_ESCAPE){
            OnBnClickedSyncCancel();
            bProcessed = true;
        }
    }

    if(bProcessed)
        return TRUE;
    else
        return CFormView::PreTranslateMessage(pMsg);
}


void CSyncSettings::OnCbnSelchangeSchedulerComboValue()
{
    saveScheduler = true;
}

void CSyncSettings::OnBnClickedSyncCheckOutlookOpen()
{
    saveAttach = true;
}


//
// util methods
//
CString CSyncSettings::composeCheckboxText(const char* sourceName)
{
    CString ret;

    //
    // Add the name of the source
    //
    if      (!strcmp(sourceName, CONTACT_))      { ret.LoadString(IDS_CONTACTS); }
    else if (!strcmp(sourceName, APPOINTMENT_))  { ret.LoadString(IDS_CALENDAR); }
    else if (!strcmp(sourceName, TASK_))         { ret.LoadString(IDS_TASKS); }
    else if (!strcmp(sourceName, NOTE_))         { ret.LoadString(IDS_NOTES); }
    else if (!strcmp(sourceName, PICTURE_))      { ret.LoadString(IDS_PICTURES); }
    else if (!strcmp(sourceName, VIDEO_))        { ret.LoadString(IDS_VIDEOS); }
    else if (!strcmp(sourceName, FILES_))        { ret.LoadString(IDS_FILES); }

    //
    // Append the "(Download/Upload Only)" if a one-way is currently set
    //
    OutlookConfig* config = getConfig();
    SyncSourceConfig* wssc = config->getSyncSourceConfig(sourceName);
    if (!wssc) {
        return ret;
    }

    const char* syncMode = wssc->getSync();

    if (!strcmp(syncMode, SYNC_MODE_ONE_WAY_FROM_CLIENT) ||
        !strcmp(syncMode, SYNC_MODE_SMART_ONE_WAY_FROM_CLIENT)) {
        CString s1;
        s1.LoadString(IDS_UPLOAD_ONLY);
        ret += " (";
        ret += s1;
        ret += ")";
    }
    else if (!strcmp(syncMode, SYNC_MODE_ONE_WAY_FROM_SERVER) ||
             !strcmp(syncMode, SYNC_MODE_SMART_ONE_WAY_FROM_SERVER)) {
        CString s1;
        s1.LoadString(IDS_DOWNLOAD_ONLY);
        ret += " (";
        ret += s1;
        ret += ")";
    }

    return ret;
}


CButton* CSyncSettings::getCheckbox(const StringBuffer& sourceName) {
    
    if (sourceName.empty())          { return NULL; }
   
    if (sourceName == CONTACT_)      { return &checkContacts; }
    if (sourceName == APPOINTMENT_)  { return &checkCalendar; }
    if (sourceName == TASK_)         { return &checkTasks;    }
    if (sourceName == NOTE_)         { return &checkNotes;    }
    if (sourceName == PICTURE_)      { return &checkPictures; }
    if (sourceName == VIDEO_)        { return &checkVideos;   }
    if (sourceName == FILES_)        { return &checkFiles;    }
    
    return NULL;
}

CButton* CSyncSettings::getButton(const StringBuffer& sourceName) {
    
    if (sourceName.empty())          { return NULL; }
   
    if (sourceName == CONTACT_)      { return &butContacts; }
    if (sourceName == APPOINTMENT_)  { return &butCalendar; }
    if (sourceName == TASK_)         { return &butTasks;    }
    if (sourceName == NOTE_)         { return &butNotes;    }
    if (sourceName == PICTURE_)      { return &butPictures; }
    if (sourceName == VIDEO_)        { return &butVideos;   }
    if (sourceName == FILES_)        { return &butFiles;    }
    
    return NULL;
}

CWnd* CSyncSettings::getSeparator(const int index) {

    switch (index) {
        case 1:   return GetDlgItem(IDC_SEPARATOR_1);
        case 2:   return GetDlgItem(IDC_SEPARATOR_2);
        case 3:   return GetDlgItem(IDC_SEPARATOR_3);
        case 4:   return GetDlgItem(IDC_SEPARATOR_4);
        case 5:   return GetDlgItem(IDC_SEPARATOR_5);
        case 6:   return GetDlgItem(IDC_SEPARATOR_6);
        default:  return NULL;
    }
}

void CSyncSettings::hideAllSources() 
{
    checkContacts.ShowWindow(SW_HIDE);
    checkCalendar.ShowWindow(SW_HIDE);
    checkTasks.ShowWindow(SW_HIDE);
    checkNotes.ShowWindow(SW_HIDE);
    checkPictures.ShowWindow(SW_HIDE);
    checkVideos.ShowWindow(SW_HIDE);
    checkFiles.ShowWindow(SW_HIDE);

    butContacts.ShowWindow(SW_HIDE);
    butCalendar.ShowWindow(SW_HIDE);
    butTasks.ShowWindow(SW_HIDE);
    butNotes.ShowWindow(SW_HIDE);
    butPictures.ShowWindow(SW_HIDE);
    butVideos.ShowWindow(SW_HIDE);
    butFiles.ShowWindow(SW_HIDE);

    GetDlgItem(IDC_SEPARATOR_1)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SEPARATOR_2)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SEPARATOR_3)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SEPARATOR_4)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SEPARATOR_5)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SEPARATOR_6)->ShowWindow(SW_HIDE);
}

// not used anymore
void CSyncSettings::disableSource(CButton& button1, CButton& button2, bool* synctype, int sep1, int sep2) {
    
    button1.EnableWindow(FALSE);
    button2.EnableWindow(FALSE);
    *synctype = false;
    GetDlgItem(sep1)->EnableWindow(FALSE);
    if (sep2 > 0) {
        GetDlgItem(sep2)->EnableWindow(FALSE);
    }
}

// not used anymore
void CSyncSettings::hideSource(CButton& button1, CButton& button2, bool* synctype, int sep1, int sep2) {
    
    button1.ShowWindow(SW_HIDE);
    button2.ShowWindow(SW_HIDE);
    *synctype = false;
    GetDlgItem(sep1)->ShowWindow(SW_HIDE);
    if (sep2 > 0) {
        GetDlgItem(sep2)->ShowWindow(SW_HIDE);
    }
}