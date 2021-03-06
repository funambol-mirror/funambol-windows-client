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


#if !defined(AFX_MAINFRM_H__FA98B70F_D0B7_11D3_BC39_00C04F602FEE1__INCLUDED_)
#define AFX_MAINFRM_H__FA98B70F_D0B7_11D3_BC39_00C04F602FEE1__INCLUDED_

/** @cond OLPLUGIN */
/** @addtogroup UI */
/** @{ */

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "HwndFunctions.h"
#include "ConfigFrm.h"


class CSyncForm;



// Thread related
DWORD WINAPI syncThread(LPVOID lpParam);
DWORD WINAPI syncThreadKiller(LPVOID lpParam);
DWORD WINAPI callSAPIRestoreCallThread(LPVOID lpParam);

DWORD WINAPI loginThread(LPVOID lpParam);
DWORD WINAPI loginThreadKiller(LPVOID lpParam);
DWORD WINAPI callSAPIRestoreKiller(LPVOID lpParam);


/**
 * Main window.
 * Contains: 
 * - images of main window
 * - methods to interact with the DLL
 * - methods called for msg mapping on main UI
 *
 * TODO: refactoring: use arrays of ssources
 */
class CMainSyncFrame : public CFrameWnd
{

protected:

    HANDLE hSyncThread;
	HANDLE hLoginThread;
	
    DWORD dwThreadId;
    bool configOpened;
    int dpiX, dpiY;


    // info about the sync in progress
    int currentSource;
    int totalClientItems;
    int totalServerItems;
    int currentClientItem;
    int currentServerItem;

    int itemTotalSize;
    int partialCompleted;
    bool progressStarted;

    CFont fontBold;

    ////// test popups and status text ////////
    /// Usually 0, just set to a value during "Test popups" execution
    int testingStatusText;

    void testAllStatusText(const int sourceID);

    void onTestStatusText();

    /// test all popups and and source status
    afx_msg void OnTestPopups();
    ///////////////////////////////////////////

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CConfigFrame)
    protected:
    virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
    //}}AFX_VIRTUAL

	DECLARE_MESSAGE_MAP()

    
    /// Overrided to dynamically remove the 'view User Guide' button & separator.
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);


public:

    CMainSyncFrame();
    virtual ~CMainSyncFrame();
    DECLARE_DYNCREATE(CMainSyncFrame)


    CConfigFrame* pConfigFrame;

    CSyncForm* syncForm;

	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    void showSettingsWindow(const int paneToDisplay = 1);
    
    // check if the user has set the connection settings
    bool checkConnectionSettings(); 

    int getDpiX() {return dpiX;}
    int getDpiY() {return dpiY;}
  
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    CStatusBar wndStatusBar;
    CSplitter wndSplitter;

	bool bSchedulerWasDisabledByLogin; // if true the scheduler was cancelled after login settings
									   // useful if we want to show a messagebox after the sync process,
									   // for warning the user that the Windows task was deleted by
							           // server settings

	CString lastSyncURL;
	CString lastUserName;			   // used for detecting if was changed
	CString lastUserPassword;		   // used for detecting if was changed


    void OnConfigClosed();

    void StartSync(const int sourceID = -1);

    int CancelSync(bool confirm = true);
	void StartLogin();
	void RestoreCharge(); // starts the thread for SAPI Restore charge

    /// Shows/hides the main screen menu
    void showMenu(bool show);

    //afx_msg void OnUpdatePage(CCmdUI *pCmdUI); //status bar update
    afx_msg LRESULT OnMsgSyncBegin      (WPARAM , LPARAM);
    afx_msg LRESULT OnMsgSyncEnd        (WPARAM , LPARAM);
    afx_msg LRESULT OnMsgSyncSourceBegin(WPARAM , LPARAM);
    afx_msg LRESULT OnMsgSyncSourceEnd  (WPARAM , LPARAM);
    afx_msg LRESULT OnMsgItemSynced     (WPARAM , LPARAM lParam);
    afx_msg LRESULT OnMsgTotalItems     (WPARAM , LPARAM); 
    afx_msg LRESULT OnMsgStartsyncEnded (WPARAM , LPARAM); 
    afx_msg LRESULT OnMsgRefreshStatusBar(WPARAM, LPARAM);

	// sended after login process if task was cancelled from server setting (auto-sync was disabled)
	afx_msg LRESULT OnMsgSchedulerDisabled( WPARAM , LPARAM lParam); 
	afx_msg LRESULT OnMsgRefreshSources (WPARAM , LPARAM);  // for refresh sources PANE after SAPI Login!
	afx_msg LRESULT OnMsgSapiLoginBegin (WPARAM , LPARAM);  // shows a message 
	afx_msg LRESULT OnMsgSAPILoginEnded (WPARAM , LPARAM);

	afx_msg LRESULT OnMsgSapiRestoreChargeBegin(WPARAM wParam, LPARAM lParam); // sapi for restore charge begins
	afx_msg LRESULT OnMsgSapiRestoreChargeEnded(WPARAM wParam, LPARAM lParam); 


    // progress percentage. wparam is the total size. lparam is the partial upload or download
    afx_msg LRESULT OnMsgSapiProgress   (WPARAM , LPARAM);
    
    afx_msg LRESULT OnCheckMediaHubFolder(WPARAM wParam, LPARAM lParam);
    BOOL createMediaHubDesktopIniFile(const char* folderPath, const char* installPath);

    afx_msg LRESULT CancelSync(WPARAM wParam, LPARAM lParam);

    afx_msg LRESULT Synchronize(WPARAM wParam, LPARAM lParam);

    afx_msg void OnFileConfiguration();
    afx_msg void OnToolsFullSync();
    afx_msg void OnFileSynchronize();
    afx_msg void OnToolsSetloglevel();

    afx_msg BOOL OnNcActivate(BOOL bActive);
    afx_msg void OnClose();

    afx_msg LRESULT OnMsgPopup(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOKMsg(WPARAM wParam, LPARAM lParam);
};


/** @} */
/** @endcond */
#endif
