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

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "OutlookPlugindoc.h"
#include "LeftView.h"
#include "SyncForm.h"
#include "MainSyncFrm.h"
#include "AccountSettings.h"
#include "FullSync.h"
#include "LogSettings.h"
#include "AnimatedIcon.h"

#include "ClientUtil.h"
#include "utils.h"
#include "SyncException.h"

#include "HwndFunctions.h"
#include "comutil.h"
#include "Popup.h"
#include "UICustomization.h"
#include "MediaHubSetting.h"
#include <Shlwapi.h>

#include "sapi/SapiMediaRequestManager.h"




#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "winmaincpp.h"

#include "Tlhelp32.h"

/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CMainSyncFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainSyncFrame, CFrameWnd)

    ON_WM_CREATE()
    ON_WM_NCACTIVATE()
    ON_WM_CLOSE()
    ON_WM_INITMENUPOPUP()

    ON_MESSAGE(ID_MYMSG_SYNC_BEGIN,             &CMainSyncFrame::OnMsgSyncBegin)
    ON_MESSAGE(ID_MYMSG_SYNC_END,               &CMainSyncFrame::OnMsgSyncEnd)
    ON_MESSAGE(ID_MYMSG_SYNCSOURCE_BEGIN,       &CMainSyncFrame::OnMsgSyncSourceBegin)
    ON_MESSAGE(ID_MYMSG_SYNCSOURCE_END,         &CMainSyncFrame::OnMsgSyncSourceEnd)
    ON_MESSAGE(ID_MYMSG_SYNC_ITEM_SYNCED,       &CMainSyncFrame::OnMsgItemSynced)
    ON_MESSAGE(ID_MYMSG_SYNC_TOTALITEMS,        &CMainSyncFrame::OnMsgTotalItems)
    ON_MESSAGE(ID_MYMSG_STARTSYNC_ENDED,        &CMainSyncFrame::OnMsgStartsyncEnded)
    ON_MESSAGE(ID_MYMSG_REFRESH_STATUSBAR,      &CMainSyncFrame::OnMsgRefreshStatusBar)
    ON_MESSAGE(ID_MYMSG_CANCEL_SYNC,            &CMainSyncFrame::CancelSync)
    ON_COMMAND(ID_FILE_CONFIGURATION,           &CMainSyncFrame::OnFileConfiguration)
    ON_COMMAND(ID_TOOLS_FULLSYNC,               &CMainSyncFrame::OnToolsFullSync)
    ON_COMMAND(ID_FILE_SYNCHRONIZE,             &CMainSyncFrame::OnFileSynchronize)
    ON_COMMAND(ID_TOOLS_SETLOGLEVEL,            &CMainSyncFrame::OnToolsSetloglevel)

    ON_MESSAGE(ID_MYMSG_SAPI_PROGRESS,          &CMainSyncFrame::OnMsgSapiProgress)
    ON_MESSAGE(ID_MYMSG_POPUP,                  &CMainSyncFrame::OnMsgPopup)
    ON_MESSAGE(ID_MYMSG_OK,                     &CMainSyncFrame::OnOKMsg)
    ON_MESSAGE(ID_MYMSG_CHECK_MEDIA_HUB_FOLDER, &CMainSyncFrame::OnCheckMediaHubFolder)
	ON_MESSAGE(ID_MYMSG_SCHEDULER_DISABLED,     &CMainSyncFrame::OnMsgSchedulerDisabled)
	ON_MESSAGE(ID_MYMSG_REFRESH_SOURCES,		&CMainSyncFrame::OnMsgRefreshSources)

	// SAPI login msg
	ON_MESSAGE(ID_MYMSG_SAPILOGIN_BEGIN,		&CMainSyncFrame::OnMsgSapiLoginBegin)
	ON_MESSAGE(ID_MYMSG_SAPILOGIN_ENDED,		&CMainSyncFrame::OnMsgSAPILoginEnded)

	// SAPI Restore charge management
	ON_MESSAGE(ID_MYMSG_SAPI_RESTORE_CHARGE_BEGIN,	 &CMainSyncFrame::OnMsgSapiRestoreChargeBegin)
	ON_MESSAGE(ID_MYMSG_DOSAPI_RESTORE_CHARGE_ENDED, &CMainSyncFrame::OnMsgSapiRestoreChargeEnded)

    ON_COMMAND(ID_MENU_TEST_POPUPS, &CMainSyncFrame::OnTestPopups)

END_MESSAGE_MAP()


static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	//ID_INDICATOR_CAPS,
	//ID_INDICATOR_NUM,
	//ID_INDICATOR_SCRL,
};


// Flag used to lock/unlock the statusbar (and other objects).
// During canceling sync, we don't want bar to be updated / object enabled.
bool cancelingSync = false;


/**
 * Function used to refresh the statusbar.
 * Statusbar is not updated if locked by the flag 'cancelingSync'.
 */
void refreshStatusBar(CString& msg) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer tmp, msglog;
        tmp.convert(msg.GetBuffer());
        msglog.sprintf("%s: canceling=%d, msg = %s", __FUNCTION__, cancelingSync, tmp.c_str());
        printLog(msglog.c_str(), LOG_DEBUG);
    }

    // Avoid updating the statusbar when canceling sync.
    if (!cancelingSync && msg.GetLength()) {

        CMainSyncFrame* mainForm = (CMainSyncFrame*)AfxGetMainWnd();
        if (mainForm) {
            mainForm->wndStatusBar.SetPaneText(0, msg);
        }
    }
}

void refreshStatusBar(const int resourceID) {
    CString s1;
    s1.LoadString(resourceID);
    refreshStatusBar(s1);
}



/////////////////////////////////////////////////////////////////////////////

CMainSyncFrame::CMainSyncFrame() {

    hSyncThread = NULL;
    hLoginThread = NULL;
    dwThreadId  = NULL;
    configOpened = false;
    cancelingSync = false;
    dpiX = 0;
    dpiY = 0;
    itemTotalSize = 0;
    partialCompleted = 0;
    progressStarted = false;

    currentClientItem = 0;
    currentServerItem = 0;
    totalClientItems = 0;
    totalServerItems = 0;
    currentSource = 0;

    testingStatusText = 0;
}

CMainSyncFrame::~CMainSyncFrame() {
    if (dwThreadId) {
        CloseHandle(hSyncThread);
    }
}

int CMainSyncFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (!wndStatusBar.Create(this) ||
		!wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

    // TODO: hide splitter here
    EnableDocking(CBRS_ALIGN_ANY);
    wndSplitter.SetActivePane(0,1);
    wndSplitter.SetColumnInfo(0,0,0);
    RecalcLayout();
    SetWindowText(WPROGRAM_NAME);

	bSchedulerWasDisabledByLogin = false;

    // The syncForm, used to update main screen UI objects
    syncForm = (CSyncForm*)wndSplitter.GetPane(0,1);
    if (!syncForm) {
        printLog("UI error: NULL syncForm!", LOG_ERROR);
        return -1;
    }

	return 0;
}


void CMainSyncFrame::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) {

    if (!pPopupMenu) {
        goto finally;
    }

    if (!VIEW_USER_GUIDE_LINK) {
        //
        // Menu index: 0 = File, 1 = Tools, 2 = Help
        //
        if (nIndex == 2) {
            UINT firstItemID = pPopupMenu->GetMenuItemID(0);
            if (firstItemID == ID_VIEW_GUIDE) {
                // remove view User guide & separator
                pPopupMenu->RemoveMenu(0, MF_BYCOMMAND  | MF_BYPOSITION);
                pPopupMenu->RemoveMenu(0, MF_BYPOSITION | MF_SEPARATOR);
            }
        }
    }

    // "Software update" menu
    if (nIndex == 2 && isNewSwVersionAvailable()) {
        UINT firstItemID = pPopupMenu->GetMenuItemID(0);
        if (firstItemID != ID_MENU_UPDATE_SW) {
            CString s1;
            s1.LoadString(IDS_UPDATE_SOFTWARE);
            pPopupMenu->InsertMenu(0, MF_BYPOSITION | MF_ENABLED, ID_MENU_UPDATE_SW, s1);
            // pPopupMenu->EnableMenuItem(ID_MENU_UPDATE_SW, MF_GRAYED);
        }
    }

    // "test popups" menu
    if (nIndex == 2 && TEST_POPUPS) {
        UINT firstItemID = pPopupMenu->GetMenuItemID(0);
        if (firstItemID != ID_MENU_TEST_POPUPS) {
            CString s1 = "Test popups";
            pPopupMenu->InsertMenu(0, MF_BYPOSITION | MF_ENABLED, ID_MENU_TEST_POPUPS, s1);
            // pPopupMenu->EnableMenuItem(ID_MENU_TEST_POPUPS, MF_GRAYED);
        }
    }

finally:
    CFrameWnd::OnInitMenuPopup(pPopupMenu, nIndex, bSysMenu);
}


BOOL CMainSyncFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CFrameWnd::PreCreateWindow(cs) )
        return FALSE;

    // TODO: set here main window size and style
    cs.style =  WS_SYSMENU  | WS_VISIBLE | WS_MINIMIZEBOX;
    // cs.dwExStyle = 0 ;
    HDC hdc = ::GetDC(0);
    dpiX = ::GetDeviceCaps(hdc,LOGPIXELSX);
    dpiY = ::GetDeviceCaps(hdc,LOGPIXELSY);
    ::ReleaseDC(0,hdc);

    // Get the size dynamically (checks the sources number).
    CPoint size = getMainWindowSize();
    cs.cx = size.x;
    cs.cy = size.y;

    // Center window
    cs.x = (GetSystemMetrics(SM_CXSCREEN) - cs.cx)/2;
    cs.y = (GetSystemMetrics(SM_CYSCREEN) - cs.cy)/2;

    // Set the class name, previously registered to be now used.
    // Class name is important to correctly use FindWindow() function.
    cs.lpszClass = PLUGIN_UI_CLASSNAME;

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// diagnostics
#ifdef _DEBUG
void CMainSyncFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}
void CMainSyncFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}
#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////


BOOL CMainSyncFrame::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
    if (!wndSplitter.CreateStatic(this,1,2,WS_CHILD | WS_VISIBLE | WS_MINIMIZEBOX))
	{
		TRACE(_T("failed to create the splitter"));
		return FALSE;
	}

    if (!wndSplitter.CreateView(0,0,RUNTIME_CLASS(CSyncForm),CSize(100,100),pContext))
	{
		TRACE(_T("Failed to create view in first pane"));
		return FALSE;
	}

    if (!wndSplitter.CreateView(0,1,RUNTIME_CLASS(CSyncForm),CSize(100,100),pContext))
	{
		TRACE(_T("failed to create view in second pane"));
		return FALSE;
	}

	return TRUE;
}

void CMainSyncFrame::OnFileConfiguration()
{
    if (checkConnectionSettings()) {
        // show config: Sync settings (default)
        showSettingsWindow(1);
    }
    else {
        // show config: Account settings
        showSettingsWindow(0);
    }
}


void CMainSyncFrame::OnToolsFullSync()
{
    // if sync is in progress we don't open the recover panel
    if(checkSyncInProgress()){
        CString s1;
        s1.LoadString(IDS_ERROR_CANNOT_CHANGE_SETTINGS);
        wsafeMessageBox(s1);
        return;
    }

    // show full sync dialog
    CFullSync wndFullSync;
    INT_PTR result = wndFullSync.DoModal();
}


/**
 * Thread to start the sync process.
 */
DWORD WINAPI syncThread(LPVOID lpParam) {

    int ret = 0;

    try {
        ret = startSync();
    }
    catch (SyncException* e) {
        // Catch SyncExceptions:
        //   code 2 = aborted by user (soft termination)
        //   code 3 = Outlook fatal exception
        StringBuffer msg;
        msg.sprintf("syncException received: code %d", e->getErrorCode());
        printLog(msg.c_str(), LOG_DEBUG);
        ret = e->getErrorCode();
    }
    catch (std::exception &e) {
        // Catch STL exceptions: code 7
        CStringA s1 = "Unexpected STL exception: ";
        s1.Append(e.what());
        printLog(s1.GetBuffer(), LOG_ERROR);
        ret = WIN_ERR_UNEXPECTED_STL_EXCEPTION;        // code 7
    }
    catch(...) {
        // Catch other unexpected exceptions.
        CStringA s1;
        s1.LoadString(IDS_UNEXPECTED_EXCEPTION);
        printLog(s1.GetBuffer(), LOG_ERROR);
        ret = WIN_ERR_UNEXPECTED_EXCEPTION;            // code 6
    }


    Sleep(200);     // Just to be sure that everything has been completed...
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_STARTSYNC_ENDED, NULL, (LPARAM)ret);


    if (ret) {
        // In case of COM exceptions, the COM library could not be
        // reused with 'CoInitialize()'. While terminating the thread like this seems
        // working fine...
        TerminateThread(GetCurrentThread(), ret);
    }
    return 0;
}

/**
 * Thread used to kill the syncThread after a timeout.
 * @param lpParam : the syncThread HANDLE
 */
DWORD WINAPI syncThreadKiller(LPVOID lpParam) {

    // Wait on the sync thread (timeout = 8sec)
    int ret = 0;
    HANDLE hSyncThread = lpParam;
    DWORD dwWaitResult = WaitForSingleObject(hSyncThread, TIME_OUT_ABORT * 1000);

    switch (dwWaitResult) {
        // Thread exited -> no need to kill it (should be the usual way).
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0: {
            ret = 0;
            break;
        }
        // Sync is still running after timeout -> kill thread.
        case WAIT_TIMEOUT: {
            hardTerminateSync(hSyncThread);
            SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_STARTSYNC_ENDED, NULL, (LPARAM)4);
            ret = 0;
            break;
        }
        // Some error occurred (case WAIT_FAILED)
        default: {
            ret = 1;
            break;
        }
    }

    // To enable again the refresh of statusbar.
    cancelingSync = false;

    return ret;
}


/**
 * Thread to start the SAPI login process
 */
DWORD WINAPI loginThread(LPVOID lpParam) {

    int ret = 0;

	SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPILOGIN_BEGIN, NULL, NULL);

    try {
        ret = doSapiLoginThread(); // ret is a ESapiMediaRequestStatus from ESapiMediaRequest login()
    }
    catch (SyncException* e) {
        // Catch SyncExceptions:
        //   code 2 = aborted by user (soft termination)
        ret = e->getErrorCode();
    }
    catch (std::exception &e) {
        // Catch STL exceptions: code 7
        CStringA s1 = "Unexpected STL exception: ";
        s1.Append(e.what());
        printLog(s1.GetBuffer(), LOG_ERROR);
        ret = WIN_ERR_UNEXPECTED_STL_EXCEPTION;        // code 7
    }
    catch(...) {
        // Catch other unexpected exceptions.
        CStringA s1;
        s1.LoadString(IDS_UNEXPECTED_EXCEPTION);
        printLog(s1.GetBuffer(), LOG_ERROR);
        ret = WIN_ERR_UNEXPECTED_EXCEPTION;            // code 6
    }

	// update UI
	SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_SOURCES, NULL, NULL);

	Sleep(200);     // Just to be sure that everything has been completed...

	SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPILOGIN_ENDED, NULL, (LPARAM)ret);

    if (ret) {
        // **** Investigate on this ****
        // In case of COM exceptions, the COM library could not be
        // reused with 'CoInitialize()'. While terminating the thread like this seems
        // working fine...
        TerminateThread(GetCurrentThread(), ret);
    }
    return 0;
}


/**
 * Thread used to call the sapi for restore charge.
 *
 */
DWORD WINAPI callSAPIRestoreChargeThread(LPVOID lpParam) {
    int ret = 0;

    try {
        ret = doSAPIRestoreCharge(); // in winmain.cpp
    }
    catch (SyncException* e) {
        // Catch SyncExceptions:
        //   code 2 = aborted by user (soft termination)
        ret = e->getErrorCode();
    }
    catch (std::exception &e) {
        // Catch STL exceptions: code 7
        CStringA s1 = "Unexpected STL exception: ";
        s1.Append(e.what());
        printLog(s1.GetBuffer(), LOG_ERROR);
        ret = WIN_ERR_UNEXPECTED_STL_EXCEPTION;        // code 7
    }
    catch(...) {
        // Catch other unexpected exceptions.
        CStringA s1;
        s1.LoadString(IDS_UNEXPECTED_EXCEPTION);
        printLog(s1.GetBuffer(), LOG_ERROR);
        ret = WIN_ERR_UNEXPECTED_EXCEPTION;            // code 6
    }


    Sleep(200);     // Just to be sure that everything has been completed...

    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_DOSAPI_RESTORE_CHARGE_ENDED, NULL, (LPARAM)ret);

    if (ret) {
        // **** Investigate on this ****
        // In case of COM exceptions, the COM library could not be
        // reused with 'CoInitialize()'. While terminating the thread like this seems
        // working fine...
        TerminateThread(GetCurrentThread(), ret);
    }
    return 0;
}


/**
 * Thread used to kill the loginThread after a timeout.
 * @param lpParam : the syncThread HANDLE
 */
DWORD WINAPI loginThreadKiller(LPVOID lpParam) {

    // Wait on the target thread (timeout = custom)
    int ret = 0;
    HANDLE hTargetThread = lpParam;
    DWORD dwWaitResult = WaitForSingleObject(hTargetThread, LOGIN_TIMEOUT * 1000);

    switch (dwWaitResult) {
        // Thread exited -> no need to kill it (should be the usual way).
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0: {
            ret = 0;
            break;
        }
        // Target thread is still running after timeout -> kill it.
        case WAIT_TIMEOUT: {
            TerminateThread(hTargetThread, 0);
	        SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_SOURCES, NULL, NULL);
	        SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPILOGIN_ENDED, NULL, (LPARAM)ESMRRequestTimeout);
            ret = 0;
            break;
        }
        // Some error occurred (case WAIT_FAILED)
        default: {
            ret = 1;
            break;
        }
    }
    return ret;
}


/**
 * Thread used to kill the callSAPIRestoreCall after a timeout.
 * @param lpParam : the syncThread HANDLE
 */
DWORD WINAPI callSAPIRestoreKiller(LPVOID lpParam) {

    // Wait on the target thread (timeout = custom)
    int ret = 0;
    HANDLE hTargetThread = lpParam;
    DWORD dwWaitResult = WaitForSingleObject(hTargetThread, RESTORE_CHARGE_TIMEOUT * 1000);

    switch (dwWaitResult) {
        // Thread exited -> no need to kill it (should be the usual way).
        case WAIT_ABANDONED:
        case WAIT_OBJECT_0: {
            ret = 0;
            break;
        }
        // Target thread is still running after timeout -> kill it.
        case WAIT_TIMEOUT: {
            TerminateThread(hTargetThread, 0);
            SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_DOSAPI_RESTORE_CHARGE_ENDED, NULL, (LPARAM)ret);
            ret = 0;
            break;
        }
        // Some error occurred (case WAIT_FAILED)
        default: {
            ret = 1;
            break;
        }
    }
    return ret;
}




void CMainSyncFrame::OnFileSynchronize() {

    CString s1;
    if(  (!checkSyncInProgress()) ){
        // No sync in progress -> StartSync.
        StartSync();
    }
    else{
        if (getConfig()->getScheduledSync()) {
            // It's running a scheduled sync -> error msg.
            s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
            wsafeMessageBox(s1);
        }
    }
}



void CMainSyncFrame::showSettingsWindow(const int paneToDisplay){

    if(checkSyncInProgress()){
        CString s1;
        s1.LoadString(IDS_ERROR_CANNOT_CHANGE_SETTINGS);
        wsafeMessageBox(s1);
        return;
    }

    CDocument* pDoc = NULL;
    pConfigFrame = NULL;

    CSingleDocTemplate* docSettings = ((COutlookPluginApp*)AfxGetApp())->docSettings;


	lastSyncURL      = getConfig()->getAccessConfig().getSyncURL();
	lastUserName     = getConfig()->getAccessConfig().getUsername();
	lastUserPassword = getConfig()->getAccessConfig().getPassword();

    pDoc = docSettings->CreateNewDocument();
    if (pDoc != NULL)
    {
        // If creation worked, use create a new frame for
        // that document.
        pConfigFrame = (CConfigFrame*)docSettings->CreateNewFrame(pDoc, NULL);

        if (pConfigFrame != NULL)
        {
            docSettings->SetDefaultTitle(pDoc);

            // If document initialization fails
            if (!pDoc->OnNewDocument())
            {
                pConfigFrame->DestroyWindow();
                pConfigFrame = NULL;
            }
            //else
            //{
            //    docSettings->InitialUpdateFrame(pConfigFrame, pDoc, TRUE);
            //}
        }
    }

    // if it failed
    if (pConfigFrame == NULL || pDoc == NULL)
    {
        delete pDoc;
        AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);
    }

    pConfigFrame->wndSplitter.SetActivePane(0,0);
    pConfigFrame->wndSplitter.SetColumnInfo(0,65,40);
    //pConfigFrame->wndSplitter.RecalcLayout();
    docSettings->InitialUpdateFrame(pConfigFrame, pDoc, TRUE);


    //select the desired pane to be displayed.
    ((CLeftView*)pConfigFrame->wndSplitter.GetPane(0,0))->selectItem(paneToDisplay);

    pConfigFrame->wndSplitter.GetPane(0,1)->SendMessage(WM_PAINT);

    this->BeginModalState(); // this is required
    configOpened = true;
}

void CMainSyncFrame::OnToolsSetloglevel()
{
    // show the Log Level dialog
    CLogSettings wndLog;
    wndLog.DoModal();
}



LRESULT CMainSyncFrame::OnMsgSyncBegin(WPARAM , LPARAM lParam) {

    refreshStatusBar(IDS_TEXT_STARTING_SYNC);

    // hide the menu
    showMenu(false);

    progressStarted = false;
	bSchedulerWasDisabledByLogin = false;
    Invalidate();

    return 0;
}

// UI received a sync end message
LRESULT CMainSyncFrame::OnMsgSyncEnd( WPARAM , LPARAM ) {
    if (UICustomization::verboseUIDebugging) {
        printLog("msg syncEnd received by UI", LOG_DEBUG);
    }

    refreshStatusBar(IDS_TEXT_SYNC_ENDED);

    progressStarted = false;

	// show a message that alert the user  (?)
	if ( bSchedulerWasDisabledByLogin ) {
		//s1.LoadString(IDS_TEXT_SCHEDULER_DISABLED);
        //wsafeMessageBox(s1);
	}
	bSchedulerWasDisabledByLogin = false;
	return 0;
}

// UI received sync source begin message
LRESULT CMainSyncFrame::OnMsgSyncSourceBegin(WPARAM wParam, LPARAM lParam) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer tmp;
        tmp.sprintf("%s: lParam = %d", __FUNCTION__, lParam);
        printLog(tmp.c_str(), LOG_DEBUG);
    }

    CString s1;
    currentSource = lParam;
    currentClientItem = 0;
    currentServerItem = 0;

    //
    // sets the source to SYNC state (starts animation)
    //
    syncForm->onSyncSourceBegin(currentSource);


    // change statusbar &status to: "Connecting..."
    syncForm->refreshSourceStatus(IDS_CONNECTING, currentSource);
    refreshStatusBar(IDS_CONNECTING);

    return 0;
}

// UI received a item synced message
LRESULT CMainSyncFrame::OnMsgItemSynced( WPARAM wParam, LPARAM lParam) {

    if (!currentSource) {
        // it may happen if msg arrives from a scheduled sync
        currentSource = lParam;
        syncForm->onSyncSourceBegin(currentSource);
    }
    
    int currentItem = 0;
    int totalItems = 0;
    CString statusBarText;

    if(wParam == -1) {        
        totalItems = totalClientItems;
        currentClientItem ++;
        currentItem = currentClientItem;        
        CString ci, ti;
        ci.Format(L"%i", currentItem);
        ti.Format(L"%i", totalItems);
        if(totalItems > 0)  {
            statusBarText.FormatMessage(IDS_SENDING_XY, ci, ti);            
        } else {
            statusBarText.FormatMessage(IDS_SENDING_X, ci);                    
        }
    }
    else {
        totalItems = totalServerItems;
        currentServerItem ++;
        currentItem = currentServerItem;
        
        CString ci, ti;
        ci.Format(L"%i", currentItem);
        ti.Format(L"%i", totalItems);
        if(totalItems > 0)  {
            statusBarText.FormatMessage(IDS_RECEIVING_XY, ci, ti);            
        } else {
            statusBarText.FormatMessage(IDS_RECEIVING_X, ci);                    
        }
    }
   
    refreshStatusBar(statusBarText);

    // refresh source pane status
    syncForm->refreshSourceStatus(statusBarText, currentSource);


    return 0;
}



afx_msg LRESULT CMainSyncFrame::OnMsgRefreshStatusBar( WPARAM wParam, LPARAM lParam) {

    //
    // *** TODO: use UI string resources!!!! ***
    //

    CString s1;
    WCHAR tmp[20];
    switch (lParam) {
        case SBAR_CHECK_ALL_ITEMS: {
            wsprintf(tmp, TEXT("%d"), (int)wParam);
            s1.FormatMessage(IDS_READING_ALLITEMS, (int)tmp);
            break;
        }
        case SBAR_CHECK_MOD_ITEMS: {
            s1.LoadString(IDS_CHECKING_MODITEMS);
            break;
        }
        case SBAR_CHECK_MOD_ITEMS2: {
            wsprintf(tmp, TEXT("%d"), (int)wParam);
            s1.FormatMessage(IDS_CHECKING_MODITEMS2, tmp);    
            break;
        }       
        case SBAR_DELETE_CLIENT_ITEMS: {

            CString src = getLabelStringFromID(currentSource);
            s1.FormatMessage(IDS_DELETING_ITEMS, src);
            break;
        }
        case SBAR_SENDDATA_BEGIN: {
            s1.LoadString(IDS_SENDING_DATA);
            break;
        }
        case SBAR_RECEIVE_DATA_BEGIN: {
            s1.LoadString(IDS_RECEIVING_DATA);
            
            break;
        }        
        case SBAR_ENDING_SYNC: {
            s1.LoadString(IDS_ENDING_SYNC);
            refreshStatusBar(s1);
            syncForm->refreshSourceStatus(s1, currentSource);
            return 0;
    }
    }

    
    refreshStatusBar(s1);

    // Refresh source labels for some case
    // Not for media, because items are big and we need to keep the items' number on the source pane.
    if (currentSource != SYNCSOURCE_PICTURES &&
        currentSource != SYNCSOURCE_VIDEOS   &&
        currentSource != SYNCSOURCE_FILES) {
        if ( lParam == SBAR_SENDDATA_BEGIN ||
             lParam == SBAR_RECEIVE_DATA_BEGIN ||
             lParam == SBAR_DELETE_CLIENT_ITEMS ) {

            syncForm->refreshSourceStatus(s1, currentSource);
        }
    }

    return 0;
}



afx_msg LRESULT CMainSyncFrame::OnMsgTotalItems( WPARAM wParam, LPARAM lParam) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: wParam = %d, lParam = %d", __FUNCTION__, wParam, lParam);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    if (wParam == 0) {
        totalClientItems = lParam;
    } else {
        totalServerItems = lParam;
    }

    return 0;
}

// the config window has closed, and the user is returned to the main window
void CMainSyncFrame::OnConfigClosed() {

    EndModalState();
    SetForegroundWindow();
    configOpened = false;

	// checking if login settings was changed...
	if ( ( lastUserName     != getConfig()->getAccessConfig().getUsername() ) ||
		 ( lastUserPassword != getConfig()->getAccessConfig().getPassword() ) ||
		 ( lastSyncURL      != getConfig()->getAccessConfig().getSyncURL()  ) ) {

        if (ENABLE_LOGIN_ON_ACCOUNT_CHANGE) {
            //
            // starts the login thread that calls SAPI login.
            //
            StartLogin();

            // reset all sources timestamps and last errors
            resetAllSourcesTimestamps();

            // reset also the updater node!
            getConfig()->resetUpdaterConfig();
        }
	}

    syncForm->refreshSources();
}


LRESULT CMainSyncFrame::OnMsgRefreshSources(WPARAM wParam, LPARAM lParam)
{
	syncForm->refreshSources();
    return 0;
}



LRESULT CMainSyncFrame::OnMsgSapiRestoreChargeBegin(WPARAM wParam, LPARAM lParam)
{
	syncForm->refreshSources();
    return 0;
}



LRESULT CMainSyncFrame::OnMsgSapiRestoreChargeEnded(WPARAM wParam, LPARAM lParam)
{
	// lparam contains the SAPI result from thread
	int exitCode = lParam;

	syncForm->refreshSources();

	if ( exitCode == 0) { // ESRCSuccess
		setRefreshSourcesListToSync(false); // don't want to refresh the sources list to sync, mantain the old list.
		//
        // Restart SYNC
        //
        StartSync();
	}
    else {
        StringBuffer msg;
        msg.sprintf("SAPI restore charge failed, exit code = %d", exitCode);
        printLog(msg.c_str(), LOG_ERROR);

        //CString s1 = "";
		//s1.Format(_T("Sorry, server response code is: %d\nYou will not be able to use restore service."), exitCode);
		//int msgboxFlags = MB_OK | MB_ICONASTERISK | MB_SETFOREGROUND | MB_APPLMODAL;
		//int selected = wsafeMessageBox(s1.GetBuffer(), 0, msgboxFlags);
		//if (selected == IDYES ) { // managing YES / NO ?
		//	// do something if yes
		//}
	}
    return 0;
}



LRESULT CMainSyncFrame::OnMsgSapiLoginBegin(WPARAM wParam, LPARAM lParam)
{
    refreshStatusBar(IDS_LOGGING_IN);

    // lock UI during sapi login (also lock syncAll pane?)
    syncForm->lockButtons();

    return 0;
}


/**
 * Message received when SAPI Login thread has exited.
 * 'lParam' is the exitThread code (0 if no errors).
 * Here errors of sync process are managed, and then UI refreshed.
 */
LRESULT CMainSyncFrame::OnMsgSAPILoginEnded(WPARAM wParam, LPARAM lParam) {

    CString s1;
    int exitCode = lParam;
    StringBuffer msg;

    // Sync has finished: unlock buttons
	cancelingSync = false;

    syncForm->unlockButtons();

    //
    // Error occurred: display error message on status bar. @#@#
    //
	bool changeStatusBar=false;

	switch (exitCode) {
		// shows a message in the status bar
		case ESMRSuccess:
			s1.LoadString(IDS_LOGIN_SUCCESSFUL);
			changeStatusBar = true;
			break;
		case ESMRAccessDenied:
			s1.LoadString(IDS_LOGIN_AUTH_FAILED);
			changeStatusBar = true;
			break;

		// no messages in status bar (back compatibility)
		case ESMRGenericHttpError:
            msg.sprintf("SAPI login: Generic HTTP error: %d", exitCode );
            printLog(msg.c_str(), LOG_ERROR);
			break;
		case ESMRHTTPFunctionalityNotSupported:
            msg.sprintf("SAPI login: functionality not supported: %d", exitCode );
            printLog(msg.c_str(), LOG_ERROR);
			break;
        case ESMRRequestTimeout:
            printLog("SAPI Login error, request timeout", LOG_ERROR);
			break;
		default:
            msg.sprintf("SAPI login: generic error: %d", exitCode );
            printLog(msg.c_str(), LOG_ERROR);
			break;
	}

	if (!changeStatusBar) {
		s1.LoadString(AFX_IDS_IDLEMESSAGE);
	}
	refreshStatusBar(s1);


    // show the menu
    showMenu(true);


    // Refresh sources.
    syncForm->refreshSources();

	SetForegroundWindow();
    Invalidate(FALSE);
    currentSource = 0;          // Invalidating the currentSource, here it's finished.
    progressStarted = false;
    return 0;
}



/**
 * Message received when sync thread has exited.
 * 'lParam' is the exitThread code (0 if no errors).
 * Here errors of sync process are managed, and then UI refreshed.
 */
LRESULT CMainSyncFrame::OnMsgStartsyncEnded(WPARAM wParam, LPARAM lParam) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer tmp;
        tmp.sprintf("%s: lParam = %d", __FUNCTION__, lParam);
        printLog(tmp.c_str(), LOG_DEBUG);
    }

    int exitCode = lParam;
    const bool isScheduled = getConfig()->getScheduledSync();

    cancelingSync = false;

    //
    // Error occurred: display error message on a msgBox.
    //
    if (exitCode != 0) {
        BeginModalState();
        manageSyncErrorMsg(exitCode);
        EndModalState();
        SetForegroundWindow();
    }

    // for these errors, force exit the client
    if (exitCode == WIN_ERR_FATAL_OL_EXCEPTION ||
        exitCode == WIN_ERR_THREAD_TERMINATED) {
        exit(1);
    }

    //
    // Open settings window if error on invalid credentials.
    //
    if ( (!isScheduled) &&
         (exitCode == 407  ||                   // 407  = Auth failed
          exitCode == 401  ||                   // 401  = Wrong credentials
          exitCode == 404  ||                   // 404  = not found
          exitCode == 2001 ||                   // 2001 = HTTP connection error
          exitCode == 2060 ||                   // 2060 = HTTP resource not found (status 404)
          exitCode == 2102) ) {                 // 2102 = No sources to sync

        if (exitCode == 404 ||
            exitCode == 2102) {
            showSettingsWindow(1);              // -> show Sync settings
        }
        else  {
            showSettingsWindow(0);              // -> show Account settings
        }

	}

    // messagebox alerting payment required for restore
	//-----------------------------------------------------------------------------------------
    if (ENABLE_PAYMENT_REQUIRED_CHARGE) {
	    UINT msgboxFlags = 0;
	    int  selected = 0;

	    setRefreshSourcesListToSync(true);
	    if ( (!isScheduled) && ( exitCode == WIN_ERR_PAYMENT_REQUIRED ) ) {

		    // interactive messagebox @#@#@#
            // **** TODO: use string resources! ****
		    CString s1 = "Warning, a payment is required for performing restore!\r\nIf you continue a charge will be applied on you account.\r\n Do you want continue?";
		    msgboxFlags = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_APPLMODAL;
		    selected = wsafeMessageBox(s1.GetBuffer(), 0, msgboxFlags);
		    if (selected == IDYES ) {
			    RestoreCharge(); // SAPI call to charge and then restart restore.
		    }
	    }
    }
	//-----------------------------------------------------------------------------------------


    refreshStatusBar(IDS_TEXT_SYNC_ENDED);

    // To make sure the config in memory is updated, in case the sync was interrupted
    // in an unexpected way
    getConfig()->read();

    // unlock and refresh all UI panes
    syncForm->onSyncEnded();


    // show the menu
    showMenu(true);

    SetForegroundWindow();

    currentSource = 0;          // Invalidating the currentSource, here it's finished.
    progressStarted = false;
    return 0;
}




void CMainSyncFrame::StartSync(const int sourceID) {

    StringBuffer tmp;
    tmp.sprintf("\n\n--- %s: sourceID = %d ---", __FUNCTION__, sourceID);
    printLog(tmp.c_str(), LOG_DEBUG);

    //test all status text
    if (TEST_POPUPS && testingStatusText) {
        testAllStatusText(sourceID);
        return;
    }

    CString s1;

    // Check on sync in progress.
    if (checkSyncInProgress()) {
        printLog("sync already in progress, exiting", LOG_DEBUG);
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
        return;
    }

    // Check if connection settings are valid.
    if(! checkConnectionSettings()) {
        printLog("missing credentials, exiting to account screen", LOG_DEBUG);
        s1.LoadString(IDS_ERROR_SET_CONNECTION);
        wsafeMessageBox(s1);
        showSettingsWindow(0);          // 0 = 'Account Settings' pane.
        return;
    }

    // UI is unlocked by OnMsgStartSyncEnded()
    if (syncForm->isUILocked()) {
        printLog("UI still locked, exiting", LOG_DEBUG);
        return;
    }

    // Hide the menu.
    showMenu(false);

    currentSource = 0;
    currentClientItem = 0;
    currentServerItem = 0;
    totalClientItems = 0;
    totalServerItems = 0;

    //
    // locks and refreshes all UI panes
    //
    if (sourceID == -1) {
        syncForm->onSyncAllStarted();
    } else {
        syncForm->onSyncStarted(sourceID);
    }


    //
    // Start the sync thread.
    //
    printLog("Starting the syncThread...", LOG_DEBUG);
    cancelingSync = false;
    hSyncThread = CreateThread(NULL, 0, syncThread, 0, 0, &dwThreadId);
    if (hSyncThread == NULL) {
        DWORD errorCode = GetLastError();
        CString s1 = "Thread error: syncThread";
        wsafeMessageBox(s1);
        return;
    }
}

LRESULT CMainSyncFrame::CancelSync(WPARAM wParam, LPARAM lParam){
    return CancelSync(false);
}

int CMainSyncFrame::CancelSync(bool confirm) {

    printLog("User requested to cancel sync", LOG_INFO);
    int ret = 1;
    CString msg;
    CString s1;

    // This will avoid clicking 2 times on cancel sync.
    if (cancelingSync) {
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
        return ret;
    }

    if (getConfig()->getScheduledSync()) {
        //
        // Can't stop a scheduled sync in the usual way (it's a different process)
        // (TBD: find process and kill it?)
        //
        s1.LoadString(IDS_TEXT_SYNC_IN_PROGRESS);
        int flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
        MessageBox(s1, WPROGRAM_NAME, flags);
        return ret;
    }


    //
    // Display warning.
    //
    int selected = IDYES;
    if (confirm) {
        unsigned int flags = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_APPLMODAL;
        CString s1; s1.LoadString(IDS_CANCEL_SYNC_IN_PROGRESS);
        selected = MessageBox(s1.GetString(), WPROGRAM_NAME, flags);
    }

    // First check again if sync is running (could be terminated in the meanwhile...)
    if (!checkSyncInProgress()) {
        printLog("Sync is no more running, exit", LOG_DEBUG);
        return ret;
    }

    if (selected == IDYES) {
        ret = 0;

        // Refresh status
        refreshStatusBar(IDS_TEXT_CANCELING_SYNC);
        syncForm->refreshSourceStatus(IDS_TEXT_CANCELING_SYNC, currentSource);

        // LOCK the statusbar and other controls.
        cancelingSync = true;
        progressStarted = false;

        syncForm->lockButtons();

        // First we try to terminate the sync in a soft way.
        softTerminateSync();

        // If it fails, this thread will kill the syncThread (after a timeout).
        DWORD killerThreadId;
        HANDLE h = CreateThread(NULL, 0, syncThreadKiller, hSyncThread, 0, &killerThreadId);

        syncForm->OnNcPaint();

        // show the menu
        showMenu(true);

        Invalidate();
        currentSource = 0;
        currentClientItem = 0;
        currentServerItem = 0;
        totalClientItems = 0;
        totalServerItems = 0;
    }

    return ret;
}



void CMainSyncFrame::StartLogin() {

	int ret = 1;
    CString msg;
    CString s1;

    printLog("Start SAPI LOGIN", LOG_INFO);

    // Check on sync in progress.
    if (checkSyncInProgress()) {
        printLog("Can't start sapi login: sync already in progress", LOG_ERROR);
        return;
    }

    // Check if connection settings are valid.
    if(! checkConnectionSettings()) {
        s1.LoadString(IDS_ERROR_SET_CONNECTION);
        wsafeMessageBox(s1);
        showSettingsWindow(0);          // 0 = 'Account Settings' pane.
        return;
    }

    // Lock the UI buttons?
    //syncForm->lockButtons();

    // Hide the menu.
    showMenu(false);


    //
    // Start the login thread.
    //
    printLog("Starting SAPI Login thread...", LOG_DEBUG);

	hLoginThread = CreateThread(NULL, 0, loginThread, 0, 0, &dwThreadId);
    if (hLoginThread == NULL) {
        DWORD errorCode = GetLastError();
        CString s1 = "Thread error: loginThread";
        wsafeMessageBox(s1);
        return;
    }

    // To handle login thread timeout (see LOGIN_TIMEOUT)
    DWORD killerThreadId;
    HANDLE h = CreateThread(NULL, 0, loginThreadKiller, hLoginThread, 0, &killerThreadId);
}

// starts the thread for SAPI restore charge
void CMainSyncFrame::RestoreCharge() {

    CString s1;
    currentSource = 0;
    currentClientItem = 0;
    currentServerItem = 0;
    totalClientItems = 0;
    totalServerItems = 0;
    printLog("Start SAPI Restore charge call", LOG_DEBUG);

    // Check on sync in progress.
    if (checkSyncInProgress()) {
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
        return;
    }

    // Check if connection settings are valid.
    if(! checkConnectionSettings()) {
        s1.LoadString(IDS_ERROR_SET_CONNECTION);
        wsafeMessageBox(s1);
        showSettingsWindow(0);          // 0 = 'Account Settings' pane.
        return;
    }

    // Lock the UI buttons?
    //syncForm->lockButtons();

    // hide menu
    showMenu(false);


    //
    // Refresh of main UI.
    //
    syncForm->refreshSources();


    //
    // Start the SAPI Restore Charge Call thread.
    //
    printLog("Starting the thread for SAPI Charge...", LOG_DEBUG);
    cancelingSync = false;
    hSyncThread = CreateThread(NULL, 0, callSAPIRestoreChargeThread, 0, 0, &dwThreadId);
    if (hSyncThread == NULL) {
        DWORD errorCode = GetLastError();
        CString s1 = "Thread error: SAPI Restore Charge";
        wsafeMessageBox(s1);
        return;
    }

    // To handle SAPI Restore Charge thread timeout (see RESTORE_CHARGE_TIMEOUT)
    DWORD killerThreadId;
    HANDLE h = CreateThread(NULL, 0, callSAPIRestoreKiller, hSyncThread, 0, &killerThreadId);
}


void CMainSyncFrame::showMenu(bool show) {

    DWORD flag = MF_ENABLED;
    if (show) {
        printLog("Show menu", LOG_DEBUG);
    } else {
        printLog("Hide menu", LOG_DEBUG);
        flag = MF_GRAYED;
    }

    HMENU hMenu = ::GetMenu(GetSafeHwnd());
    int nCount = GetMenuItemCount(hMenu);
    for(int i=0; i < nCount; i++){
        EnableMenuItem(hMenu, i, MF_BYPOSITION | flag);
    }
    DrawMenuBar();
    UpdateWindow();
}


// handling for minimizing/restoring the UI when the config is opened
BOOL CMainSyncFrame::OnNcActivate(BOOL bActive) {

    // needs special handling only when the config window is opened
    if(configOpened){
        if( (bActive) && (pConfigFrame != NULL))
            if( (! this->IsWindowEnabled()) &&
                (pConfigFrame->IsWindowVisible() )
                //(pConfigFrame->IsWindowVisible()) //&&
                //(pConfigFrame->IsWindowEnabled())
                )
            {
                pConfigFrame->SetForegroundWindow();
                pConfigFrame->SetFocus();
            };
    };

    CFrameWnd::OnNcActivate(bActive);
    return TRUE;
}

void CMainSyncFrame::OnClose(){

    // CancelSync only if sync in progress AND not a scheduled one!
    // (if scheduled, the sync will continue in bkground)
    if( (checkSyncInProgress()) && (!getConfig()->getScheduledSync()) ) {

        if (CancelSync()) {
            // cancelled
            return;
        }
    }

    closeClient();
    CFrameWnd::OnClose();
}



bool CMainSyncFrame::checkConnectionSettings()
{
    // first check if the server URL is not empty
    if (!strcmp(getConfig()->getAccessConfig().getSyncURL(), "")) {
        return false;
    }

    if( (!strcmp(getConfig()->getAccessConfig().getUsername(), "")) ||
        (!strcmp(getConfig()->getAccessConfig().getPassword(), "")) ) {
        return false;
    }

    return true;
}


LRESULT CMainSyncFrame::OnMsgSyncSourceEnd(WPARAM wParam, LPARAM lParam) {

    StringBuffer tmp;
    tmp.sprintf("%s: wParam = %d", __FUNCTION__, wParam);
    printLog(tmp.c_str(), LOG_DEBUG);

    // updates the source pane (stops animation)
    syncForm->onSyncSourceEnd(currentSource);

    return 0;
}


LRESULT CMainSyncFrame::Synchronize(WPARAM wParam, LPARAM lParam){
    OnFileSynchronize();
    return NULL;
}


LRESULT CMainSyncFrame::OnOKMsg(WPARAM wParam, LPARAM lParam) {

    this->ShowWindow(SW_MINIMIZE);
    return 0;
}


LRESULT CMainSyncFrame::OnMsgPopup(WPARAM wParam, LPARAM lParam) {

    CString button1;
    CString button2;
    CString button3;
    CString swap;
    CString msg;
    CString buttonval;
    WCHAR* currentMsg;
    int sizeOfString;
    WCHAR*  buffer;
    wstring formattedDate;

    OutlookConfig* c = getConfig();
    if (c == NULL) {
        return 0;
    }

    UpdaterConfig& config = c->getUpdaterConfig();
    StringBuffer date = config.getReleaseDate();
    if (date.empty()) {
        c->readUpdaterConfig(true);
        config = c->getUpdaterConfig();
        date = config.getReleaseDate();
    }
    switch(wParam) {
        case TYPE_SKIPPED_ACTION:
            buttonval.LoadString(IDS_OK);
            msg.LoadString(IDS_UP_MESSAGE_SKIPPED);
            break;
        case TYPE_NOW_LATER_SKIP_OPTIONAL:
            buttonval.LoadString(IDS_BUT_NOW_LATER_SKIP);
            msg.LoadString(IDS_UP_MESSAGE);
            break;
        case TYPE_NOW_LATER_RECCOMENDED:
            buttonval.LoadString(IDS_BUT_NOW_LATER);
            msg.LoadString(IDS_UP_MESSAGE);
            break;
        case TYPE_NOW_LATER_MANDATORY:
            buttonval.LoadString(IDS_BUT_NOW_LATER);
            msg.LoadString(IDS_UP_MANDATORY_MESSAGE);
            sizeOfString = (msg.GetLength() + 1);
            buffer = new WCHAR[sizeOfString];
            wcsncpy(buffer, msg, sizeOfString);
            formattedDate = formatDate((StringBuffer&)date);
            currentMsg = new WCHAR[sizeOfString + 100];
            wsprintf(currentMsg, buffer, formattedDate.c_str());
            msg = currentMsg;
            delete [] currentMsg;
            delete [] buffer;
            break;
        case TYPE_NOW_EXIT_MANDATORY:
            buttonval.LoadString(IDS_BUT_NOW_EXIT);
            msg.LoadString(IDS_UP_MANDATORY_MESSAGE_EXIT);
            break;
        default:
            break;
    }

    int b1 = buttonval.Find(L"*");
    int b2 = buttonval.Find(L"*",b1+1);
    if (b1 == -1 && b2 == -1) {
        button1 = buttonval;
        button2 = L"";
    } else if (b2 == -1){ //just 2 buttons
        button1 = buttonval.Left(b1);
        button2 = buttonval.Right(buttonval.GetLength() - b1 -1);
        button3 = L"";
    } else { //3 buttons
        button1 = buttonval.Left(b1);
        swap = buttonval.Right(buttonval.GetLength() - b1 -1);
        int s = swap.Find(L"*");
        button3 = swap.Right(swap.GetLength() - s -1);
        button2 = swap.Left(s);
    }
    return CMessageBox(msg, button1, button2, button3);
}


/**
 * wParam = -2  begin            -> lParam = total size
 * wparam = -1  partial (resume) -> lParam = already exchanged size
 * wparam =  0  in progress      -> lParam = partial exchanged size
 * wParam =  1  end
 */
afx_msg LRESULT CMainSyncFrame::OnMsgSapiProgress(WPARAM wParam, LPARAM lParam) {

    //StringBuffer msg;
    //msg.sprintf("[%s] wParam = %d, lParam = %d", __FUNCTION__, wParam, lParam);
    //printLog(msg, LOG_DEBUG);

    if (wParam == -2) {
        itemTotalSize = lParam;
        partialCompleted = 0;
        progressStarted = true;
        return 1;
    }

    if (progressStarted == false) {
        // progress events are accepted only after a begin event
        return 1;
    }

    if (wParam == -1) {             // partially exchanged (download-upload)
        partialCompleted += lParam;
        return 1;
    }

    if (wParam == 1) {
        progressStarted = false;
        return 1;
    }

    // if here, wParam = 0 and progressStarted = true

    partialCompleted += lParam;
    int percentage = (int)((double)partialCompleted / (double)itemTotalSize * (double)100);
    if (percentage > 100) {
        percentage = 100;
    }

    CString p;
    p.Format(L"(%i%%)", percentage);
    
    // append to source status
    CString s = syncForm->getSourceStatus(currentSource);

    // remove the percentage in this line
    int start = s.Find(L"(");
    if (start > -1) {
        int end = s.Find(L")");
        CString par = s.Mid(start, end-start+1);
        s.Replace(par, L"");
        s.Trim();
    }

    CString s1; s1.FormatMessage(IDS_CONCAT_1_2, s, p); 
    syncForm->refreshSourceStatus(s1, currentSource);
    return 0;

}

afx_msg LRESULT CMainSyncFrame::OnCheckMediaHubFolder(WPARAM wParam, LPARAM lParam) {

    OutlookConfig* config = ((OutlookConfig*)getConfig());

    int ret = IDOK;
    if (!isMediaHubFolderSet()) {
        CMediaHubSetting mediaHubSetting;
        ret = mediaHubSetting.DoModal();
        if (ret == IDOK) {
            config->saveSyncSourceConfig(PICTURE_);
            config->saveSyncSourceConfig(VIDEO_);
            config->saveSyncSourceConfig(FILES_);
        }   else {
            unsigned int failFlags= MB_OK | MB_ICONASTERISK | MB_SETFOREGROUND | MB_APPLMODAL;
            CString s1;
            s1.FormatMessage(IDS_MEDIA_HUB_ALERT_FOLDER_NOT_SET, _T(MEDIA_HUB_DEFAULT_LABEL));

            //s1.LoadString(IDS_MEDIA_HUB_ALERT_FOLDER_NOT_SET);
            //MessageBox(s1, WPROGRAM_NAME, failFlags);
        }
    }
    if (config) {
        StringBuffer fpath = config->getSyncSourceConfig(PICTURE_)->getProperty(PROPERTY_MEDIAHUB_PATH);
        const char* installPath = config->getWorkingDir();
        createMediaHubDesktopIniFile(fpath.c_str(), installPath);
    }
    return ret;
}

BOOL CMainSyncFrame::createMediaHubDesktopIniFile(const char* folderPath, const char* installPath) {

    if (isWindowsXP()) {
        return TRUE;
    }

    WCHAR* tmp = toWideChar(folderPath);
    BOOL ret = PathMakeSystemFolder(tmp);
    if (ret != 0) {
        // create the file
        StringBuffer file(folderPath);
        file.append("\\");
        file.append("Desktop.ini");
        WCHAR* wfile = toWideChar(file.c_str());

        // create the IconFile path
        StringBuffer icoName(installPath);
        icoName.append("\\images\\");
        icoName.append(MEDIA_HUB_DEFAULT_ICO);

        // populate the infoTip
        CString s1;
        s1.FormatMessage(IDS_MEDIA_HUB_DESKTOPINI_TIP, _T(MEDIA_HUB_DEFAULT_LABEL));
        // s1.LoadString(IDS_MEDIA_HUB_DESKTOPINI_TIP);
        StringBuffer tip = ConvertToChar(s1);

        FILE* f = fileOpen(file.c_str(), "w+");
        if (f) {
            StringBuffer s;
            s = "[.ShellClassInfo]\r\n";
            s.append("IconFile=");
            s.append(icoName);
            s.append("\r\n");
            s.append("IconIndex=0\r\n");
            s.append("InfoTip=");
            s.append(tip);

            fwrite(s.c_str(), 1, s.length(), f);
            fclose(f);
            SetFileAttributes(wfile, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);

        }
        delete [] wfile;
    }
    delete [] tmp;

    return ret;

}

LRESULT CMainSyncFrame::OnMsgSchedulerDisabled( WPARAM , LPARAM lParam) {
    CString s1;
    //s1.LoadString(IDS_TEXT_SCHEDULER_DISABLED);
    s1 = TEXT("");
    refreshStatusBar(s1);

	bSchedulerWasDisabledByLogin = true;
    Invalidate();

    return 0;
}

void CMainSyncFrame::OnTestPopups()
{
    //
    // sync ended popups
    //
    manageSyncErrorMsg(WIN_ERR_GENERIC);                     // 1
    //manageSyncErrorMsg(WIN_ERR_SYNC_CANCELED);               // 2
    //manageSyncErrorMsg(WIN_ERR_FATAL_OL_EXCEPTION);        // 3   deprecated
    manageSyncErrorMsg(WIN_ERR_THREAD_TERMINATED);           // 4
    //manageSyncErrorMsg(WIN_ERR_FULL_SYNC_CANCELED);        // 5   deprecated
    manageSyncErrorMsg(WIN_ERR_UNEXPECTED_EXCEPTION);        // 6
    //manageSyncErrorMsg(WIN_ERR_UNEXPECTED_STL_EXCEPTION);  // 7  (same msg as code 6)
    manageSyncErrorMsg(WIN_ERR_SERVER_QUOTA_EXCEEDED);       // 8
    manageSyncErrorMsg(WIN_ERR_LOCAL_STORAGE_FULL);          // 9
    //manageSyncErrorMsg(WIN_ERR_DROPPED_ITEMS);               // 10
    //manageSyncErrorMsg(WIN_ERR_DROPPED_ITEMS_SERVER);        // 11
    manageSyncErrorMsg(WIN_ERR_NO_SOURCES);                  // 12
    //manageSyncErrorMsg(WIN_ERR_SAPI_NOT_SUPPORTED);          // 13
    manageSyncErrorMsg(WIN_ERR_INVALID_CREDENTIALS);         // 401
    //manageSyncErrorMsg(WIN_ERR_PROXY_AUTH_REQUIRED);       // 407 (same msg as code 401)
    //manageSyncErrorMsg(WIN_ERR_REMOTE_NAME_NOT_FOUND);       // 404
    //manageSyncErrorMsg(WIN_ERR_AUTOSYNC_DISABLED);	         // 500
    manageSyncErrorMsg(WIN_ERR_WRONG_HOST_NAME);             // 2001
    //manageSyncErrorMsg(WIN_ERR_NETWORK_ERROR);             // 2050 (same msg as code 2050)

    CString s1;
    unsigned int flags = 0;

    //
    // restore popups
    //
    //flags = MB_YESNO | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
    //MessageBox(WMSG_BOX_REFRESH_FROM_SERVER, WPROGRAM_NAME, flags);
    //MessageBox(WMSG_BOX_REFRESH_FROM_CLIENT, WPROGRAM_NAME, flags);


    // Account screen popups
    s1.LoadString(IDS_ERROR_SET_URL);           wsafeMessageBox(s1);
    s1.LoadString(IDS_ERROR_SET_USERNAME);      wsafeMessageBox(s1);
    s1.LoadString(IDS_ERROR_SET_PASSWORD);      wsafeMessageBox(s1);



    //
    // misc popups
    //
    s1.LoadString(IDS_TEXT_SYNC_IN_PROGRESS);
    flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
    MessageBox(s1, WPROGRAM_NAME, flags);

    s1.LoadString(IDS_ERROR_CANNOT_CHANGE_SETTINGS);
    wsafeMessageBox(s1);

    s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
    wsafeMessageBox(s1);

    s1.LoadString(IDS_ERROR_SET_CONNECTION);
    wsafeMessageBox(s1);

    flags = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_APPLMODAL;
    s1.LoadString(IDS_CANCEL_SYNC_IN_PROGRESS);
    MessageBox(s1.GetString(), WPROGRAM_NAME, flags);
    // MessageBox(WMSG_BOX_CANCEL_SYNC, WPROGRAM_NAME, flags);

    //AfxMessageBox(AFX_IDP_FAILED_TO_CREATE_DOC);

    //s1.LoadString(IDS_ERROR_SET_REMOTE_NAME);
    //wsafeMessageBox(s1);

    s1.LoadString(IDS_ERROR_PROXY_USERNAME);
    wsafeMessageBox(s1);

    s1.LoadString(IDS_ERROR_PROXY_PASSWORD);
    wsafeMessageBox(s1);

    //s1.LoadString(IDS_SCHEDULER_CANNOT_SCHEDULE);
    //wsafeMessageBox(s1);

    //char msg[500];
    //sprintf(msg, ERR_OUTLOOK_BAD_FOLDER_TYPE, itemType.c_str());
    //safeMessageBox(msg);


    //s1.LoadString(IDS_WARNING_ONEWAY_REMOVAL);
    //flags = flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
    //MessageBox(NULL, s1, WPROGRAM_NAME, flags);


    // send log popups
    //CString str;
    //str.LoadString(IDS_ERROR_SET_CONNECTION);
    //MessageBox((LPCTSTR)str, TEXT(PROGRAM_NAME), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND);

    // SAPI payment required (TODO: use resources)
    //flags = MB_YESNO | MB_ICONQUESTION | MB_SETFOREGROUND | MB_APPLMODAL;
	//wsafeMessageBox(s1.GetBuffer(), 0, msgboxFlags);

    //
    // SW upgrade
    //
    OnMsgPopup(TYPE_SKIPPED_ACTION, NULL);
    OnMsgPopup(TYPE_NOW_LATER_SKIP_OPTIONAL, NULL);
    OnMsgPopup(TYPE_NOW_LATER_RECCOMENDED, NULL);
    OnMsgPopup(TYPE_NOW_EXIT_MANDATORY, NULL);

    UpdaterConfig& config = getConfig()->getUpdaterConfig();
    config.setReleaseDate("20110812");
    OnMsgPopup(TYPE_NOW_LATER_MANDATORY, NULL);


    // all status bar messages: will be triggered clicking on a source tab
    safeMessageBox("*** NOT A MESSAGE: Click on a row to continue ***");
    testingStatusText = 1;
}


void CMainSyncFrame::testAllStatusText(const int sourceID)
{
    currentSource = sourceID;
    onTestStatusText();

    if (testingStatusText > 28) {
        // test finished: alert user and reset UI/config
        testingStatusText = 0;
        currentSource = 0;
        getConfig()->read();
        syncForm->refreshSources();
        safeMessageBox("*** NOT A MESSAGE: Test completed! ***");
    }
}

void CMainSyncFrame::onTestStatusText()
{

    CString s, ci, ti, p, s1;
    StringBuffer name = syncSourceIndexToName(currentSource);
    if (name.empty()) goto finally;

    SyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(name.c_str());
    if (!ssc) goto finally;


    switch (testingStatusText)
    {
    //
    // sync status messages
    //
    case 1:
        refreshStatusBar(IDS_TEXT_STARTING_SYNC);       // sync begin
        break;
    case 2:
        refreshStatusBar(IDS_CONNECTING);               // syncsource begin
        syncForm->refreshSourceStatus(IDS_CONNECTING, currentSource);
        break;

    case 3:
        refreshStatusBar(IDS_TEXT_SENDING);             // sending x/y (TODO: fix with numbers)
        
        ci.Format(L"%i", 3);
        ti.Format(L"%i", 10);
        s.FormatMessage(IDS_SENDING_XY, ci, ti);
        syncForm->refreshSourceStatus(s, currentSource);
        break;
    case 4:        
        p.Format(L"(%i%%)", 30);
        ci.Format(L"%i", 3);
        ti.Format(L"%i", 10);
        s.FormatMessage(IDS_SENDING_XY, ci, ti);    
        s1.FormatMessage(IDS_CONCAT_1_2, s, p); 
        syncForm->refreshSourceStatus(s1, currentSource);
        break;

    case 5:
        refreshStatusBar(IDS_TEXT_RECEIVING);           // receiving x/y (TODO: fix with numbers)        
        ci.Format(L"%i", 4);
        ti.Format(L"%i", 10);
        s.FormatMessage(IDS_RECEIVING_XY, ci, ti);
        syncForm->refreshSourceStatus(s, currentSource);
        break;
    case 6:
       
        p.Format(L"(%i%%)", 30);
        ci.Format(L"%i", 3);
        ti.Format(L"%i", 10);
        s.FormatMessage(IDS_RECEIVING_XY, ci, ti);    
        s1.FormatMessage(IDS_CONCAT_1_2, s, p); 
        syncForm->refreshSourceStatus(s1, currentSource);
        break;

    case 7:
        // currentSource needs to be set
        //OnMsgRefreshStatusBar(  0, SBAR_DELETE_CLIENT_ITEMS);
        //break;
        testingStatusText ++;
    case 8:
        //OnMsgRefreshStatusBar(120, SBAR_CHECK_ALL_ITEMS);
        //break;
        testingStatusText ++;
    case 9:
        OnMsgRefreshStatusBar(  0, SBAR_CHECK_MOD_ITEMS);
        break;
    case 10:
        OnMsgRefreshStatusBar( 10, SBAR_CHECK_MOD_ITEMS2);
        break;
    case 11:
        // OnMsgRefreshStatusBar(  0, SBAR_WRITE_OLD_ITEMS);
        // break;
        testingStatusText ++;
    case 12:
        OnMsgRefreshStatusBar(  0, SBAR_SENDDATA_BEGIN);
        break;
    case 13:
        OnMsgRefreshStatusBar(  0, SBAR_RECEIVE_DATA_BEGIN);
        break;
    case 14:
        OnMsgRefreshStatusBar(  0, SBAR_ENDING_SYNC);
        break;
    case 15:
        refreshStatusBar(IDS_LOGGING_IN);
        break;
    case 16:
        //refreshStatusBar(IDS_LOGIN_SUCCESSFUL);
        //break;
        testingStatusText ++;
    case 17:
        //refreshStatusBar(IDS_LOGIN_AUTH_FAILED);
        //break;
        testingStatusText ++;
    case 18:
        refreshStatusBar(IDS_TEXT_CANCELING_SYNC);
        syncForm->refreshSourceStatus(IDS_TEXT_CANCELING_SYNC, currentSource);
        break;
    case 19:
        refreshStatusBar(IDS_TEXT_SYNC_ENDED);
        break;
    case 20:
        refreshStatusBar(AFX_IDS_IDLEMESSAGE);
        break;

    //
    // last sync messages
    //
    case 21:
        ssc->setLastSourceError(WIN_ERR_GENERIC);
        syncForm->refreshSources();
        break;
    case 22:
        ssc->setLastSourceError(WIN_ERR_SYNC_CANCELED);
        syncForm->refreshSources();
        break;
    case 23:
        ssc->setLastSourceError(WIN_ERR_SERVER_QUOTA_EXCEEDED);
        syncForm->refreshSources();
        break;
    case 24:
        ssc->setLastSourceError(WIN_ERR_LOCAL_STORAGE_FULL);
        syncForm->refreshSources();
        break;
    case 25:
        ssc->setLastSourceError(WIN_ERR_SAPI_NOT_SUPPORTED);
        syncForm->refreshSources();
        break;
    case 26:
        // Last sync with tstamp
        ssc->setEndSyncTime((long)time(NULL));
        ssc->setLastSourceError(WIN_ERR_NONE);
        syncForm->refreshSources();
        break;
    case 27:
        // Not synchronized
        ssc->setEndSyncTime(0);
        ssc->setLastSourceError(WIN_ERR_NONE);
        syncForm->refreshSources();
        break;
    case 28:
        // empty
        syncForm->refreshSourceStatus(_T(""), currentSource);
        break;

    default:
        break;
    }

finally:
    testingStatusText ++;
}
