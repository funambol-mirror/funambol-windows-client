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

#ifndef SECURITY_WIN32
#define SECURITY_WIN32
#endif

#include <shlobj.h>                     // Used by SHGetFolderPath()
#include <direct.h>                     // Used by _wmkdir()
#include <errno.h>                      // Used by _get_errno()
#include <security.h>                   // Used by GetUserNameEx()

#include "spdm/constants.h"
#include "spdm/DMTreeFactory.h"
#include "spds/DataTransformerFactory.h"
#include "spds/B64Encoder.h"
#include "spds/B64Decoder.h"
#include "spds/DESEncoder.h"
#include "spds/DESDecoder.h"

#include "outlook/ClientException.h"
#include "outlook/utils.h"
#include "winmaincpp.h"
#include "OutlookConfig.h"
#include "utils.h"
#include "customization.h"

#include "base/adapter/PlatformAdapter.h"


using namespace std;



static HWND g_hwndTimedOwner;
static BOOL g_bTimedOut;

/**
 *  MessageBoxTimer.
 *  The timer callback function that posts the fake quit message.
 *  This function causes the message box to exit because the message box
 *  has determined that the application is exiting.
 *
 */
static void CALLBACK MsgBoxTimer(HWND hw, UINT uiMsg, UINT idEv, DWORD time) {
    g_bTimedOut = TRUE;
    if (g_hwndTimedOwner) {
        EnableWindow(g_hwndTimedOwner, TRUE);
    }
    PostQuitMessage(0);
}


/**
 *  TimedMessageBox.
 *  The same as the standard MessageBox, except that TimedMessageBox
 *  also accepts a timeout. If the user does not respond within the
 *  specified timeout, the value 0 is returned instead of one of the
 *  ID* values.
 *
 */
int TimedMessageBox(HWND hwndOwner,
                    LPCTSTR pszMessage,
                    LPCTSTR pszTitle,
                    UINT flags,
                    DWORD dwTimeout) {

    UINT idTimer;
    int iResult;

    g_hwndTimedOwner = NULL;
    g_bTimedOut = FALSE;

    if (hwndOwner && IsWindowEnabled(hwndOwner)) {
        g_hwndTimedOwner = hwndOwner;
    }

    // Set a timer to dismiss the message box.
    idTimer = SetTimer(NULL, 0, dwTimeout, (TIMERPROC)MsgBoxTimer);

    iResult = MessageBox(hwndOwner, pszMessage, pszTitle, flags);

    // Finished with the timer.
    KillTimer(NULL, idTimer);


    // See if there is a WM_QUIT message in the queue if we timed out.
    // Eat the message so we do not quit the whole application.
    if (g_bTimedOut) {
        MSG msg;
        PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);
        iResult = -1;
    }

    return iResult;
}





/**
 * Returns true if dataType is one of accepted SIF formats. (wstring version)
 */
bool isSIF(const wstring& dataType) {

    if ( dataType == L"text/x-s4j-sifc" ||
         dataType == L"text/x-s4j-sife" ||
         dataType == L"text/x-s4j-sift" ||
         dataType == L"text/x-s4j-sifn" ){
        return true;
    }
    else {
        return false;
    }
}

/**
 * Returns true if dataType is one of accepted SIF formats. (string version)
 */
bool isSIF(const string& dataType) {

    if ( dataType == "text/x-s4j-sifc" ||
         dataType == "text/x-s4j-sife" ||
         dataType == "text/x-s4j-sift" ||
         dataType == "text/x-s4j-sifn" ) {
        return true;
    }
    else {
        return false;
    }
}


/**
 * Returns true if dataType is one of accepted formats by Outlook plugin. (wstring version)
 */
bool isAcceptedDataType(const wstring& dataType) {

    if ( dataType == L"text/x-s4j-sifc"  ||
         dataType == L"text/x-s4j-sife"  ||
         dataType == L"text/x-s4j-sift"  ||
         dataType == L"text/x-s4j-sifn"  ||
         dataType == L"text/x-vcard"     ||
         dataType == L"text/x-vnote"     ||
         dataType == L"text/x-vcalendar"){
        return true;
    }
    else {
        return false;
    }
}

/**
 * Returns true if dataType is one of accepted formats by Outlook plugin. (string version)
 */
bool isAcceptedDataType(const string& dataType) {

    if ( dataType == "text/x-s4j-sifc"  ||
         dataType == "text/x-s4j-sife"  ||
         dataType == "text/x-s4j-sift"  ||
         dataType == "text/x-s4j-sifn"  ||
         dataType == "text/x-vcard"     ||
         dataType == "text/x-vnote"     ||
         dataType == "text/x-vcalendar" ) {
        return true;
    }
    else {
        return false;
    }
}




/**
 * Returns the syncmode name given its code.
 */
char* syncModeName(SyncMode code) {

	switch (code) {
		case SYNC_NONE:
			return "none";
			break;
		case SYNC_TWO_WAY:
        case SYNC_TWO_WAY_BY_SERVER:
			return "two-way";
			break;
		case SYNC_SLOW:
			return "slow";
			break;
		case SYNC_ONE_WAY_FROM_CLIENT:
        case SYNC_ONE_WAY_FROM_CLIENT_BY_SERVER:
			return "one-way-from-client";
			break;
		case SYNC_REFRESH_FROM_CLIENT:
        case SYNC_REFRESH_FROM_CLIENT_BY_SERVER:
			return "refresh-from-client";
			break;
		case SYNC_ONE_WAY_FROM_SERVER:
        case SYNC_ONE_WAY_FROM_SERVER_BY_SERVER:
			return "one-way-from-server";
			break;
		case SYNC_SMART_ONE_WAY_FROM_CLIENT:
			return "smart-one-way-from-client";
			break;
		case SYNC_SMART_ONE_WAY_FROM_SERVER:
			return "smart-one-way-from-server";
			break;
		case SYNC_REFRESH_FROM_SERVER:
        case SYNC_REFRESH_FROM_SERVER_BY_SERVER:
			return "refresh-from-server";
			break;
		default:
			return NULL;
			break;
	}
}


/**
 * Returns true if the passed sync mode is a 'full sync mode'
 * (slow sync, refresh from client, refresh from server).
 */
bool isFullSyncMode(SyncMode mode) {

    if ( mode == SYNC_SLOW ||
         mode == SYNC_REFRESH_FROM_SERVER ||
         mode == SYNC_REFRESH_FROM_CLIENT ) {
        return true;
    }
    else {
        return false;
    }
}


bool isPIMSource(const char* sourceName) {
    
    if (!sourceName) return false;

    if (!strcmp(sourceName, CONTACT_) ||
        !strcmp(sourceName, APPOINTMENT_) ||
        !strcmp(sourceName, TASK_) ||
        !strcmp(sourceName, NOTE_)) {
        return true;
    }

    return false;
}

bool isMediaSource(const char* sourceName) {
    
    if (!sourceName) return false;

    if (!strcmp(sourceName, PICTURE_) ||
        !strcmp(sourceName, VIDEO_) ||
        !strcmp(sourceName, FILES_)) {
        return true;
    }

    return false;
}



/**
 * Convert the path in Windows format, changing the slashes in back-slashes.
 * @param str - [IN-OUT] the string to convert
 */
void toWindows(char* str) {
    int i=0;
    while (str[i]) {
        if (str[i] == '/') {
            str[i] = '\\';
        }
        i++;
    }
}


WCHAR* readAppDataPath() {

    wchar_t p[MAX_PATH_LENGTH];

    if ( FAILED(SHGetSpecialFolderPath(NULL, p, CSIDL_APPDATA, 0)) ) {
        LOG.error("Error reading the application data folder, code %d", GetLastError());
        return NULL;
    }

    wstring path(p); 
    path += TEXT("\\");
    path += TEXT(APPDATA_CONTEXT);
    return wstrdup(path.c_str());
}


WCHAR* readDataPath(const WCHAR* itemType) {

    WCHAR* dataPath = readAppDataPath();
    if (!dataPath || !itemType) {
        return NULL;
    }

    WCHAR* oldItemsPath = new WCHAR[wcslen(dataPath) + wcslen(itemType) + 5];
    wsprintf(oldItemsPath, L"%s\\%s.db", dataPath, itemType);

    delete [] dataPath;
    return oldItemsPath;
}

StringBuffer getSapiCacheDir() {

    WCHAR* wcachePath = readAppDataPath();

    StringBuffer cachePath;
    cachePath.convert(wcachePath);
    if (!cachePath.endsWith("/") && !cachePath.endsWith("\\")) {
        cachePath += "/";
    }
    cachePath += SAPI_STORAGE_FOLDER;
    delete [] wcachePath;

    return cachePath;
}

StringBuffer getPIMCacheDir() {

    WCHAR* wcachePath = readAppDataPath();

    StringBuffer cachePath;
    cachePath.convert(wcachePath);
    delete [] wcachePath;

    return cachePath;
}

StringBuffer getLogFileDir() {

    WCHAR* wlogDir = readAppDataPath();

    StringBuffer logDir;
    logDir.convert(wlogDir);
    delete [] wlogDir;

    return logDir;
}


// DEPRECATED - not used
StringBuffer getDefaultPicturesPath() {

    StringBuffer path;
    WCHAR buf[MAX_PATH];

    SHGetSpecialFolderPath(NULL, buf, CSIDL_MYPICTURES, 0);
    if (buf && wcslen(buf) > 0) {
        path.convert(buf);
    }
    return path;
}

// DEPRECATED - not used
StringBuffer getDefaultFilesPath() {

    StringBuffer path;
    WCHAR buf[MAX_PATH];

    SHGetSpecialFolderPath(NULL, buf, CSIDL_MYDOCUMENTS, 0); 
    if (buf && wcslen(buf) > 0) {
        path.convert(buf);
    }
    return path;
}

// DEPRECATED - not used
StringBuffer getDefaultVideosPath() {

    StringBuffer path;
    WCHAR buf[MAX_PATH];

    SHGetSpecialFolderPath(NULL, buf, CSIDL_MYVIDEO, 0); 
    if (buf && wcslen(buf) > 0) {
        path.convert(buf);
    }
    return path;
}

StringBuffer getDefaultMyDocumentsPath() {

    StringBuffer path;
    WCHAR buf[MAX_PATH];

    SHGetSpecialFolderPath(NULL, buf, CSIDL_MYDOCUMENTS, 0); 
        
    if (buf && wcslen(buf) > 0) {
        path.convert(buf);
    }
    return path;
}

bool isMediaHubFolderSet() {
    
    OutlookConfig* config = OutlookConfig::getInstance();
    StringBuffer fpath("");
    if (config) {
        fpath = config->getSyncSourceConfig(PICTURE_)->getCommonConfig()->getProperty(PROPERTY_MEDIAHUB_PATH);
    }
    if (fpath.empty() == false) {
        return true;
    } 
    return false;

}


/**
 * Reads a file from filesystem and write its content into a string.
 * @param filePath : the path of file to read
 * @return           the wstring with content read (empty if file not found)
 * @note             reads chars from file and use 'toWideChar' function to convert
 *                   data into WCHAR, because we MUST use UTF-8 charset.
 */
wstring readFromFile(const wstring& filePath) {

    FILE* f;
    if ((f  = _wfopen(filePath.c_str(), L"r")) == NULL) {
        // File does not exists...
        return EMPTY_WSTRING;
    }

    string content = EMPTY_STRING;
	char line[1024];
    while(fgets(line, 1024, f) != NULL) {
        content.append(line);
    }

    WCHAR* tmp = toWideChar(content.c_str());
    wstring wcontent = tmp;

    if (tmp) delete [] tmp;
    fflush(f);
    fclose(f);

    return wcontent;
}

/**
 * Write the passed string 'content' into a file on filesystem (wstring version).
 * If file not found, it is created.
 * @param content  : the string to write
 * @param filePath : path of file to write
 * @param mode     : [OPTIONAL] mode of writing (default = "w" = writing, content destroyed)
 * @return           0 if no errors
 * @note             writes chars to file and use 'toMultibyte' function to convert
 *                   data from WCHAR, because we MUST use UTF-8 charset.
 */
int writeToFile(const wstring& content, const wstring& filePath, const WCHAR* mode) {

    if (!mode) {
        setErrorF(getLastErrorCode(), ERR_FILE_OPEN_MODE);
        return 1;
    }

    FILE* f;
    if ((f  = _wfopen(filePath.c_str(), mode)) == NULL) {
        setErrorF(getLastErrorCode(), ERR_WFILE_OPEN, filePath.c_str());
        return 1;
    }

    char* tmp = toMultibyte(content.c_str());
    if (tmp) {
        if (fprintf(f, "%s", tmp) < 0) {
            setErrorF(getLastErrorCode(), ERR_WFILE_WRITE_ON, filePath.c_str());
            return 1;
        }
        delete [] tmp;
    }

    fclose(f);
    return 0;
}


/**
 * Write the passed string 'content' into a file on filesystem (string version).
 * If file not found, it is created.
 * @param content  : the string to write
 * @param filePath : path of file to write
 * @param mode     : [OPTIONAL] mode of writing (default = "w" = writing, content destroyed)
 * @return           0 if no errors
 */
int writeToFile(const string& content, const string& filePath, const char* mode) {

    if (!mode) {
        setErrorF(getLastErrorCode(), ERR_FILE_OPEN_MODE);
        return 1;
    }

    FILE* f;
    if ((f  = fileOpen(filePath.c_str(), mode)) == NULL) {
        setErrorF(getLastErrorCode(), ERR_FILE_OPEN, filePath.c_str());
        return 1;
    }

    if (fprintf(f, "%s", content.c_str()) < 0) {
        setErrorF(getLastErrorCode(), ERR_FILE_WRITE_ON, filePath.c_str());
        return 1;
    }

    fclose(f);
    return 0;
}


/**
 * Create directories for data files under 'application data' dir.
 * @return  0 if no errors
 */
int makeDataDirs() {
    int err = 0;

    WCHAR* path = readAppDataPath();
    if (!path) {
        return 2;
    }
    StringBuffer pt;
    pt.convert(path);
    delete [] path;

    err = createFolder(pt.c_str());
    if (err) {
        setErrorF(getLastErrorCode(), ERR_DIR_CREATE, pt.c_str());
        return 1;
    }
    return err;
}



/**
 * Gets the Windows current user (the one now active).
 * @param   [OUT] the Windows current user
 * @return  0 if no errors
 */
int getWindowsUser(wstring& userName) {

    userName = EMPTY_WSTRING;
    WCHAR user[256];
    unsigned long len = 255;

    if (!GetUserName(user, &len)) {
        DWORD code = GetLastError();
        char* msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_USER_NAME, code, msg);
        delete [] msg;
        return 1;
    }

    userName = user;
    return 0;
}

/**
 * Gets the Windows current user (the one now active) in the
 * extended format "Machine\User".
 * @param   [OUT] the Windows current user
 * @return  0 if no errors
 */
int getWindowsUserEx(wstring& userName) {

    userName = EMPTY_WSTRING;
    WCHAR user[256];
    unsigned long len = 255;

    if (!GetUserNameEx(NameSamCompatible, user, &len)) {
        DWORD code = GetLastError();
        char* msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_USER_NAME, code, msg);
        delete [] msg;
        return 1;
    }

    userName = user;
    return 0;
}



/**
 * Returns the syncMutex unique name: "fol-syncInProgress-xxxxxxx".
 * "xxxxxxx" is the BeginSync timestamp, stored in win registry (HKCU).
 * We don't use OutlookConfig object because the mutex name can be
 * asked BEFORE the initialization of client (so config not yet available).
 * So here we directly access to the win registry.
 */
string getSyncMutexName() {

    DMTree* dmt          = NULL;
    ManagementNode* node = NULL;
    char* value          = NULL;
    string ret = SYNC_MUTEX_NAME;      // By default.

    char context[DIM_MANAGEMENT_PATH];
    sprintf(context, "%s%s%s", PLUGIN_ROOT_CONTEXT, CONTEXT_SPDS_SYNCML, CONTEXT_EXT);

    // Get value.
    dmt = DMTreeFactory::getDMTree(context);
    if (!dmt)   goto finally;
    node = dmt->readManagementNode(context);
    if (!node)  goto finally;
    value = node->readPropertyValue(PROPERTY_SYNC_BEGIN);
    if (!value) goto finally;

    ret.append("-");
    ret.append(value);

finally:
    if (dmt)   delete dmt;
    if (node)  delete node;
    if (value) delete [] value;

    return ret;
}



/**
 * Utility function to retrieve the correspondant message for a generic System error.
 * Pointer returned is allocated new, must be freed by caller.
 *
 * @param errorCode : the code of error (obtained by 'getLastError()')
 * @return            the (new allocated) error message
 */
char* readSystemErrorMsg(DWORD errorCode) {

    if (!errorCode) {
        errorCode = GetLastError();
    }

    char* errorMessage = new char[512];
    memset(errorMessage, 0, 512);

    FormatMessageA(
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                errorCode,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                errorMessage,
                512,
                NULL);

    if (!errorMessage || strlen(errorMessage) == 0) {
        sprintf(errorMessage, ERR_UNKNOWN);
    }
    return errorMessage;
}


/**
 * Function used to display a message-box with a message for the user.
 * The message-box is displayed only if this is NOT a scheduled sync
 * (otherwise the message will be redirected to LOG file)
 *
 * @param message : the message to display
 * @param title   : [OPTIONAL] the title of the message box (default = MSGBOX_ERROR_TITLE)
 * @param flags   : [OPTIONAL] flags for the message-box    (default = MB_OK | MB_ICONEXCLAMATION)
 * @return          the value returned from MessageBox call
 */
int safeMessageBox(const char* message, const char* title, unsigned int flags) {

    int ret = 0;
    if (!message) {
        return -1;
    }

    OutlookConfig* config = OutlookConfig::getInstance();

    // Normal sync
    if (config->getScheduledSync() == false) {
        WCHAR* wtitle = NULL;
        WCHAR* wmessage = toWideChar(message);

        if (!flags) {
            flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
        }

        if (!title) {
            wtitle = wstrdup(WMSGBOX_ERROR_TITLE);
        }
        else {
            wtitle = toWideChar(title);
        }

        ret = MessageBox(NULL, wmessage, wtitle, flags);

        if (wmessage) delete [] wmessage;
        if (wtitle)   delete [] wtitle;
    }

    // Scheduled sync
    else {
        LOG.error(message);
    }

    return ret;
}


/**
 * Function used to display a message-box with a message for the user. (WCHAR version)
 * The message-box is displayed only if this is NOT a scheduled sync
 * (otherwise the message will be redirected to LOG file)
 *
 * @param message : the message to display
 * @param title   : [OPTIONAL] the title of the message box (default = MSGBOX_ERROR_TITLE)
 * @param flags   : [OPTIONAL] flags for the message-box    (default = MB_OK | MB_ICONEXCLAMATION)
 * @return          the value returned from MessageBox call
 */
int wsafeMessageBox(const WCHAR* wmessage, const WCHAR* wtitle, unsigned int flags) {

    int ret = 0;
    if (!wmessage) {
        return -1;
    }

    OutlookConfig* config = OutlookConfig::getInstance();

    // Normal sync
    if (config->getScheduledSync() == false) {

        if (!flags) {
            flags = MB_OK | MB_ICONEXCLAMATION | MB_SETFOREGROUND | MB_APPLMODAL;
        }
        if (!wtitle) {
            wtitle = WMSGBOX_ERROR_TITLE;
        }

        ret = MessageBox(NULL, wmessage, wtitle, flags);
    }

    // Scheduled sync
    else {
        char* message = toMultibyte(wmessage);
        LOG.error(message);
        delete [] message;
    }

    return ret;
}


/**
 * Prints a message (char*) into LOG file. Used by UI.
 */
void printLog(const char* msg, const char* level) {

    if (!msg || !strlen(msg)) {
        return;
    }

    if (!strcmp(level, LOG_INFO)) {
        LOG.info("[UI] %s", msg);
    }
    else if (!strcmp(level, LOG_DEBUG)) {
        LOG.debug("[UI] %s", msg);
    }
    else if (!strcmp(level, LOG_ERROR)) {
        LOG.error("[UI] %s", msg);
    }
    return;
}

/**
 * Prints a message (WCHAR*) into LOG file. Used by UI.
 */
void printLog(const WCHAR* wmsg, const char* level) {

    if (!wmsg || !wcslen(wmsg)) {
        return;
    }
    char* msg = toMultibyte(wmsg);
    printLog(msg, level);
    delete [] msg;
    return;
}



/**
 * Returns a smart name of the item passed, used for log/debug or in
 * case of errors. Usually the "Subject" is returned (if present).
 */
wstring getSafeItemName(ClientItem* cItem) {
    if (!cItem) {
        return L"(null)";
    }

    wstring name = EMPTY_WSTRING;
    try {
        name = cItem->getProperty(L"Subject");
        if (name != EMPTY_WSTRING) {
            return name;
        }
        name = cItem->getID();
        if (name != EMPTY_WSTRING) {
            wstring ret = L"ID = ";
            ret += name;
            return ret;
        }
    }
    catch (ClientException* e) {
        LOG.debug(DBG_SAFE_ITEM_NAME, e->getErrorMsg());
        // Reset error code: we don't consider errors here, and this
        // code could be checked later for the sync result.
        if (getLastErrorCode()) {
            //lastErrorCode = 0;
            resetError();
        }
    }
    return L"(new item)";
}





int syncSourceNameToIndex(const StringBuffer& sourceName)
{
    int id = 0;

    if (sourceName == CONTACT_) {
        id = SYNCSOURCE_CONTACTS;
    }
    else if (sourceName == APPOINTMENT_) {
        id = SYNCSOURCE_CALENDAR;
    }
    else if (sourceName == TASK_) {
        id = SYNCSOURCE_TASKS;
    }
    else if (sourceName == NOTE_) {
        id = SYNCSOURCE_NOTES;
    }
    else if (sourceName == PICTURE_) {
        id = SYNCSOURCE_PICTURES;
    }
    else if (sourceName == VIDEO_) {
        id = SYNCSOURCE_VIDEOS;
    }
    else if (sourceName == FILES_) {
        id = SYNCSOURCE_FILES;
    }

    return id;
}

StringBuffer syncSourceIndexToName(const int sourceID)
{
    switch (sourceID) {
        case (SYNCSOURCE_CONTACTS): return CONTACT_;
        case (SYNCSOURCE_CALENDAR): return APPOINTMENT_;
        case (SYNCSOURCE_TASKS):    return TASK_;
        case (SYNCSOURCE_NOTES):    return NOTE_;
        case (SYNCSOURCE_PICTURES): return PICTURE_;
        case (SYNCSOURCE_VIDEOS):   return VIDEO_;
        case (SYNCSOURCE_FILES):    return FILES_;
        default:                    return "";
    }
}


StringBuffer getDefaultSyncMode(const char* sourceName) {

    if (!sourceName) return "";

    if (!strcmp(sourceName, CONTACT_))     return DEFAULT_CONTACTS_SYNC_MODE;
    if (!strcmp(sourceName, APPOINTMENT_)) return DEFAULT_APPOINTMENTS_SYNC_MODE;
    if (!strcmp(sourceName, TASK_))        return DEFAULT_TASKS_SYNC_MODE;
    if (!strcmp(sourceName, NOTE_))        return DEFAULT_NOTES_SYNC_MODE;
    if (!strcmp(sourceName, PICTURE_))     return DEFAULT_PICTURES_SYNC_MODE;
    if (!strcmp(sourceName, VIDEO_))       return DEFAULT_VIDEOS_SYNC_MODE;
    if (!strcmp(sourceName, FILES_))       return DEFAULT_FILES_SYNC_MODE;

    return SYNC_MODE_TWO_WAY;
}


/**
 * Returns a friendly name for the passed syncsource name.
 */
char* friendlyName(const char* sourceName) {

    if (!sourceName) {
        return EMPTY_STRING;
    }
    if (!strcmp(sourceName, APPOINTMENT_)) {
        return "Calendar";
    }
    else if (!strcmp(sourceName, CONTACT_)) {
        return "Contacts";
    }
    else if (!strcmp(sourceName, NOTE_)) {
        return "Notes";
    }
    else if (!strcmp(sourceName, TASK_)) {
        return "Tasks";
    }
    else if (!strcmp(sourceName, PICTURE_)) {
        return "Pictures";
    }
    else if (!strcmp(sourceName, VIDEO_)) {
        return "Videos";
    }
    else if (!strcmp(sourceName, FILES_)) {
        return "Files";
    }
    return EMPTY_STRING;
}



/**
 * Returns an integer value rapresenting the build number read from
 * the string version passed (e.g. "6.1.12" -> 60112).
 */
int getBuildNumberFromVersion(const char* swv) {

    int major=0, minor=0, build=0;
    if (!swv) {
        return 0;
    }
    sscanf(swv, "%d.%d.%d", &major, &minor, &build);

    if (build > 1000) {
        // Fix for build numbers like "20091022" = date of today :)
        build = 0;
    }
    return (major*10000 + minor*100 + build);
}


bool isSourceVisibleA(const char* sourceName) {

    const ArrayList& sourcesVisible = getConfig()->getSourcesVisible();

    for (int i=0; i<sourcesVisible.size(); i++) {
        StringBuffer* name = (StringBuffer*)sourcesVisible.get(i);
        if (*name == sourceName) {
            return true;
        }
    }
    return false;
}

bool isSourceVisible(const WCHAR* sourceName) {

    StringBuffer name;
    name.convert(sourceName);
    return isSourceVisibleA(name.c_str());
}


int countSourceVisible() {
    const ArrayList& sourcesVisible = getConfig()->getSourcesVisible();
    return sourcesVisible.size();
}

bool isSourceEnabled(const char* sourceName) {

    OutlookConfig* config = OutlookConfig::getInstance();
    WindowsSyncSourceConfig* ssc = config->getSyncSourceConfig(sourceName);
    if (ssc) {
        return ssc->isEnabled();
    }
    return false;
}


//
// ----------------------------------- Time/date functions ----------------------------------------
//

/**
 * Variant time (double format)  ->  Timestamp (long format).
 * NOTE: the internal call 'mktime()' adjusts time to UTC... this is OK because
 *       timestamps always refer to UTC time.
 *
 * @param vTime : input time in variant format (milliseconds from Jan 1 1900)
 * @return        output time as timestamp (seconds from Jan 1 1970)
 */
long variantTimeToTimeStamp(const double vTime) {

    tm t;
    SYSTEMTIME st;
    VariantTimeToSystemTime(vTime, &st);

    t.tm_hour = st.wHour;
    t.tm_mday = st.wDay;
    t.tm_min  = st.wMinute;
    t.tm_mon  = st.wMonth - 1;      // range [0-11]
    t.tm_sec  = st.wSecond;
    t.tm_wday = st.wDayOfWeek;      // range [0-6] (0 = sunday)
    t.tm_year = st.wYear - 1900;    // From 1900
    t.tm_isdst= -1;                 // less than 0 means to auto-adjust standard time or daylight savings time.

    // This call automatically converts to UTC!
    return (long)mktime(&t);
}

bool isWindowsXP() {

    OSVERSIONINFO osvi;    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);

    return osvi.dwMajorVersion < 6; // is xp or 2003. if >= 6 is vista or 7      
}

HRESULT registerDLL(const char* dllPath, bool bRegister) { 

    HRESULT hResult = S_OK;
    if (!dllPath || strlen(dllPath)==0) {
        return 1;
    }

    const WCHAR* wdllPath = toWideChar(dllPath);

    // Load the server dll into our process space. 
    HINSTANCE hOleServerInst = ::LoadLibrary(wdllPath);

    if (hOleServerInst) { 
        HRESULT (STDAPICALLTYPE *pfnRegServer)(void); 

        if (bRegister) { 
            (FARPROC&)pfnRegServer = ::GetProcAddress(hOleServerInst, "DllRegisterServer"); 
        } 
        else { 
            (FARPROC&)pfnRegServer = ::GetProcAddress(hOleServerInst, "DllUnregisterServer"); 
        } 

        if (pfnRegServer) { 
            hResult = (*pfnRegServer)(); 
        } 
        else { 
            hResult = HRESULT_FROM_WIN32(::GetLastError()); 
        } 

        ::CoFreeLibrary(hOleServerInst); 
    } 
    else { 
        hResult = HRESULT_FROM_WIN32(::GetLastError()); 
    }

    delete [] wdllPath;
    return hResult; 
} 
