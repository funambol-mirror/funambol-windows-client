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

#ifndef INCL_UTILS_DLL
#define INCL_UTILS_DLL

/** @cond OLPLUGIN */
/** @addtogroup utils */
/** @{ */

#include "base/fscapi.h"
#include "base/log.h"
#include "spds/SyncReport.h"
#include "WindowsSyncSource.h"
#include "outlook/ClientRecurrence.h"
#include "spds/MappingStoreBuilder.h"
#include "winmaincpp.h"

#include <string>

/// Source panes used by UI
#define SYNCSOURCE_CALENDAR     1
#define SYNCSOURCE_CONTACTS     2
#define SYNCSOURCE_NOTES        3
#define SYNCSOURCE_TASKS        4
#define SYNCSOURCE_PICTURES     5
#define SYNCSOURCE_FILES        6
#define SYNCSOURCE_VIDEOS       7



/// Used as mapping for the updates choice for the OpenMessageBox for the UI.
/// With this code type we map the action and the message for the user for the update

#define TYPE_SKIPPED_ACTION             0   // the user has choosen to skip the optional update
#define TYPE_NOW_LATER_SKIP_OPTIONAL    1
#define TYPE_NOW_LATER_RECCOMENDED      2
#define TYPE_NOW_LATER_MANDATORY        3
#define TYPE_NOW_EXIT_MANDATORY         4



/*-----------utils functions -----------------*/

// Open a message box with a timeout
int TimedMessageBox(HWND hwndOwner, LPCTSTR pszMessage, LPCTSTR pszTitle, UINT flags, DWORD dwTimeout);
// Display a message box only if not scheduled sync.
int safeMessageBox (const char*   message, const char*   title = NULL, unsigned int flags = 0);
int wsafeMessageBox(const WCHAR* wmessage, const WCHAR* wtitle = NULL, unsigned int flags = 0);

_declspec(dllexport) void printLog(const char* msg,  const char* level = LOG_INFO);
_declspec(dllexport) void printLog(const WCHAR* msg, const char* level = LOG_INFO);

bool  isSIF            (const std::wstring& dataType);
bool  isSIF            (const std::string&  dataType);
bool isAcceptedDataType(const std::wstring& dataType);
bool isAcceptedDataType(const std::string&  dataType);
char* syncModeName     (SyncMode code);
bool isFullSyncMode    (SyncMode mode);

/// Returns true if the source is a PIM source (contacts, calendar, tasks or notes)
_declspec(dllexport) bool isPIMSource(const char* sourceName);

/// Returns true if the source is a Media source (pictures, videos or files)
_declspec(dllexport) bool isMediaSource(const char* sourceName);

void  toWindows        (char* str);

/**
 * Returns the path of current user's application data folder "%APPDATA%/APPDATA_CONTEXT".
 * For example:
 * "C:\Users\Settimio\AppData\Roaming\Funambol\WindowsClient"
 * The string returned is allocated new, so MUST be freed by caller.
 * Returns NULL in case of errors (set lastErrorMessage).
 *
 * @return   path of current user's tmp folder under 'application data'
 */
WCHAR* readAppDataPath ();

/**
 * Returns the path of file where data files for current user are stored.
 * It is located under 'application data' folder. For example:
 * "C:\Users\Settimio\AppData\Roaming\Funambol\WindowsClient"
 * The string returned is allocated new, so MUST be freed by caller.
 * Returns NULL in case of errors (set lastErrorMessage).
 *
 * @return   path of current user's tmp folder under 'application data'
 */
WCHAR* readDataPath    (const WCHAR* itemType);

/**
 * Returns the path where sapi cache files for current user are stored.
 * It is located under 'application data / sapi_media_storage' folder.
 */
StringBuffer getSapiCacheDir();

/**
 * Returns the path where PIM cache files for current user are stored.
 * It is located under 'application data' folder.
 */
StringBuffer getPIMCacheDir();

/**
 * Returns the path where log file for current user is stored.
 * It is located under 'application data' folder.
 */
StringBuffer getLogFileDir();

int    makeDataDirs    ();
int    getWindowsUser  (std::wstring& userName);
int    getWindowsUserEx(std::wstring& userName);

/**
 * ------ DEPRECATED ------
 * Returns the default path to store pictures 
 * (shell folder 'pictures' for this user)
 */
StringBuffer getDefaultPicturesPath();

/**
 * ------ DEPRECATED ------
 * Returns the default path to store files 
 * (shell folder 'my documents' for this user)
 */
StringBuffer getDefaultFilesPath();

/**
 * ------ DEPRECATED ------
 * Returns the default path to store videos 
 * (shell folder 'my videos' for this user)
 */
StringBuffer getDefaultVideosPath();

/**
 * ------ DEPRECATED ------
 * Returns the default path to store Media (My Documents)
 * (shell folder 'My Documents' for this user)
 */
StringBuffer getDefaultMyDocumentsPath();

/**
 * Returns true if the media hub folder is set. false otherwise
*/
bool isMediaHubFolderSet();

std::wstring readFromFile          (const std::wstring& filePath);
int          writeToFile           (const std::wstring& content, const std::wstring& filePath, const WCHAR* mode = L"w");
int          writeToFile           (const std::string&  content, const std::string&  filePath, const char*  mode = "w");
std::string  getSyncMutexName      ();
char*        readSystemErrorMsg    (DWORD errorCode = 0);
std::wstring getSafeItemName       (ClientItem* cItem);
//void         printReport           (SyncReport* sr, SyncSource** sources);
char*        friendlyName          (const char* sourceName);

int          getBuildNumberFromVersion(const char* swv);
long         variantTimeToTimeStamp   (const double vTime);

_declspec(dllexport) int syncSourceNameToIndex(const StringBuffer& sourceName);
_declspec(dllexport) StringBuffer syncSourceIndexToName(const int sourceID);

StringBuffer getDefaultSyncMode(const char* sourceName);

/**
 * Used to quicky check if a specific source is visible or not.
 * It checks the sourcesVisible array.
 */
bool isSourceVisibleA(const char* sourceName);

/**
 * Used to quicky check if a specific source is visible or not (WCHAR version).
 * It checks the sourcesVisible array.
 */
bool isSourceVisible(const WCHAR* sourceName);

/**
 * return true if at least one PIM source is visible. false otherwise (no outlok installed)
 */
bool arePIMSourcesVisible();

/**
 * Used to check the total number of sources visible.
 * It checks the sourcesVisible array.
 */
int countSourceVisible();

/**
 * Used to quicky check if a specific source is enabled or not, checking
 * the configuration. If a source is disabled, it's greyed out in UI.
 */
bool isSourceEnabled(const char* sourceName);

bool isWindowsXP();

/**
 * Registers a DLL to the system (a COM or ActiveX library).
 * It's the same as calling "regsvr32.exe <dllPath>" from a command line.
 * @param dllPath    the full path of DLL library to register
 * @param bRegister  if true (default) will register the DLL
 *                   if false, will unregister the DLL (like "regsvr32 /u")
 * @return           S_OK = 0 if no error, 
 *                   E_INVALIDARG if invalid arg passed, 
 *                   E_ABORT if registration aborted
 *                   a value < 0 for other errors registration error
 */
HRESULT registerDLL(const char* dllPath, bool bRegister = true);

/**
 * Is the given status code an error status code? Error codes are the ones
 * outside the range 200-299.
 *
 * @param status the status code to check
 */
inline static bool isErrorStatus(int status) {
    return (status) && ((status < 200) || (status > 299));
}


/**
 * Extends MappingStoreBuilder to define the mappings files for PIM sources.
 * They are placed under the 'application data' folder for this user.
 */
class PIMMappingStoreBuilder : public MappingStoreBuilder {

public:
    PIMMappingStoreBuilder() {}
    virtual ~PIMMappingStoreBuilder() {}

    /**
    * It creates a new instance of the default KeyValueStore.
    * It is a property file.
    */
    virtual KeyValueStore* createNewInstance(const char* name) const {
        StringBuffer fullName = getPIMCacheDir();
        if (createFolder(fullName.c_str())){
            LOG.error("WindowsMappingStoreBuilder::createNewInstance(): error creating config folder");
        }
        fullName += "/"; 
        fullName += name;
        fullName += ".map";
        return new PropertyFile(fullName);
    }
};


/** @} */
/** @endcond */
#endif
