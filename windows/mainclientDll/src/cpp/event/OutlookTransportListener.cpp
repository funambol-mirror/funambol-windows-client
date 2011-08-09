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

#include "event/OutlookTransportListener.h"
#include "base/Log.h"
#include "winmaincpp.h"
#include "HwndFunctions.h"

/*
 * ID_MYMSG_SAPI_PROGRESS:
 * wParam = -2  begin            -> lParam = total size
 * wparam = -1  partial (resume) -> lParam = already exchanged size
 * wparam =  0  in progress      -> lParam = partial exchanged size
 * wParam =  1  end
 */

void OutlookTransportListener::sendDataBegin(TransportEvent &event) {
	LOG.debug("Sending data of size %d...", event.getDataSize());
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_SENDDATA_BEGIN);
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, -2, event.getDataSize());
}

void OutlookTransportListener::sendingData(TransportEvent &event) {
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, 0, event.getDataSize());
    LOG.debug("Sending %d bytes...", event.getDataSize());
}

void OutlookTransportListener::sendDataEnd(TransportEvent &event) {
	LOG.debug("Finished sending data of size %d.", event.getDataSize());
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, 1, event.getDataSize());
    //SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_SENDDATA_END);
}


void OutlookTransportListener::receiveDataBegin(TransportEvent &event) {
    LOG.debug("Receiving data of size %d...", event.getDataSize());
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_RECEIVE_DATA_BEGIN);
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, -2, event.getDataSize());
}

void OutlookTransportListener::receivingData(TransportEvent &event) {
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, 0, event.getDataSize());
    LOG.debug("Reading %d bytes...", event.getDataSize());
}

void OutlookTransportListener::receiveDataEnd(TransportEvent &event) {
    LOG.debug("Finished receiving data of size %d.", event.getDataSize());
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, 1, event.getDataSize());
    //SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_RECEIVE_DATA_END);
}


void OutlookTransportListener::partialData(TransportEvent &event) {
    LOG.debug("Already exchanged data %d.", event.getDataSize());
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SAPI_PROGRESS, -1, event.getDataSize());
}

