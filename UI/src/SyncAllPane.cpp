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
#include "OutlookPlugin.h"
#include "SyncAllPane.h"
#include "winmaincpp.h"
#include "utils.h"
#include "SyncForm.h"
#include "ClientUtil.h"
#include "UICustomization.h"


IMPLEMENT_DYNAMIC(CSyncAllPane, CStatic)

void CSyncAllPane::DoDataExchange(CDataExchange* pDX) {
	CStatic::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSyncAllPane, CStatic)
    ON_WM_MOUSEMOVE()
    ON_WM_MOUSELEAVE()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_PAINT()
END_MESSAGE_MAP()


CSyncAllPane::CSyncAllPane(CSyncForm* caller) {

    syncForm = caller;
    initialize();
}

CSyncAllPane::CSyncAllPane(const CSyncAllPane& objectSrc) {

    syncForm = objectSrc.getCallerWnd();
    initialize();
}

CSyncAllPane::~CSyncAllPane() {

    DeleteObject(iconLogo);
    DeleteObject(iconSyncAll);
    DeleteObject(iconCancel);
}


void CSyncAllPane::initialize() {

    initializeFonts();

    int labelStringID = IDS_SYNCALL;
    int iconLogoID    = IDI_LOGO;
    int iconSyncAllID = IDI_SYNC_ALL_BLUE;
    int iconCancelID  = IDI_CANCEL;

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
    CPoint pos(X_SPACE_LEFT, Y_SPACE_TOP);
    Create(_T("pane"), SS_BITMAP|SS_NOTIFY|WS_VISIBLE, CRect(pos, size), syncForm);


    // Create label
    CPoint labelPos(pos.x + (int)(size.cx*0.20), 
                    pos.y + (int)(size.cy*0.35));
    CSize labelSize((int)(size.cx*0.90), 
                    (int)(size.cy*0.70));
    labelText.LoadString(labelStringID); 
    label.Create(labelText, WS_CHILD|SS_NOTIFY|WS_VISIBLE, CRect(labelPos, labelSize), syncForm);
    label.SetFont(&fontBold);

    // set this unique ID, so the text can be colored from CSyncForm::OnCtlColor()
    label.SetDlgCtrlID(IDC_MAIN_SYNCALL_LABEL);


    // Create icons
    iconLogo    = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(iconLogoID));
    iconSyncAll = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(iconSyncAllID));
    iconCancel  = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(iconCancelID));
    
    CPoint leftIconPos(pos.x + (int)(size.cx*0.05), 
                         pos.y + (size.cy - iconSize.cy)/2);
    leftIcon.Create(_T("left_icon"), SS_ICON|WS_CHILD|WS_VISIBLE|SS_NOTIFY|WS_EX_TRANSPARENT, 
                      CRect(leftIconPos, iconSize), syncForm);

    CPoint statusIconPos(pos.x + (int)(size.cx*0.85), 
                         pos.y + (size.cy - iconSize.cy)/2);
    statusIcon.Create(_T("status_icon"), SS_ICON|WS_CHILD|WS_VISIBLE|SS_NOTIFY, 
                      CRect(statusIconPos, iconSize), syncForm);
    
    // init to normal state
    state = SYNCALL_PANE_STATE_NORMAL;
    leftIcon.SetIcon(iconLogo);
    refresh();
}

void CSyncAllPane::refresh() {

    if (UICustomization::verboseUIDebugging) {
        StringBuffer msg;
        msg.sprintf("%s: state %d ", __FUNCTION__, state);
        printLog(msg.c_str(), LOG_DEBUG);
    }

    if (state == SYNCALL_PANE_STATE_NORMAL) {
        labelText.LoadString(IDS_SYNCALL);
        if (mouseOver) {
            statusIcon.SetIcon(iconSyncAll);
        } else {
            statusIcon.SetIcon(NULL);
        }
    }
    else {  // SYNCALL_PANE_STATE_SYNC
        labelText.LoadString(IDS_CANCEL_SYNC);
        statusIcon.SetIcon(iconCancel);
    }
    label.SetWindowText(labelText);

}


void CSyncAllPane::OnPaint() {

    // Set the bkground bitmap
    HBITMAP bkground = syncForm->hBmpDark;
    if (mouseOver) {
        bkground = syncForm->hBmpDarkBlue;
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
    SetWindowPos(NULL, 0, 0, size.cx, size.cy, SWP_SHOWWINDOW | SWP_NOMOVE);


    //
    // refresh all objects
    //
    statusIcon.Invalidate();
    leftIcon.Invalidate();
    label.Invalidate();

    return;
}


// CSyncAllPane message handlers
void CSyncAllPane::OnMouseMove(UINT nFlags, CPoint point) {

    if (!mouseOver) {
        mouseOver = true;

        // must do this to be notified by OnMouseLeave()
        TRACKMOUSEEVENT Tme;
        Tme.cbSize = sizeof(TRACKMOUSEEVENT);
        Tme.dwFlags = TME_LEAVE;
        Tme.hwndTrack = m_hWnd;
        TrackMouseEvent(&Tme);

        SetBitmap(syncForm->hBmpDarkBlue);
        refresh();
    }

    //CStatic::OnMouseMove(nFlags, point);
}


void CSyncAllPane::OnMouseLeave() {

    clicked = false;
    mouseOver = false;

    SetBitmap(syncForm->hBmpDark);
    refresh();
}


afx_msg void CSyncAllPane::OnLButtonDown(UINT nFlags, CPoint point) {
    clicked = true;
}

afx_msg void CSyncAllPane::OnLButtonUp(UINT nFlags, CPoint point) {

    if (!clicked) { return; }
    clicked = false;

    if (state == SYNCALL_PANE_STATE_NORMAL) {
        //
        // *** START SYNC ALL! ***
        //
        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
    else {
        //
        // *** CANCEL SYNC! ***
        //
        ((CMainSyncFrame*)AfxGetMainWnd())->CancelSync();
    }

    refresh();
    Invalidate();
}



void CSyncAllPane::onSyncStarted() {

    state = SYNCALL_PANE_STATE_SYNC;
    refresh();
}

void CSyncAllPane::onSyncEnded() {

    state = SYNCALL_PANE_STATE_NORMAL;
    refresh();
}


void CSyncAllPane::initializeFonts() {

    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));  
    lf.lfWeight = FW_BOLD; 
    lf.lfHeight =-17;
    wcscpy(lf.lfFaceName, _T("Tahoma"));
    VERIFY(fontBold.CreateFontIndirect(&lf));
}
