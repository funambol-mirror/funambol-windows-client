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

#pragma once

#include "base/util/StringBuffer.h"
#include "AnimatedIcon.h"
#include "MainSyncFrm.h"

class CSyncForm;

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */


enum SYNCALL_PANE_STATE {
    SYNCALL_PANE_STATE_NORMAL,          // normale state
    SYNCALL_PANE_STATE_SYNC             // under sync
};


/**
 * SyncAll pane on main window.
 */
class CSyncAllPane : public CStatic {
	
    DECLARE_DYNAMIC(CSyncAllPane)

private:
    
    /// The state of the pane, see SYNCALL_PANE_STATE enum.
    SYNCALL_PANE_STATE state;

    /// True if the mouse is over this pane.
    bool mouseOver;

    /// Pane size (pixels)
    CSize size;

    /// Icons size (pixels)
    CSize iconSize;


    // main label
    CStatic label;
    CString labelText;
    
    CAnimatedIcon leftIcon;
    CAnimatedIcon statusIcon;

    HICON iconLogo;         // the logo on the left (fixed)
    HICON iconSyncAll;      // the icon for syncAll, during normal state
    HICON iconCancel;       // the icon for cancel sync, during sync state

    bool clicked;

    CFont fontBold;

    double dpiX, dpiY;


    /// pointer to the SyncForm instance
    CSyncForm* syncForm;


    void initialize();

    void initializeFonts();


public:

	CSyncAllPane(CSyncForm* caller);
	virtual ~CSyncAllPane();
    CSyncAllPane(const CSyncAllPane& objectSrc);

    CSyncForm* getCallerWnd() const { return syncForm; }

    void refresh();

    /**
     * Called when sync started (for a single source).
     * Sets state to SYNC and refresh.
     */
    void onSyncStarted();

    /**
     * Called when sync ended.
     * Sets state to NORMAL and refresh.
     */
    void onSyncEnded();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnMouseLeave();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnPaint( );
    
};

/** @} */
/** @endcond */
