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
#include "customization.h"
#ifndef INCL_HWND_FUNCTIONS
#define INCL_HWND_FUNCTIONS

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

/** @cond DEV */
#define ID_MYMSG_SYNC_BEGIN           (WM_APP+1)
#define ID_MYMSG_SYNC_END             (WM_APP+2)
#define ID_MYMSG_SYNCSOURCE_BEGIN     (WM_APP+3)
#define ID_MYMSG_SYNCSOURCE_END       (WM_APP+4)
#define ID_MYMSG_SYNC_ITEM_SYNCED     (WM_APP+5)
#define ID_MYMSG_SYNC_TOTALITEMS      (WM_APP+6)
#define ID_MYMSG_STARTSYNC_ENDED      (WM_APP+7)
#define ID_MYMSG_REFRESH_STATUSBAR    (WM_APP+8)
#define ID_MYMSG_POPUP                (WM_APP+10)
#define ID_MYMSG_OK                   (WM_APP+11)
#define ID_MYMSG_CANCEL_SYNC          (WM_APP+12)
#define ID_MYMSG_SYNC                 (WM_APP+13)
#define ID_MYMSG_SAPI_PROGRESS        (WM_APP+14)

#define ID_MYMSG_CHECK_MEDIA_HUB_FOLDER		 (WM_APP+20)
#define ID_MYMSG_SCHEDULER_DISABLED			 (WM_APP+21)
#define ID_MYMSG_REFRESH_SOURCES			 (WM_APP+22)
#define ID_MYMSG_SAPILOGIN_BEGIN			 (WM_APP+23)
#define ID_MYMSG_SAPILOGIN_ENDED			 (WM_APP+24)
#define ID_MYMSG_SAPI_RESTORE_CHARGE_BEGIN   (WM_APP+25)
#define ID_MYMSG_DOSAPI_RESTORE_CHARGE_ENDED (WM_APP+26)


/** @endcond */

#define PLUGIN_UI_CLASSNAME         _T("FunambolApp")

#include "windows.h"

/**
 * Contains methods to find the UI window handle.
 * The hanndle is necessary to send messages to UI.
 */
class HwndFunctions
{
public:
	static HWND wnd;

    /// find funambol main window, if present
	static int findFunambolWindow();
	static void initHwnd();
    static HWND getWindowHandle();
};

/** @} */
/** @endcond */
#endif
