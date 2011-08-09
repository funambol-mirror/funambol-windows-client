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

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */

#include "afxwin.h"
#include "afxcmn.h"
#include "AnimatedIcon.h"

#if !defined(AFX_FORM1_H__FA98B71B_D0B7_11D3_BC39_00C04F602FEE__INCLUDED_)
#define AFX_FORM1_H__FA98B71B_D0B7_11D3_BC39_00C04F602FEE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

#include "CustomPane.h"
#include "SyncAllPane.h"
#include <list>


#define COLOR_EXT_PANE          RGB(255,255,255)    // background color (white)



/**
 * Form of the main window.
 * Contains objects on the main screen of the UI (array of CustomPanes).
 */
class CSyncForm : public CFormView
{

private:

    // true if the UI buttons are locked
    bool lockedUI;


    /**
     * The list of UI source panes. Each one is a CustomPane
     * and contains all related objects for a syncsource (status icon, label,...)
     */
    std::list<CCustomPane> sourcePanes;

    /// The top bar: syncAll pane.
    CSyncAllPane* syncAllPane;

    /**
     * Returns the corresponding source pane, given its ID.
     * Returns NULL if not found.
     */
    CCustomPane* getSourcePane(const int sourceID);

    /**
     * Just fix to make sure we draw all pane lines even if they overlap.
     * set as "last pane" also if next pane is disabled (dimmed)
     * NOTE: not used (disabled panes are windows displayed normally now)
     */
    //void fixOverlappingPanes();

protected:

    // protected constructor used by dynamic creation
	CSyncForm();
	DECLARE_DYNCREATE(CSyncForm)

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnDraw(CDC* pDC);
    virtual ~CSyncForm();

#ifdef _DEBUG
     virtual void AssertValid() const;
     virtual void Dump(CDumpContext& dc) const;
#endif

   // Generated message map functions
   DECLARE_MESSAGE_MAP()


public:

	//{{AFX_DATA(CSyncForm)
	enum { IDD = IDD_SYNC_FORM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


    CBrush  brushHollow;

    // bitmaps (common for all panes)
    HBITMAP hBmpDarkBlue;
    HBITMAP hBmpBlue;
    HBITMAP hBmpDark;
    HBITMAP hBmpLight;

    // icons (common for all source panes)
    HICON iconMouseOver;  
    HICON iconOk;
    HICON iconAlert;
    HICON iconSpin1;
    HICON iconSpin2;
    HICON iconSpin3;
    HICON iconSpin4;



    /// refresh all UI source panes and syncAll pane
    void refreshSources();

    /// Refresh status text of a source.
    void refreshSourceStatus(const CString& msg, const int sourceID);

    /// Returns the current status text of a source.
    CString getSourceStatus(const int sourceID);

    
    /**
     * Updates all UI panes when a sync for a single source is started.
     * All other sources are set to disabled, then UI is locked.
     */
    void onSyncStarted(const int sourceID);

    /**
     * Updates all UI panes when a syncAll is started.
     * UI gets locked then all source status icons are removed.
     */
    void onSyncAllStarted();

    /**
     * Updates the UI panes: sync ended.
     */
    void onSyncEnded();

    /**
     * Updates the UI panes when a specific source starts sync.
     */
    void onSyncSourceBegin(const int sourceID);

    /**
     * Updates the UI panes when a specific source ends sync.
     */
    void onSyncSourceEnd(const int sourceID);




    /**
     * Lock UI buttons of main window.
     * Buttons are locked when starting sync, to avoid errors clicking
     * quickly on buttons, and avoid displaying the cancel msg together with
     * the full-sync msg.
     * Buttons are locked when canceling sync.
     */
    void lockButtons();

    /**
     * Unlock UI buttons of main window.
     * Buttons are unlocked after the 'ContinueAfterPrepareSync()' method.
     * Buttons are unlocked when the sync process has finished.
     */
    void unlockButtons();

    bool isUILocked();


    afx_msg LRESULT OnInitForm(WPARAM, LPARAM);
    //afx_msg void OnBnClickedMainButSync();
    afx_msg void OnNcPaint( );
    afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
};


/** @} */
/** @endcond */
#endif
