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

#ifndef WINMAINCPP
#define WINMAINCPP

/** @cond OLPLUGIN */
/** @addtogroup ClientDLL */
/** @{ */

#define OUTLOOK 1

// ------------------------------- Definitions ------------------------------
// Program parameters:
#define SYNC_MUTEX_NAME                     "fol-SyncInProgress"
#define EMPTY_WSTRING                      L""


// Default remote names:
#define VCARD_DEFAULT_NAME                 L"card"
#define VCALENDAR_DEFAULT_NAME             L"event"
#define VTODO_DEFAULT_NAME                 L"task"
#define VNOTE_DEFAULT_NAME                 L"note"

#define SIFC_DEFAULT_NAME                  L"scard"
#define SIFE_DEFAULT_NAME                  L"scal"
#define SIFT_DEFAULT_NAME                  L"stask"
#define SIFN_DEFAULT_NAME                  L"snote"

#define CALENDAR_REMOTE_NAME               L"calendar"
#define CONTACTS_REMOTE_NAME               L"contacts"
#define TASKS_REMOTE_NAME                  L"tasks"
#define NOTES_REMOTE_NAME                  L"notes"

#define SHARED_SUFFIX                      L"-shared"


#define MAX_PATH_LENGTH                     512

/// If MS Outlook app is installed, this registry key exists and it's not empty (under HKLM)
#define OUTLOOK_EXE_REGKEY                 "Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\OUTLOOK.exe"

/// If Redemption.dll is registered in the system, this registry key exists and it's not empty (under HKCR)
#define REDEMPTION_CLSID_REGKEY            "CLSID\\{03C4C5F4-1893-444C-B8D8-002F0034DA92}\\InprocServer32"

//
// Item Types:
//
/** @cond DEV */
#define MAIL                               L"mail"
#define CONTACT                            L"contact"
#define APPOINTMENT                        L"appointment"
#define TASK                               L"task"
#define JOURNAL                            L"journal"
#define NOTE                               L"note"
#define PICTURE                            L"picture"
#define VIDEO                              L"video"
#define FILES                              L"files"
#define POST                               L"post"
#define DISTRIBUTION_LIST                  L"distribution list"

#define MAIL_                              "mail"
#define CONTACT_                           "contact"
#define APPOINTMENT_                       "appointment"
#define TASK_                              "task"
#define JOURNAL_                           "journal"
#define NOTE_                              "note"
#define PICTURE_                           "picture"
#define VIDEO_                             "video"
#define FILES_                             "files"
#define POST_                              "post"
#define DISTRIBUTION_LIST_                 "distribution list"


/// Name of file to store 'forced' modified appointments
#define APPOINTMENT_FORCED_MODIFIED        L"appointment_modified"

/// Default folder of items in Outlook (replaced by correct path)
#define DEFAULT_FOLDER                     L"DEFAULT_FOLDER"

/// Sapi cache dir name, under appdata dir
#define SAPI_STORAGE_FOLDER                 "sapi_media_storage"

//
// Possible source states (UI labels for each source).
//
#define SYNCSOURCE_STATE_OK                 1
#define SYNCSOURCE_STATE_FAILED             2
#define SYNCSOURCE_STATE_CANCELED           3
#define SYNCSOURCE_STATE_QUOTA_EXCEEDED     4
#define SYNCSOURCE_STATE_STORAGE_FULL       5
#define SYNCSOURCE_STATE_NOT_SUPPORTED      6   // server doesn't support the (sapi) source or sapi not present (comed)


//
// Windows client error codes returned for a sync session.
//
#define WIN_ERR_NONE                        0
#define WIN_ERR_GENERIC                     1
#define WIN_ERR_SYNC_CANCELED               2
#define WIN_ERR_FATAL_OL_EXCEPTION          3   // deprecated
#define WIN_ERR_THREAD_TERMINATED           4
#define WIN_ERR_FULL_SYNC_CANCELED          5   // deprecated
#define WIN_ERR_UNEXPECTED_EXCEPTION        6
#define WIN_ERR_UNEXPECTED_STL_EXCEPTION    7
#define WIN_ERR_SERVER_QUOTA_EXCEEDED       8
#define WIN_ERR_LOCAL_STORAGE_FULL          9
#define WIN_ERR_DROPPED_ITEMS               10
#define WIN_ERR_DROPPED_ITEMS_SERVER        11
#define WIN_ERR_NO_SOURCES                  12
#define WIN_ERR_SAPI_NOT_SUPPORTED          13
#define WIN_ERR_INVALID_CREDENTIALS         401
#define WIN_ERR_REMOTE_NAME_NOT_FOUND       404
#define WIN_ERR_PROXY_AUTH_REQUIRED         407
#define WIN_ERR_AUTOSYNC_DISABLED	        500				// auto-sync is false
#define WIN_ERR_WRONG_HOST_NAME             2001
#define WIN_ERR_NETWORK_ERROR               2050


#define WIN_ERR_PAYMENT_REQUIRED            402 // SAPI respond in a "Alert" Status that the user must pay for that service 



/** @endcond */


// Scheduler defines:
/** @addtogroup scheduler */
/** @{ */
#define SCHED_PARAM                        L"schedule"
#define NEVER                               "Never"
#define EVERY_DAY                           "Every day"
/** @} */



//
// ------------- ERROR MESSAGES -------------
//
/** @cond DEV */
// MessageBox messages


// Error messages
#define ERR_UNKNOWN                         "Unknown error."
#define ERR_OPEN_OUTLOOK                    "Error opening Outlook application."
#define ERR_ATTACH_OUTLOOK                  "Error attaching to Outlook application."
#define ERR_TAG_NOT_FOUND                   "Bad XML format: tag '%ls' not found."
#define ERR_BAD_FOLDER_PATH                 "Bad folder path: %ls"
#define ERR_INSTALL_DIR                     "Error retrieving the install directory path."
#define ERR_THREAD_PRIORITY                 "Error setting sync thread priority: code %d (%s)."
#define ERR_THREAD_NOT_TERMINATED           "Could not terminate the synchronization process. %s"
#define ERR_MUTEX_CREATE                    "Could not create the mutex for synchronization process. %s"
#define ERR_MUTEX_ALREADY_EXISTS            "Error creating the mutex for sync process: mutex already exists. %s"
#define ERR_MUTEX_NOT_RELEASED              "Could not release the mutex of synchronization process. %s"
#define ERR_MUTEX_OPEN                      "Could not open the mutex of synchronization process. %s"
#define ERR_CLASS_REG_FAILED                "Class Registration Failed! class name = %s"
#define ERR_ENCRYPT_DATA                    "Error occurred encrypting private data"
#define ERR_DECRYPT_DATA                    "Error occurred decrypting private data"
#define ERR_NO_SOURCES_TO_SYNC              "No sources to synchronize."
#define ERR_AUTOSYNC_DISABLED               "Sorry, you cannot do automatic sync."

#define ERR_BEGIN_SYNC                      "Error in begin sync of source '%ls'."
#define ERR_CLOSE_OUTLOOK                   "Some error occurred closing Outlook session. Outlook may become unstable."
#define ERR_COM_INITIALIZE                  "Error occurred initializing COM library."
#define ERR_COM_CREATE_INSTANCE             "Error occurred creating an instance for COM library."
#define ERR_END_SYNC                        "Error in end sync of source '%ls'."
#define ERR_APPDATA_PATH                    "Error retrieving current user application data path: code %d (%s)"
#define ERR_FILE_OPEN                       "Error opening file: %s"
#define ERR_WFILE_OPEN                      "Error opening file: %ls"
#define ERR_FILE_WRITE_ON                   "Error writing file: %s"
#define ERR_WFILE_WRITE_ON                  "Error writing file: %ls"
#define ERR_FILE_OPEN_MODE                  "Error writing file: bad fopen mode."
#define ERR_DIR_CREATE                      "Error creating directory: %ls"
#define ERR_DEFAULT_SSCONFIG                "Error creating default config for %s: %s"
#define ERR_MACHINE_NAME                    "GetComputerName error: code %d (%s)"
#define ERR_USER_NAME                       "GetUserName error: code %d (%s)"
#define ERR_USER_NAME_EX                    "GetUserNameEx error: code %d (%s)"
#define ERR_INIT_LOG                        "Could not create the LOG file."
#define ERR_INVALID_REG_PATH                "Invalid windows registry path: %s."
#define ERR_HKLM_KEYNOTFOUND                "Error reading HKLM registry key '%s'.\nPlease reinstall the application."

#define ERR_FOLDER_OPEN                     "Error opening Outlook folder for source '%ls' with path '%ls'."
#define ERR_FOLDER_PATH                     "'%ls' is not a folder selected for synchronization (see configuration of %ls source). Item received will be ignored."
#define ERR_FOLDER_DEFAULT_PATH             "Warning! No default folder found for %lss!"
#define ERR_ITEM_GET                        "Error getting item \"%ls\" for source '%ls'."
#define ERR_ITEM_GET_NEW                    "Error getting new item \"%ls\" for source '%ls'."
#define ERR_ITEM_GET_MOD                    "Error getting modified item \"%ls\" for source '%ls'."
#define ERR_ITEM_CREATE                     "Could not create a new %ls item into folder '%ls'"
#define ERR_ITEM_FILL                       "Error setting properties of %ls item \"%ls\". Item not saved."
#define ERR_INPUT_ITEM_FILTERED             "Incoming %ls item \"%ls\" discarded because it doesn't verify current filters. Item not saved."
#define ERR_INPUT_ITEM_DEL_FILTERED         "Incoming %ls item \"%ls\" not deleted because it doesn't verify current filters."
#define ERR_ITEM_BAD_TYPE                   "Mime type not supported: \"%ls\""
#define ERR_ITEM_SAVE                       "Could not save %ls item \"%ls\" into folder '%ls'"
#define ERR_ITEM_DELETE                     "Could not delete %ls item \"%ls\" from folder '%ls'"
#define ERR_ITEM_DELETE_INTERNALLY         L"Error removing internally the item."
#define ERR_ITEM_UPDATE_NOT_FOUND           "Could not update %ls item: ID = %ls not found."
#define ERR_ITEM_DELETE_NOT_FOUND           "Could not delete %ls item: ID = %ls not found."
#define ERR_EVENTS_CREATED                  "Error occurred while checking if Outlook created a birthday/anniversary event."
#define ERR_SOURCE_TOO_MANY_ERRORS          "%d errors occurred on source %ls: source will not be used any more."
#define ERR_SOURCE_LASTSYNCTIME_NOT_FOUND   "Error reading \"%ls\": <LastSyncTime> not found."
#define ERR_OCCURRENCE_NOT_FOUND            "Error creating the appointment exception: occurrence \"%ls\" not found."
#define ERR_OCCURRENCE_NOT_DELETED          "Could not delete one occurrence (date = %ls)."
#define ERR_OCCURRENCE_NOT_SAVED            "Could not save one occurrence (date = %ls)."
#define ERR_EXCEPTIONS_DEADLOCK             "Detected too many dependences between appointment exceptions (%d). Exceptions not saved correctly."
#define ERR_PROPERTY_REQUIRED               "Error creating %ls item: property \"%ls\" is required."

#define ERR_PARSE_SIF_TAG_NOT_FOUND         "Parsing error: SIF tag \"%ls\" not found."
#define ERR_PARSE_PROPERTY_NOT_FOUND        "Parsing error: property \"%ls\" not found."
#define ERR_PARSE_PROPERTY_EMPTY            "Parsing error: property \"%ls\" should not be empty."
#define ERR_PARSE_APP_EXCEPTION             "Error parsing appointment exception: %s"
#define ERR_PARSE_EXC_NOREC                 "Could not add exceptions on a not-recurring appointment."
#define ERR_NORMALIZE_EXCEPTIONS            "Error normalizing appointment exceptions: %s"

#define ERR_SCHED_INIT_TASK                 "Error initializing Windows task scheduler."
#define ERR_SCHED_SAVE                      "Scheduler error - failed to save the task, code = 0x%x - %s"
#define ERR_SCHED_DELETE                    "Scheduler error - failed to delete the task, code = 0x%x - %s"
#define ERR_SCHED_NEWWORKITEM               "Scheduler error - failed calling ITask::NewWorkItem, code = 0x%x - %s"
#define ERR_SCHED_CREATE_TRIGGER            "Scheduler error - failed calling ITask::CreateTrigger, code = 0x%x - %s"
#define ERR_SCHED_GET_TRIGGER               "Scheduler error - failed calling ITask::GetTrigger, code = 0x%x - %s"
#define ERR_SCHED_GET_TRIGGER2              "Scheduler error - failed calling ITaskTrigger::GetTrigger, code = 0x%x - %s"
#define ERR_SCHED_SET_TRIGGER               "Scheduler error - failed calling ITaskTrigger::SetTrigger, code = 0x%x - %s"
#define ERR_SCHED_QUERY_INTERFACE           "Scheduler error - failed calling ITask::QueryInterface, code = 0x%x - %s"
#define ERR_SCHED_ACTIVATE                  "Scheduler error - failed calling ITaskScheduler::Activate, code = 0x%x - %s"
#define ERR_SCHED_INVALID_PARAM             "One or more arguments are invalid - please select a correct interval for scheduler"
#define E_OBJECT_NOT_FOUND                  0x80070002L   // This should be mapped in winerror.h ??

// Info messages
#define INFO_ITEM_ADDED                     "Added %ls item: \"%ls\""
#define INFO_ITEM_UPDATED                   "Updated %ls item: \"%ls\""
#define INFO_ITEM_DELETED                   "Deleted %ls item: \"%ls\""
#define INFO_GET_ITEM                       "Retrieved %ls item: \"%ls\""
#define INFO_GET_NEW_ITEM                   "Retrieved new %ls item: \"%ls\""
#define INFO_GET_UPDATED_ITEM               "Retrieved updated %ls item: \"%ls\""
#define INFO_CONFIG_GENERATED               "Configuration not found: default configuration will be used."
#define INFO_SWV_UPGRADED                   "Configuration upgraded to new software version: v.%s (Funambol v.%s)"
#define INFO_CONFIG_DEVID_SAVED             "Set configuration deviceID = %s"
#define INFO_OLD_ITEMS_NOT_FOUND            "Cannot find list of items from previous sync. All items will be sent as modified."
#define INFO_SYNC_COMPLETED                 "Syncronization process completed."
#define INFO_SYNC_COMPLETED_ERRORS          "Syncronization process completed with errors (code = %d)."
#define INFO_EXIT                           "Exiting from Outlook client."
#define INFO_SYNC_ABORTING                  "Aborting synchronization session..."
#define INFO_SYNC_ABORTED_BY_USER           "Synchronization aborted by user."
#define INFO_SYNC_ABORTED_BY_USER_SLOW      "Synchronization aborted by user to avoid full-sync of %ls."
#define INFO_SCHED_TASK_CREATED             "Scheduler task created."
#define INFO_SCHED_TASK_DELETED             "Scheduler task deleted."
#define INFO_WRONG_MIME_TYPE                "Warning: mime type not recognized: \"%ls\""
#define INFO_REMOVING_ALL_ITEMS             "Removing all existing %lss (%d items to delete)..."
#define INFO_REMOVED_INTERNALLY            L"Removed internally before sync"

// Debug messages
#define DBG_OUTLOOK_OPEN                    "beginSync of %ls source: opening Outlook session..."
#define DBG_READ_ALL_ITEMS                  "Reading ALL %lss from folder '%ls' (%d found)..."
#define DBG_CONFIG_CLOSED                   "Outlook configuration closed."
#define DBG_STATE_ERR_ITEM_IGNORED          "Source state is ERROR -> item will be ignored."
#define DBG_LAST_SYNC_ABORTED               "Last synchronization has been aborted, mutex of sync process will be released now."
#define DBG_SYNC_ABORT_REQUEST              "User requested to abort synchronization process..."
#define DBG_THREAD_TERMINATED               "Synchronization thread has been terminated."
#define DBG_RRULE_BAD_FORMAT                "Error parsing RRULE, bad format: %ls"
#define DBG_SAFE_ITEM_NAME                  "Client Exception on getSafeName: %s"
#define DBG_ANNIVERSARY_DELETED             "While saving contact \"%ls\" an anniversary event was automatically created by Outlook and then just deleted!"
#define DBG_BIRTHDAY_DELETED                "While saving contact \"%ls\" a birthday event was automatically created by Outlook and then just deleted!"
#define DBG_SCHED_TASK_NOT_FOUND            "Scheduled task not found."
#define DBG_SCHED_LAST_EXECUTION            "The last task excecution was not performed. Please check windows task scheduler for more details"
#define DBG_SCHED_TASK_MANUALLY_CHANGED     "Scheduled task has been manually modified."
#define DBG_NORMALIZING_EXCEPTION           "Normalizing appointment exception: item \"%ls\", occurrence = %ls. A new item is created in Outlook."
#define DBG_ITEM_CREATED_FROM_EXCEPTION     "A new appointment \"%ls\" has been created from an event exception. Will be sent as NEW item next sync."
#define DBG_PARSING_SIF_DATA                "Parsing SIF %ls..."
#define DBG_PARSING_EXCEPTIONS              "Parsing appointment exceptions..."
#define DBG_PARSING_VOBJ_DATA               "Parsing VObject %ls (mime type = \"%ls\")..."


// Codes to update the status bar
#define SBAR_CHECK_ALL_ITEMS                1
#define SBAR_CHECK_MOD_ITEMS                2
#define SBAR_CHECK_MOD_ITEMS2               3
#define SBAR_SENDDATA_BEGIN                 5
#define SBAR_RECEIVE_DATA_BEGIN             7
#define SBAR_RECEIVE_DATA_END               8
#define SBAR_DELETE_CLIENT_ITEMS            9
#define SBAR_ENDING_SYNC                    10

/** @endcond */  // cond DEV

#include "defs.h"

// -------------------------------- Includes -------------------------------
#include <mstask.h>
#include "spds/SyncItem.h"
#include "vocl/WinItem.h"
#include "outlook/ClientItem.h"
#include "outlook/ClientAppointment.h"
#include "OutlookConfig.h"
#include "WindowsSyncSource.h"
#include "WindowsSyncClient.h"

#include <string>


//--------------------------- Public Functions ----------------------------

// Main functions:
int  initializeClient   (bool isScheduled, bool justRead = false);
int  initLog            (bool isScheduled);
int  startSync          ();
int  doSAPIRestoreCharge();
int  doSapiLoginThread  ();
int  doSapiLogin        ();

/**
 * Resets timestamps for all sources.
 * Note: this function saves the config.
 */
void resetAllSourcesTimestamps();


bool getRefreshSourcesListToSync ( void );
void setRefreshSourcesListToSync (bool b);

/**
 * Checks is MS Outlook and Redemption.dll are installed.
 * PIM sources will be shown/hidden accordingly.
 * If the Redemption lib needs to be installed and it's the free version,
 * a disclaimer popup will be shown to the user.
 * Also fixes the syncmodes in registry for PIM sources (bug#11240 fixed 
 * in v.10.0.3) if necessary.
 * If necessary, config will be saved.
 */
void initPIMSources();

/// Starts the sync of a single source.
/// Fires the syncsource_start and syncsource_end events to refresh the
/// UI, before and after the sync process.
int  synchronize        (WindowsSyncClient& winClient, WindowsSyncSource& source);
int  closeClient        ();
void closeOutlook       ();

/**
 * Checks if synchronization session has been intentionally aborted.
 * A flag 'abortSync' inside OutlookConfig singleton object is used to
 * indicate that the user wants to abort the sync.
 * The client periodically checks this flag, using this function.
 */
bool checkIsToAbort();

bool checkSyncInProgress();
void softTerminateSync  ();
int  hardTerminateSync  (HANDLE hSyncThread);
int  exitSyncThread     (int code);
void endSync();
HANDLE mutexAlloc();
void mutexRelease();
void upgradePlugin      (const int oldVersion, const int oldFunambolVersion);
void upgradeScheduledTask();


// Configuration:
/** @addtogroup config */
/** @{ */
__declspec(dllexport) OutlookConfig* getConfig();
void createDefaultConfig();
std::wstring pickOutlookFolder   (const std::wstring& itemType);
std::wstring getDefaultFolderPath(const std::wstring& itemType);
/** @} */


// dataTransformer functions:
/** @addtogroup dataTransformer */
/** @{ */
void         initWinItems       ();
WinItem*     createWinItem      (bool useSIF, const std::wstring itemType);
WinItem*     createWinItem      (bool useSIF, const std::wstring itemType, const std::wstring& data, const WCHAR** sifFields);
SyncItem*    convertToSyncItem  (ClientItem* cItem, const char* dataType, const std::wstring& defaultFolder, bool addUserProperties = true);
int          fillClientItem     (const std::wstring& sif, ClientItem* cItem, const std::wstring& itemType, const WCHAR* dataType);
WCHAR**      getProperSifArray  (const std::wstring& type);
int          normalizeExceptions(ClientItem* cItem, itemKeyList& allItems, itemKeyList& allItemsPaths);
int          deleteOccurrencesInInterval(const DATE startDate, const DATE originalDate, ClientRecurrence* cRec);
int          setRecurrenceExceptions(ClientItem * cItem, ClientRecurrence * cRec, std::list<std::wstring> &excludeDates, std::list<std::wstring> &includeDates);
int          checkIllegalXMLChars(char* data);
std::wstring getVPropertyValue  (const std::wstring& dataString, const std::wstring& propertyName);
void         replaceDefaultPath(std::wstring& path, const std::wstring& defaultFolder);
/** @} */


// Scheduler functions:
/** @addtogroup scheduler */
/** @{ */
int   setScheduler      (const bool enable, const int minutes);
bool  getScheduler      (int* minutes);
int   setScheduleTask   (const char* frequency, const int dayNum, const int minNum);
int   getScheduleTask   (bool* active, int* dayNum, int* minNum);
int   deleteScheduleTask();
ITaskScheduler* initScheduleInstance();
int   getScheduledTaskName(std::wstring& taskName);
void setProgramNameForScheduledTask(std::wstring name);
/** @} */

const char* getClientLastErrorMsg ();
const int   getClientLastErrorCode();

int OpenMessageBox(HWND hwnd, UINT type, UINT msg);
_declspec(dllexport) int checkUpdate(const char *infoURL, char *availableVersionData, char *updateURLData);

/**
* Starts the whole update procedure.
* @param hwnd the handle of the main window from the UI. It is set by the 
*             when the method is called by the UI form
* @param manual parameter indicates that the user is starting from UI
* It returns a value indicating the UI have to show the "Update Software"
* menu item to start the update procedure manually.
* ret 1,2 the UI must show the item
* ret -1,0 the UI hides the item
*/
int updateProcedure(HWND hwnd, bool manual = false);

/**
 * Returns true if a new version is known to be available for upgrade. This
 * method does not query the upgrade server, but it uses the information
 * available in the config.
 */
bool isNewSwVersionAvailable();

/**
* It is called to check if there is a new version available. If there is one,
* it sets a parameter into the reigistry.
* If necessary, will request via http the updated information.
*/
int checkUpdate();

bool checkForMandatoryUpdateBeforeStartingSync();

/**
* Added functions to get the ctcaps properties
*/
ArrayList* getVTodoProperties();
ArrayList* getVCalendarProperties();
ArrayList* getVCardProperties();
ArrayList* getNoteProperties();
ArrayList* getVNoteProperties();


/**
 * Returns the installed MS Outlook name and version, as a string.
 * Empty string in case of error.
 */
__declspec(dllexport) StringBuffer getOutlookVersion();

/// Filters SapiSyncManager errors at client level, in order to
/// get a return code for the UI (to display popups...)
int manageSapiError(const int code);

/** @} */
/** @endcond */
#endif
