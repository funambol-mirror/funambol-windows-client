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

#ifndef INCL_CUSTOMIZATION
#define INCL_CUSTOMIZATION

#include "vocl/appdefs.h"
#include <string>
#include "winmaincpp.h"

// The application name
#define APP_NAME                            "Funambol"

// The application full name, used also in About screen title
#define PROGRAM_NAME                        "Funambol Windows Sync Client"
#define WPROGRAM_NAME                       TEXT(PROGRAM_NAME)

// UI windows titles
#define PLUGIN_UI_TITLE                     PROGRAM_NAME
#define CONFIG_WINDOW_TITLE                 _T(PROGRAM_NAME)
#define MSGBOX_ERROR_TITLE                  PROGRAM_NAME
#define WMSGBOX_ERROR_TITLE                 TEXT(PROGRAM_NAME)

// Login screen
#define DEFAULT_URL                         "http://my.funambol.com/sync"
#define DEFAULT_USERNAME                    ""
#define DEFAULT_PASSWORD                    ""
#define SHOW_ACCOUNT_LOGIN_INFO             true                       /** If true, a label is displayed in the account screen with basic informations for the user */

// Addin customization
// This macro is used into the Outlook menu. The & is the value used to create a shortcut to open the client
#define ADDIN_MENU_LABEL                    L"Funa&mbol"
#define LAST_COMPATIBLE_VERSION             100000                       /**< "10.0.0" is the latest version compatible with this addin - change this value when addin need to be reinstalled */

// The program folder
#define FUNAMBOL_DIR_NAME                   TEXT("Funambol")

// About screen customization
#define ABOUT_SCREEN_SHOW_COPYRIGHT         1                       /**< if 1, will show the copyright text below */
#define ABOUT_SCREEN_TEXT_COPYRIGHT         "Copyright � 2003 - 2011 Funambol, Inc.\nAll rights reserved."

#define ABOUT_SCREEN_SHOW_MAIN_WEB_SITE     1                       /**< if 1, will show the main web address below */
#define ABOUT_SCREEN_TEXT_MAIN_WEB_SITE     "www.funambol.com"
#define ABOUT_SCREEN_TEXT_PORTAL_WEB_SITE   "http://my.funambol.com"


#define ABOUT_SCREEN_SHOW_LICENSE           1                       /**< if 1, will show the AGPL license text */
#define ABOUT_SCREEN_SHOW_POWERED_BY        0                       /**< if 1, will show the "powered by" image instead of the AGPL license */


// Others
#define VIEW_USER_GUIDE_LINK                0                           /**< 1 to display the 'view userguide' in the UI menu (hidden by default) */
#define USER_GUIDE_LINK                     "http://funambol.com/docs/v80/funambol-outlook-sync-client-user-guide.pdf"
#define PROGRAM_NAME_EXE                    "FunambolClient.exe"        // The application to run
#define SCHED_COMMENT                       TEXT(PROGRAM_NAME) TEXT(" scheduler")
#define OL_PLUGIN_LOG_NAME                  "synclog.txt"
#define ENABLE_ENCRYPTION_SETTINGS          1                           /**< 0 to hide the encryption UI check in the Settings screen */
#define SHOW_ADVANCED_SETTINGS              1                           /**< 0 to hide the advanced source settings (remote URIs) */
#define ASK_SLOW_TIMEOUT                    25                          /**< 25 seconds    */
#define TIME_OUT_ABORT                      4                           /**< 4  seconds    */
#define SCHED_DEFAULT_REPEAT_MINS           30                          /**< 30 minutes    */
#define SCHED_DURATION_DAYS                 1                           /**< 1 day         */
#define MAX_LOG_SIZE                        3000000                     /**< 3 MB          */
#define MAX_SYNCML_MSG_SIZE                 125000                      /**< [bytes], the max syncML message size. default = 125KB */
#define RESPONSE_TIMEOUT                    900                         /**< [seconds], the HTTP timeout on Server response. default = 15 minutes */

// SAPI media props
#define SAPI_LOCAL_QUOTA_STORAGE            "98%"                       /**< the max local storage for all media sources */
#define SAPI_HTTP_REQUEST_TIMEOUT           30                          /**< 30 sec    */
#define SAPI_HTTP_RESPONSE_TIMEOUT          30                          /**< 30 sec    */
#define SAPI_HTTP_UPLOAD_CHUNK_SIZE         30000                       /**< 30 KByte  */
#define SAPI_HTTP_DOWNLOAD_CHUNK_SIZE       30000                       /**< 30 KByte  */
#define SAPI_MAX_RETRY_ON_ERROR             2                           /**< 2 retries */
#define SAPI_SLEEP_TIME_ON_RETRY            500                         /**< 0.5 sec   */
#define SAPI_MIN_DATA_SIZE_ON_RETRY         10000                       /**< 10 KBytes */
#define SAPI_MAX_PICTURE_SIZE               0                           /**< max size of pictures [bytes]. 0 means unlimited. */
#define SAPI_MAX_VIDEO_SIZE                 100 * 1024 * 1024           /**< max size of videos   [bytes]. 100 MB. */
#define SAPI_MAX_FILE_SIZE                  25  * 1024 * 1024           /**< max size of files    [bytes].  25 MB. */
#define PICT_EXTENSION                      ".jpg,.jpeg,.jpe,.gif,.png,.jfif,.jif"
#define VIDEO_EXTENSION                     ".wmv,.mp4,.mov,.3g2,.3gp,.mpeg,.mpg,.mpe,.asf,.movie,.avi,.mpa,.mp2,.m4u,.m4v,.swf,.flv"
#define FILE_EXTENSION                      "!," PICT_EXTENSION "," VIDEO_EXTENSION  ",.tmp"  /**< everything else */

// MediaHub info.
#define MEDIA_HUB_DEFAULT_FOLDER            "MediaHub"     // name of the default folder to store picture/video/files
#define MEDIA_HUB_DEFAULT_LABEL             "MediaHub"      // label that is used in the resources
#define MEDIA_HUB_DEFAULT_ICO               "MediaHubFolder.ico"     // name of the default ico to put in the Desktop.ini

// If true, the client will integrate with Microsoft Outlook for PIM sync.
// If false, all Outlook references are removed (no Redemption/addin DLL registration)
// and only media sync is possible (PIM sources are removed from SOURCE_ORDER_IN_REGISTRY).
#define USE_OUTLOOK                         true

// This is the list of sources visible on UI, by default.
#define SOURCE_ORDER_IN_REGISTRY            "contact,appointment,task,note,picture,video,files"

// set if the sources are enabled/disabled (meaning they can be enabled by settings)
#define CONTACT_SOURCE_ENABLED              true
#define APPOINTMENT_SOURCE_ENABLED          true
#define TASK_SOURCE_ENABLED                 true
#define NOTE_SOURCE_ENABLED                 true
#define PICTURE_SOURCE_ENABLED              true
#define VIDEO_SOURCE_ENABLED                true
#define FILE_SOURCE_ENABLED                 true

// set if the sources allowed enabled/disabled (meaning they can be enabled by settings)
#define CONTACT_SOURCE_ALLOWED              true
#define APPOINTMENT_SOURCE_ALLOWED          true
#define TASK_SOURCE_ALLOWED                 true
#define NOTE_SOURCE_ALLOWED                 true
#define PICTURE_SOURCE_ALLOWED              true
#define VIDEO_SOURCE_ALLOWED                true
#define FILE_SOURCE_ALLOWED                 true


/// If true, a SAPI login is done before every sync, to retrieve config settings for freemium/premium users.
/// If false, the default settings are always used (DEFAULT_AUTO_SYNC, DEFAULT_<source>_ALLOWED)
#define ENABLE_SERVICE_PROFILING            false
#define DEFAULT_AUTO_SYNC                   true     /**< false means the scheduled service is not allowed by default */

#define ENABLE_LOGIN_ON_ACCOUNT_CHANGE      false    /**< If true, executes a SAPI login on account change*/
#define LOGIN_TIMEOUT                       30       /**< Timeout (in seconds) for the SAPI login */


/// In case of SyncML status code 402 (payment required) a warning query is prompted to the user
/// asking to continue charging the account, or to abort. If false, no warning popup/charge is executed.
#define ENABLE_PAYMENT_REQUIRED_CHARGE      false
#define RESTORE_CHARGE_TIMEOUT              30       /**< Timeout (in seconds) for restore charge sapi request. */
#define DATA_PLAN_WEB_PAGE                  ""


// List of available sync modes for each source (comma separated values).
// These are the values available from the client's settings for each source.
#define CONTACTS_SYNC_MODES                 SYNC_MODE_TWO_WAY
#define APPOINTMENTS_SYNC_MODES             SYNC_MODE_TWO_WAY
#define TASKS_SYNC_MODES                    SYNC_MODE_TWO_WAY
#define NOTES_SYNC_MODES                    SYNC_MODE_TWO_WAY
#define PICTURES_SYNC_MODES                 SYNC_MODE_TWO_WAY "," SYNC_MODE_ONE_WAY_FROM_CLIENT "," SYNC_MODE_ONE_WAY_FROM_SERVER
#define VIDEOS_SYNC_MODES                   SYNC_MODE_TWO_WAY "," SYNC_MODE_ONE_WAY_FROM_CLIENT "," SYNC_MODE_ONE_WAY_FROM_SERVER
#define FILES_SYNC_MODES                    SYNC_MODE_TWO_WAY

// Default sync mode for each source.
// It MUST be one of the values specified in the list of available sync modes above.
#define DEFAULT_CONTACTS_SYNC_MODE          SYNC_MODE_TWO_WAY
#define DEFAULT_APPOINTMENTS_SYNC_MODE      SYNC_MODE_TWO_WAY
#define DEFAULT_TASKS_SYNC_MODE             SYNC_MODE_TWO_WAY
#define DEFAULT_NOTES_SYNC_MODE             SYNC_MODE_TWO_WAY
#define DEFAULT_PICTURES_SYNC_MODE          SYNC_MODE_TWO_WAY
#define DEFAULT_VIDEOS_SYNC_MODE            SYNC_MODE_TWO_WAY
#define DEFAULT_FILES_SYNC_MODE             SYNC_MODE_TWO_WAY

#define SCHEDULED_MINUTES_VALUES            "30(default),45"
#define SCHEDULED_HOURS_VALUES              "1,2,4,6,8,12,24"

#define ENABLE_COMPRESSION					true

// If true, a menu "test popups" will be added in main screen to test all 
// possible strings (popups and source status)
#define TEST_POPUPS                         false

// Win registry root context.
// This is NOT intended to be customized: we need to use the same registry keys to ensure correct checks
// between different versions of the client (i.e. avoid installing 2 plugins, addin cleanup)
// Note: in case of change, please make sure at least one "/" exist.
#define PLUGIN_ROOT_CONTEXT                 "Funambol/OutlookClient"

// Application data folder, the context under %APPDATA%.
// It is used to save the log file, the cache and all the user's data.
#define APPDATA_CONTEXT                     "Funambol\\WindowsClient"

// The deviceId will be like: "fol-b64(%1:%2)" - see OutlookConfig::setUniqueDevID()
// where %1 is the local machine name, %2 is the Windows current user name (encoded in base64)
#define DEVICE_ID_PREFIX                    "fol"

// The user-agent is this fixed value + the funambol sw version.
// For example: "Funambol Windows Sync Client v. 10.0.0"
#define FUNAMBOL_USER_AGENT                 "Funambol Windows Sync Client"

// Auto-update feature
#define UP_URL_RESOURCE                     "/sapi/profile/client?action=get-update-info"
#define CLIENT_PLATFORM                     "windows"

#include "base/util/StringBuffer.h"


class DLLCustomization {
public:
    static const bool defaultUseSubfolders              = true;     /**< include subfolders check (PIM) */
    static const bool removeFilteredDataOnCleanup       = true;     /**< send filtered out items as Delete items, in case of refresh sync */
    static const bool sendMovedAsNew                    = false;    /**< send items as new instead of updated, if moved to another folder */
    static const bool dontSendFilteredItemsAsDeleted    = true;     /**< don't send items out of the calendar filter as Deleted items */
    static const bool neverSendPhotos                   = false;    /**< don't send contact's photo */
    static const bool saveFileAs                        = false;    /**< save the contact's "fileAs" field, otherwise it's auto generated by Outlook */
    static const bool syncAttendees                     = true;     /**< to sync calendar's attendees */
    static const bool sendTimezone                      = true;     /**< to avoid sending events tz */
    static const bool continueOnSlowWithOneWay          = false;    /**< will not auto-continue a slow if sync direction is set to one-way */
    static const bool warnOnLargeDelete                 = false;    /**< If 50% of items or more are deleted, displays a warning */

    /// to validate custom X-foo properties
    static bool validateExtraProperty(const std::wstring & name) {
        const wchar_t * n = name.c_str();
        return
            (!wcsncmp(n,X_PREFIX,wcslen(X_PREFIX)) && // any X- Properties
            wcsncmp(n,X_FUNAMBOL_PREFIX,wcslen(X_FUNAMBOL_PREFIX)) && // any non-funambol X- Properties
            wcsncmp(n,X_MICROSOFT_PREFIX,wcslen(X_MICROSOFT_PREFIX)) && // any non-microsoft properties
            wcscmp(n,X_WM_CLIENT_CONTAINER_ID) && wcscmp(n,X_WM_CLIENT_CONTAINER_NAME)) // not certain WM props
            ;
    }

    // Source defaults
    static const char *  sourceDefaultEncoding;         /**< the default encoding for synsources */
    static const bool    sourceNotesDefaultSif;         /**< default notes data format (true = SIF, false = vnote) */
    static const char *  sourceNotesSifUri;             /**< the default sources URI for SIF notes */
    static const char *  sourceNotesVnoteUri;           /**< the default sources URI for vNote */
    static const char *  sourceTasksVcalUri;            /**< the default sources URI for vEvent */
    static const char *  sourceCalendarVcalUri;         /**< the default sources URI for vTodo */
    static const char *  sourceContactsVcardUri;        /**< the default sources URI for vCard */
    static const char *  sourcePicturesUri;             /**< the default sources URI for pictures */
    static const char *  sourceVideosUri;               /**< the default sources URI for videos */
    static const char *  sourceFilesUri;                /**< the default sources URI for files */

    // For upgrades
    static const bool shouldFakeOldFunambolSwv = false; /**< if true, the installed version is replaced by a custom value */
    static const int fakeOldFunambolSwv = 80100;        /**< the custom value (see shouldFakeOldFunambolSwv) */

};

#endif
