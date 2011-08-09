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
#include "CustomPane.h"
#include "winmaincpp.h"
#include "utils.h"
#include "SyncForm.h"
#include "ClientUtil.h"
#include "UICustomization.h"


IMPLEMENT_DYNAMIC(CCustomPane, CStatic)

void CCustomPane::DoDataExchange(CDataExchange* pDX) {
	CStatic::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CCustomPane, CStatic)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
    ON_WM_TIMER()
END_MESSAGE_MAP()


CCustomPane::CCustomPane(CSyncForm* caller, const int sourceId, const int ix, bool last) {

    syncForm = caller;
    id = sourceId;
    index = ix;
    lastPane = last;
    initialize();
}

CCustomPane::CCustomPane(const CCustomPane& objectSrc) {

    syncForm = objectSrc.getCallerWnd();
    id = objectSrc.getId();
    index = objectSrc.getIndex();
    lastPane = objectSrc.isLastPane();
    initialize();
}

CCustomPane::~CCustomPane() {

    DeleteObject(sourceIconEnabled);
    DeleteObject(sourceIconDisabled);
}


void CCustomPane::initialize() {

    initializeFonts();

    int labelStringID;
    int sourceIconEnabledID;
    int sourceIconDisabledID;

    switch (id) {
        case SYNCSOURCE_CONTACTS:
        {
            name                 = CONTACT_;
            labelStringID        = IDS_MAIN_CONTACTS;
            sourceIconEnabledID  = IDI_CONTACTS;
            sourceIconDisabledID = IDI_CONTACTS_GREY;
            break;
        }
        case SYNCSOURCE_CALENDAR:
        {
            name                 = APPOINTMENT_;
            labelStringID        = IDS_MAIN_CALENDAR;
            sourceIconEnabledID  = IDI_CALENDAR;
            sourceIconDisabledID = IDI_CALENDAR_GREY;
            break;
        }
        case SYNCSOURCE_TASKS:
        {
            name                 = TASK_;
            labelStringID        = IDS_MAIN_TASKS;
            sourceIconEnabledID  = IDI_TASKS;
            sourceIconDisabledID = IDI_TASKS_GREY;
            break;
        }
        case SYNCSOURCE_NOTES:
        {
            name                 = NOTE_;
            labelStringID        = IDS_MAIN_NOTES;
            sourceIconEnabledID  = IDI_NOTES;
            sourceIconDisabledID = IDI_NOTES_GREY;
            break;
        }
        case SYNCSOURCE_PICTURES:
        {
            name                 = PICTURE_;
            labelStringID        = IDS_MAIN_PICTURES;
            sourceIconEnabledID  = IDI_PICTURES;
            sourceIconDisabledID = IDI_PICTURES_GREY;
            break;
        }
        case SYNCSOURCE_VIDEOS:
        {
            name                 = VIDEO_;
            labelStringID        = IDS_MAIN_VIDEOS;
            sourceIconEnabledID  = IDI_VIDEOS;
            sourceIconDisabledID = IDI_VIDEOS_GREY;
            break;
        }
        case SYNCSOURCE_FILES:
        {
            name                 = FILES_;
            labelStringID        = IDS_MAIN_FILES;
            sourceIconEnabledID  = IDI_FILES;
            sourceIconDisabledID = IDI_FILES_GREY;
            break;
        }
        default:
        {
            StringBuffer msg;
            msg.sprintf("[%s] wrong pane ID: %d", __FUNCTION__, id);
            printLog(msg.c_str(), "ERROR");
            return;
        }
    }

    // config
    SyncSourceConfig* ssconfig = getSSConfig();
    if (!ssconfig) return;

    counterAnim = 0;            // sync animation
    showStatusIcon = true;      // set to false to hide the status icon on startup (it's set to true after 1st sync)
    clicked = false;            // true when mouse click-down
    mouseOver = false;
    
    //
    // Get Windows dpi and fix objects size
    //
    HDC hdc = ::GetDC(0);
    dpiX = ::GetDeviceCaps(hdc, LOGPIXELSX);
    dpiY = ::GetDeviceCaps(hdc, LOGPIXELSY);
    ::ReleaseDC(0, hdc);

    size.cx     = PANE_SIZE_X;     // original size of bitmap
    size.cy     = PANE_SIZE_Y;
    iconSize.cx = ICON_SIZE_X;     // original size of icon
    iconSize.cy = ICON_SIZE_Y;
    if (dpiX != 96) {
        size.cx     = (int) (size.cx     * (dpiX/96.0));
        size.cy     = (int) (size.cy     * (dpiY/96.0));
        iconSize.cx = (int) (iconSize.cx * (dpiX/96.0));
        iconSize.cy = (int) (iconSize.cy * (dpiY/96.0));
    }


    // Create the pane
    int yStartPos = Y_SPACE_TOP;
    if (USE_SYNCALL_PANE) {
        yStartPos += size.cy + Y_SPACE_BELOW_SYNCALL;
    }
    int offset = yStartPos + index * (size.cy + Y_SPACE_BETWEEN_PANES);
    CPoint pos(X_SPACE_LEFT, offset + Y_SPACE_BETWEEN_PANES);
    Create(_T("pane"), SS_BITMAP|SS_NOTIFY|WS_VISIBLE, CRect(pos, size), syncForm);


    // Create source label
    CPoint labelPos(pos.x + (int)(size.cx*0.20), 
                    pos.y + (int)(size.cy*0.20));
    CSize labelSize((int)(size.cx*0.90), 
                    (int)(size.cy*0.70));
    labelText.LoadString(labelStringID); 
    sourceLabel.Create(labelText, WS_CHILD|SS_NOTIFY|WS_VISIBLE, CRect(labelPos, labelSize), syncForm);
    sourceLabel.SetFont(&fontBold);

    // Create status label
    CPoint statusPos(pos.x + (int)(size.cx*0.20), 
                     pos.y + (int)(size.cy*0.60));
    CSize statusSize((int)(size.cx*0.90), 
                     (int)(size.cy*0.70));
    statusLabel.Create(_T(""), WS_CHILD|SS_NOTIFY|WS_VISIBLE, CRect(statusPos, statusSize), syncForm);
    statusLabel.SetFont(&fontNormal);


    // Create icons
    sourceIconEnabled  = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(sourceIconEnabledID));
    sourceIconDisabled = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(sourceIconDisabledID));
    
    CPoint sourceIconPos(pos.x + (int)(size.cx*0.05), 
                         pos.y + (size.cy - iconSize.cy)/2);
    sourceIcon.Create(_T("source_icon"), SS_ICON|WS_CHILD|WS_VISIBLE|SS_NOTIFY|WS_EX_TRANSPARENT, 
                      CRect(sourceIconPos, iconSize), syncForm);

    CPoint statusIconPos(pos.x + (int)(size.cx*0.85), 
                         pos.y + (size.cy - iconSize.cy)/2);
    statusIcon.Create(_T("status_icon"), SS_ICON|WS_CHILD|WS_VISIBLE|SS_NOTIFY, 
                      CRect(statusIconPos, iconSize), syncForm);


    // refresh icons and status text (read from config)
    refresh();
}

SyncSourceConfig* CCustomPane::getSSConfig() {

    SyncSourceConfig* ssconfig = getConfig()->getSyncSourceConfig(name);
    if (!ssconfig) {
        StringBuffer msg;
        msg.sprintf("[%s] NULL config for source %s", __FUNCTION__, name.c_str());
        printLog(msg.c_str(), LOG_ERROR);
    }
    return ssconfig;
}


void CCustomPane::refresh() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: source %s, state %d, mouseOver=%d, locked=%d", 
            __FUNCTION__, name.c_str(), state, mouseOver, syncForm->isUILocked());
        printLog(msg.c_str(), LOG_DEBUG);
    }

    if (state == PANE_STATE_SYNC) {
        // Don't refresh the pane items if under sync or UI locked (forced)
        return;
    }

    SyncSourceConfig* ssconfig = getSSConfig();
    if (!ssconfig) return;

    // source state: OK, error,... (must be set first!)
    int error = ssconfig->getLastSourceError(); 
    sourceState = manageWinErrors(error);

    // status text: updated from config
    statusText = getLastSyncStatusText();
    statusLabel.SetWindowText(statusText);

    // allowed/enabled -> show/dim objects
    if (ssconfig->isAllowed()) {
        if (ssconfig->isEnabled()) {
            state = PANE_STATE_NORMAL;
            sourceIcon.SetIcon(sourceIconEnabled);
            sourceLabel.EnableWindow(TRUE);
            statusLabel.EnableWindow(TRUE);
            refreshStatusIcon();    // only if source enabled & allowed
        } 
        else {
            state = PANE_STATE_DISABLED;
            sourceIcon.SetIcon(sourceIconDisabled);
            statusIcon.SetIcon(NULL);
            sourceLabel.EnableWindow(FALSE);
            statusLabel.EnableWindow(FALSE);
        }
    }
    else {
        // source not allowed
        state = PANE_STATE_NOT_ALLOWED;
        sourceIcon.SetIcon(sourceIconDisabled);
        statusIcon.SetIcon(NULL);
        sourceLabel.EnableWindow(FALSE);
        statusLabel.EnableWindow(FALSE);
    }
}


void CCustomPane::OnPaint() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: source %s, state %d, mouseOver=%d, locked=%d", 
            __FUNCTION__, name.c_str(), state, mouseOver, syncForm->isUILocked());
        printLog(msg.c_str(), LOG_DEBUG);
    }

    // Set the bkground bitmap
    HBITMAP bkground = syncForm->hBmpLight;
    if (state == PANE_STATE_SYNC || mouseOver) {
        bkground = syncForm->hBmpBlue;
    }
    else if (state == PANE_STATE_DISABLED ||
             state == PANE_STATE_NOT_ALLOWED) {
        bkground = NULL; //syncForm->hBmpDisabled;
    }


    // refresh background and stretch it
    CPaintDC dc(this);
    CDC tempdc;
    tempdc.CreateCompatibleDC(&dc);
    tempdc.SelectObject(bkground);

    if(dpiX == 96.0){
        dc.BitBlt(0, 0, size.cx, size.cy, &tempdc, 0, 0, SRCCOPY);
    } 
    else{
        dc.StretchBlt(0, 0, size.cx, size.cy, &tempdc, 0, 0, PANE_SIZE_X, PANE_SIZE_Y, SRCCOPY);  
    }

    // Extend the focus on the whole pane
    // (if overlapping panes, reduce focus to avoid double repaints)
    int dy = size.cy;
    if (Y_SPACE_BETWEEN_PANES < 0 && !lastPane) {
        dy += Y_SPACE_BETWEEN_PANES;
    }
    SetWindowPos(NULL, 0, 0, size.cx, dy, SWP_SHOWWINDOW | SWP_NOMOVE);


    //
    // refresh all objects
    //
    statusIcon.Invalidate();
    sourceIcon.Invalidate();

    sourceLabel.Invalidate();
    statusLabel.Invalidate();

    return;
}

void CCustomPane::setStatusText(const CString& msg) { 

    statusText = msg; 
    statusLabel.SetWindowText(statusText);
}


// CCustomPane message handlers
void CCustomPane::OnMouseMove(UINT nFlags, CPoint point) {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: source %s, state %d, mouseOver=%d, locked=%d", 
            __FUNCTION__, name.c_str(), state, mouseOver, syncForm->isUILocked());
        printLog(msg.c_str(), LOG_DEBUG);
    }

    if (state == PANE_STATE_SYNC || 
        state == PANE_STATE_DISABLED ||
        state == PANE_STATE_NOT_ALLOWED) {
        // no reaction on mouse passing
        return;
    }
    if (syncForm->isUILocked()) {
        // no reaction on mouse passing
        return;
    }

    if (!mouseOver) {
        mouseOver = true;

        // must do this to be notified by OnMouseLeave()
        TRACKMOUSEEVENT Tme;
        Tme.cbSize = sizeof(TRACKMOUSEEVENT);
        Tme.dwFlags = TME_LEAVE;
        Tme.hwndTrack = m_hWnd;
        TrackMouseEvent(&Tme);

        SetBitmap(syncForm->hBmpBlue);
        statusIcon.SetIcon(syncForm->iconMouseOver);
    }

    //CStatic::OnMouseMove(nFlags, point);
}


void CCustomPane::OnMouseLeave() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: source %s, state %d, mouseOver=%d, locked=%d", 
            __FUNCTION__, name.c_str(), state, mouseOver, syncForm->isUILocked());
        printLog(msg.c_str(), LOG_DEBUG);
    }

    clicked = false;
    if( state == PANE_STATE_SYNC || 
        state == PANE_STATE_DISABLED ||
        state == PANE_STATE_NOT_ALLOWED) {
        // no reaction on mouse passing
        return;
    }

    if (syncForm->isUILocked()) {
        // no reaction on mouse passing
        return;
    }

    mouseOver = false;
    state = PANE_STATE_NORMAL;
    SetBitmap(syncForm->hBmpLight);

    refreshStatusIcon();
}

void CCustomPane::refreshStatusIcon() {

    if (!showStatusIcon) {
        // don't want to see the status icon
        statusIcon.SetIcon(NULL);
        return;
    }

    if (sourceState == SYNCSOURCE_STATE_OK) {
        SyncSourceConfig* ssconfig = getSSConfig();
        if (!ssconfig) return;

        if (ssconfig->getEndSyncTime() == 0) {
            // not synchronized: no status icon
            statusIcon.SetIcon(NULL);
        } 
        else {
            // last sync was OK
            statusIcon.SetIcon(syncForm->iconOk);
        }
    } 
    else {
        // last sync was failed
        statusIcon.SetIcon(syncForm->iconAlert);
    }
}

void CCustomPane::setStatusIcon(HICON hIcon) {
    if (showStatusIcon) {
        statusIcon.SetIcon(hIcon);
    }
}


afx_msg void CCustomPane::OnLButtonDown(UINT nFlags, CPoint point) {
    clicked = true;
}

afx_msg void CCustomPane::OnLButtonUp(UINT nFlags, CPoint point) {

    if (!clicked) { return; }
    clicked = false;

    if (state == PANE_STATE_NORMAL) {
        //
        // *** START SYNC! ***
        // (state will be set to SYNC only when sync really starts)
        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync(id);
    }

    // else: nothing to do
    // TODO: we may show popups here for 
    //       PANE_STATE_DISABLED and PANE_STATE_NOT_ALLOWED
}



void CCustomPane::onSyncStarted() {

    state = PANE_STATE_SYNC;
    counterAnim = 0;
    SetTimer(ANIM_ICON_ARROWS, ANIM_ICON_DELAY, NULL);

    refresh();
}

void CCustomPane::onSyncEnded() {

    KillTimer(ANIM_ICON_ARROWS);
    state = PANE_STATE_NORMAL;

    // to make sure we refresh the bkground bmp
    mouseOver = false;

    refresh();
}


void CCustomPane::initializeFonts() {

    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));  
    lf.lfWeight = FW_BOLD; 
    lf.lfHeight =-15;
    wcscpy(lf.lfFaceName, _T("Tahoma"));
    VERIFY(fontBold.CreateFontIndirect(&lf));

    memset(&lf, 0, sizeof(LOGFONT));  
    lf.lfHeight = -10;
    wcscpy(lf.lfFaceName, _T("Tahoma"));
    VERIFY(fontNormal.CreateFontIndirect(&lf)); 
}

CString CCustomPane::getLastSyncStatusText() {

    CString s1;

    switch (sourceState) {
        case SYNCSOURCE_STATE_OK: 
        {
            SyncSourceConfig* ssconfig = getSSConfig();
            if (!ssconfig) return _T("config error");

            long tstamp = ssconfig->getEndSyncTime();
            if (tstamp == 0) {
                s1.LoadString(IDS_NOT_SYNCHRONIZED); 
            } else {
                CTime endSyncTime(tstamp);
                s1.LoadString(IDS_SYNCHRONIZED); 
                s1+= " ";
                s1 += endSyncTime.Format(LAST_SYNC_TIME_FORMAT);
            }
            break;
        }
        case SYNCSOURCE_STATE_FAILED:
            s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
            break;
        case SYNCSOURCE_STATE_CANCELED:
            s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
            break;
        case SYNCSOURCE_STATE_QUOTA_EXCEEDED:
            s1.LoadString(IDS_STATUS_QUOTA_EXCEEDED);
            break;
        case SYNCSOURCE_STATE_STORAGE_FULL:
            s1.LoadString(IDS_STATUS_STORAGE_FULL);
            break;
        case SYNCSOURCE_STATE_NOT_SUPPORTED:
            s1.LoadString(IDS_SOURCE_NOT_SUPPORTED_BY_SERVER);
            break;                
        default:
            s1 = "";
            break;
    }
    return s1;
}

void CCustomPane::OnTimer(UINT_PTR nIDEvent ) {

    if(counterAnim == 4) {
        counterAnim = 0;
    }
    switch(counterAnim){
        case 0:
            statusIcon.SetIcon(syncForm->iconSpin1);
            break;
        case 1:
            statusIcon.SetIcon(syncForm->iconSpin2);
            break;
        case 2:
            statusIcon.SetIcon(syncForm->iconSpin3); 
            break;
        case 3:
            statusIcon.SetIcon(syncForm->iconSpin4);
            break;
    }
    counterAnim++;

    // correctly refreshes only the required rect
    Invalidate();
}
