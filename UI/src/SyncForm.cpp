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


#pragma once
#include "stdafx.h"
#include "OutlookPlugin.h"
#include "SyncForm.h"
#include "MainSyncFrm.h"
#include "winmaincpp.h"
#include "ClientUtil.h"
#include "UICustomization.h"
#include "utils.h"
#include "AnimatedIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CSyncForm, CFormView)



void startCheckMediaFolderThread();

CSyncForm::CSyncForm() : CFormView(CSyncForm::IDD) {
    lockedUI = false;
    syncAllPane = NULL;
}

CSyncForm::~CSyncForm() {
    
    DeleteObject(hBmpDarkBlue);
    DeleteObject(hBmpBlue);
    DeleteObject(hBmpDark);
    DeleteObject(hBmpLight);
    DeleteObject(iconAlert);
    DeleteObject(iconOk);
    DeleteObject(iconMouseOver);

    delete syncAllPane;
}

void CSyncForm::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CSyncForm, CFormView)

    ON_MESSAGE( WM_INITDIALOG, OnInitForm ) 
    ON_WM_NCPAINT( )
    ON_WM_CTLCOLOR()
    ON_WM_ERASEBKGND()

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSyncForm diagnostics
#ifdef _DEBUG
void CSyncForm::AssertValid() const
{
	CFormView::AssertValid();
}
void CSyncForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////


LRESULT CSyncForm::OnInitForm(WPARAM, LPARAM) {
    
    CFormView::OnInitialUpdate();
    
    VERIFY(brushHollow.CreateStockObject(HOLLOW_BRUSH));
    

    // load common bitmaps
    hBmpDarkBlue = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_DARK_BLUE));
    hBmpBlue     = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_BLUE));
    hBmpDark     = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_DARK));
    hBmpLight    = LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_LIGHT));

    // load common icons
    iconMouseOver = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC_ALL));
    iconOk        = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_OK));
    iconAlert     = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));

    iconSpin1     = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32B));
    iconSpin2     = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32C));
    iconSpin3     = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32D));
    iconSpin4     = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ARROWS32A));


    // Create the SYNC ALL pane
    if (USE_SYNCALL_PANE) {
        syncAllPane = new CSyncAllPane(this);
    }

    //
    // Create the array of source panes
    //
    const ArrayList& sources = getConfig()->getSourcesVisible();
    for (int i=0; i<sources.size(); i++) {
        StringBuffer* sourceName = (StringBuffer*)sources.get(i);
        if (!sourceName) continue;

        int sourceId = syncSourceNameToIndex(*sourceName);
        if (sourceId == 0) continue;

        bool last = false;
        if (i == sources.size()-1) {
            last = true;
        }

        CCustomPane pane(this, sourceId, i, last);
        sourcePanes.push_back(pane);
    }


    if (checkSyncInProgress()) {
        // It may happen (scheduled sync): this way the UI is locked
        onSyncAllStarted();
    } 
    else {
        refreshSources();
    }

    if (isSourceVisible(PICTURE) || isSourceVisible(VIDEO) || isSourceVisible(FILES)) {
        startCheckMediaFolderThread();        
    }
    return 0;
}

static DWORD WINAPI checkMediaFolderThread(LPVOID lpv) {
    
    for (int i = 0; i < 20; i++) {
        Sleep(300);    
        HWND dd = HwndFunctions::getWindowHandle();
        if (dd == NULL) {
            continue;
        }
        ::SendMessage(dd, ID_MYMSG_CHECK_MEDIA_HUB_FOLDER, 0, 0);    
        break;
    }
    return 0;
}


void startCheckMediaFolderThread() {      
    static bool internalCheckMediaHubFolder = false;
    if (internalCheckMediaHubFolder == false) {
        if ( !CreateThread(NULL, 0, checkMediaFolderThread, (LPVOID)NULL, 0, NULL) ) {
            LOG.error("startCheckMediaFolderThread Error creating thread.");           
        }
    }
    internalCheckMediaHubFolder = true;
}


CCustomPane* CSyncForm::getSourcePane(const int sourceID) {

    list<CCustomPane>::iterator it = sourcePanes.begin();
    while (it != sourcePanes.end()) {
        if (sourceID == (*it).getId()) {
            return &(*it);
        } 
        it ++;
    }
    // not found
    return NULL;
}


void CSyncForm::refreshSources() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s", __FUNCTION__);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    // no more necessary: disabled panes are windows displayed normally now
    //fixOverlappingPanes();

    if (syncAllPane) {
        syncAllPane->refresh();
    }

    list<CCustomPane>::iterator it = sourcePanes.begin();
    while (it != sourcePanes.end()) {
        (*it).refresh();
        it ++;
    }

    Invalidate();
    return;
}


void CSyncForm::refreshSourceStatus(const CString& msg, const int sourceID) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer logmsg;
        logmsg.sprintf("%s: sourceID = %d", __FUNCTION__, sourceID);
        printLog(logmsg.c_str(), LOG_DEBUG);
    }

    if (!lockedUI && checkSyncInProgress() /* && getConfig()->getScheduledSync() */) {
        // trick - just for scheduled syncs, or in case something went
        // wrong: UI should always be locked during sync.
        onSyncAllStarted();
    }

    CCustomPane* sourcePane = getSourcePane(sourceID);
    if (sourcePane) {
        sourcePane->setStatusText(msg);
        sourcePane->Invalidate();
    }
}

void CSyncForm::refreshSourceStatus(const int resourceID, const int sourceID) {

    if (!resourceID) return;

    CString msg;
    msg.LoadString(resourceID);
    refreshSourceStatus(msg, sourceID);
}

CString CSyncForm::getSourceStatus(const int sourceID) {

    CCustomPane* sourcePane = getSourcePane(sourceID);
    if (sourcePane) {
        return sourcePane->getStatusText();
    }
    return _T("");
}


void CSyncForm::onSyncAllStarted() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s", __FUNCTION__);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    // lock UI at beginning, so we can remove the status 
    // icon for all sources
    lockedUI = true;

    if (syncAllPane) {
        syncAllPane->onSyncStarted();
    }

    // lock all sources with no status icon
    list<CCustomPane>::iterator it = sourcePanes.begin();
    while (it != sourcePanes.end()) {
        (*it).setStatusIcon(NULL);
        it ++;
    }
}

void CSyncForm::onSyncStarted(const int sourceID) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: sourceID = %d", __FUNCTION__, sourceID);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    if (syncAllPane) {
        syncAllPane->onSyncStarted();
    }

    list<CCustomPane>::iterator it = sourcePanes.begin();
    while (it != sourcePanes.end()) {
        SyncSourceConfig* ssc = (*it).getSSConfig();
        if (ssc) {
            if (sourceID == (*it).getId()) {
                // The source under sync should be enabled
                ssc->setIsEnabled(true);
            } 
            else {
                // Other sources are temporarly disabled
                ssc->setIsEnabled(false);
            }
        }
        it ++;
    }

    refreshSources();

    // lock UI at the end, when all other sources are disabled
    lockedUI = true;
}



void CSyncForm::onSyncEnded() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s", __FUNCTION__);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    lockedUI = false;

    if (syncAllPane) {
        syncAllPane->onSyncEnded();
    }

    list<CCustomPane>::iterator it = sourcePanes.begin();
    while (it != sourcePanes.end()) {
        (*it).onSyncEnded();
        it ++;
    }

    refreshSources();
}



void CSyncForm::onSyncSourceBegin(const int sourceID) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: sourceID = %d", __FUNCTION__, sourceID);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    //lockedUI = true;
    if (syncAllPane) {
        syncAllPane->onSyncStarted();
    }

    CCustomPane* sourcePane = getSourcePane(sourceID);
    if (sourcePane) {
        sourcePane->onSyncStarted();
    }
}

void CSyncForm::onSyncSourceEnd(const int sourceID) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: sourceID = %d", __FUNCTION__, sourceID);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    CCustomPane* sourcePane = getSourcePane(sourceID);
    if (sourcePane) {
        sourcePane->onSyncEnded();
        sourcePane->Invalidate();
    }
}


void CSyncForm::OnNcPaint() {
    CFormView::OnNcPaint();
    CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));
}


HBRUSH CSyncForm::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

    //set text color to 'Sync All' text
    if (syncAllPane && pWnd) {
        if (pWnd->GetDlgCtrlID() == IDC_MAIN_SYNCALL_LABEL) {
            int r = UICustomization::syncAllTextRed;
            int g = UICustomization::syncAllTextGreen;
            int b = UICustomization::syncAllTextBlue;
            pDC->SetTextColor(RGB(r,g,b));
        }
    }

    switch(nCtlColor) {
        case CTLCOLOR_STATIC:
        case CTLCOLOR_BTN:
            // let static controls shine through
            pDC->SetBkMode(TRANSPARENT);
            return HBRUSH(brushHollow);

        default:
            break;
    }
    return hbr;
}


BOOL CSyncForm::OnEraseBkgnd(CDC* pDC) {
    return TRUE;
}

void CSyncForm::OnDraw(CDC* pDC) {

    CRect rect;
    GetClientRect(&rect);
    CDC dc;
    dc.CreateCompatibleDC(pDC);

    CRect rect1;
    GetWindowRect(&rect1);
    ScreenToClient(&rect1);

    pDC->FillSolidRect(rect, COLOR_EXT_PANE);

    // no need for color, already has backgrounds
    dc.DeleteDC();
}

void CSyncForm::lockButtons() {
    lockedUI = true;
}

void CSyncForm::unlockButtons() {
    lockedUI = false;
}

bool CSyncForm::isUILocked() {
    return lockedUI;
}


// NOTE: not used (disabled panes are windows displayed normally now)
//void CSyncForm::fixOverlappingPanes() {
//
//    list<CCustomPane>::iterator it = sourcePanes.begin();
//    while (1) {
//        CCustomPane* previous = &(*it);
//        it ++;
//        if (it == sourcePanes.end()) {
//            previous->setLastPane(true);    // last pane
//            break;
//        }
//        // set as "last pane" also if next pane is disabled (dimmed)
//        SyncSourceConfig* ssconfig = (*it).getSSConfig();
//        if (ssconfig && ssconfig->isEnabled() == false) {
//            previous->setLastPane(true);
//        } else {
//            previous->setLastPane(false);
//        }
//    }
//}

