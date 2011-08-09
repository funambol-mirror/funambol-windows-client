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

#include "base/util/StringBuffer.h"
#include "AnimatedIcon.h"
#include "MainSyncFrm.h"

class CSyncForm;

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */


enum PANE_STATE {
    PANE_STATE_NORMAL,          // normale state
    PANE_STATE_SYNC,            // source under sync
    PANE_STATE_DISABLED,        // source disabled (from settings)
    PANE_STATE_NOT_ALLOWED      // source NOT allowed to sync (can't be enabled from settings)
};


/**
 * Panes (active source buttons) on main window.
 */
class CCustomPane : public CStatic {
	
    DECLARE_DYNAMIC(CCustomPane)

private:

    /// The source ID (SYNCSOURCE_CALENDAR,...)
    int id;

    /// The source name
    Funambol::StringBuffer name;

    /// The source state, see winmaincpp.h: SYNCSOURCE_STATE_OK,...
    int sourceState; 
    
    /// The state of the pane, see PANE_STATE enum.
    PANE_STATE state;

    /// True if the mouse is over this pane.
    bool mouseOver;

    /// pane index: the first from top is index 0.
    int index;

    /// True if this pane is the last one on the bottom.
    bool lastPane;

    /// Pane size (pixels)
    CSize size;

    /// Icons size (pixels)
    CSize iconSize;


    // main label
    CStatic sourceLabel;
    CString labelText;

    // status label
    CStatic statusLabel;
    CString statusText;
    
    CAnimatedIcon sourceIcon;
    CAnimatedIcon statusIcon;

    HICON sourceIconEnabled;
    HICON sourceIconDisabled;

    bool showStatusIcon;
    bool clicked;

    CFont fontBold;
    CFont fontNormal;

    double dpiX, dpiY;


    /// pointer to the SyncForm instance
    CSyncForm* syncForm;

    int counterAnim;


    void initialize();

    void initializeFonts();

    CString getLastSyncStatusText();

    /// sets the status icon according to source status (ok, alert, none)
    void refreshStatusIcon();


public:

	CCustomPane(CSyncForm* caller, const int id, const int ix, bool last = false);
	virtual ~CCustomPane();
    CCustomPane(const CCustomPane& objectSrc);

    void refresh();

    int getId()               const { return id; }
    int getIndex()            const { return index; }
    CSyncForm* getCallerWnd() const { return syncForm; }
    bool isLastPane()         const { return lastPane; }

    void setLastPane  (bool val)    { lastPane = val;  }

    /// Sets a custom status text
    void setStatusText(const CString& msg);

    /// Gets the current status text (buffered)
    CString& getStatusText() { return statusText; }

    /// Sets a custom status icon
    void setStatusIcon(HICON hIcon);


    
    /**
     * Returns a pointer to the syncsource config (externally owned)
     * It's read each time because the config object can be read in
     * several points, and a read() causes a refresh of ssource pointers.
     */
    SyncSourceConfig* getSSConfig();

    /**
     * Called when sync started for this source.
     * Sets state to SYNC and refresh.
     */
    void onSyncStarted();

    /**
     * Called when sync ended: stop the animation and refresh.
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
    afx_msg void OnTimer( UINT_PTR nIDEvent );

};

/** @} */
/** @endcond */
