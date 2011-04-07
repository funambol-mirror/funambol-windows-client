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

#include "base/fscapi.h"
#include "base/Log.h"
#include "base/util/utils.h"
#include "base/memTracker.h"
#include "http/HttpConnection.h"

#include "winmaincpp.h"
#include "WindowsSyncSource.h"
#include "WindowsSyncClient.h"
#include "client/FileSyncSource.h"
#include "utils.h"
#include "HwndFunctions.h"

#include "outlook/ClientApplication.h"
#include "outlook/ClientException.h"
#include "SyncException.h"

// Listeners
#include "event/SetListener.h"
#include "event/ManageListener.h"
#include "event/OutlookSyncListener.h"
#include "event/OutlookSyncSourceListener.h"
#include "event/OutlookSyncStatusListener.h"
#include "event/OutlookSyncItemListener.h"
#include "event/OutlookTransportListener.h"
#include "base/adapter/PlatformAdapter.h"
#include "http/Proxy.h"
#include "http/URL.h"
#include "http/TransportAgentFactory.h"
#include "http/WinTransportAgent.h"
#include "base/util/XMLProcessor.h"

#include "sapi/FileSapiSyncSource.h" //SAPI
//#include "sapi/MediaSapiSyncSource.h" //SAPI
#include "sapi/SapiSyncManager.h"
#include "spds/MappingsManager.h"

#include <string>

#include "UpdateManager.h"

using namespace std;


HANDLE syncMutex  = NULL;
HANDLE syncThread = NULL;

bool isScheduledSync = false;
bool resetLog        = false;
void launchSyncClient();

int synchronizeSapi(SapiSyncSource& sapiSource, SyncReport& report);

/**
 * Returns a pointer to the OutlookConfig object (singleton).
 * It's used to access the whole configuration object from UI, to get/set plugin properties.
 * If configuration never instantiated, we need to initialize client first.
 * (config could not exist, log must be opened)
 */
__declspec(dllexport) OutlookConfig* getConfig() {

    if (!OutlookConfig::isInstantiated()) {
        initializeClient(false);
    }
    return OutlookConfig::getInstance();
}




/**
 * Initialize the client: open configuration, open LOG.
 * Configuration is a singleton object that MUST remain active during all
 * program life. Will be released with close() method.
 *
 * @param isScheduled true if it's a scheduled sync
 * @return            0 if no errors
 */

int initializeClient(bool isScheduled, bool justRead) {

    // --- Uncomment for debug logging at startup ---
    //makeDataDirs();
    //StringBuffer logDir = getLogFileDir();
    //LOG.setLogPath(logDir.c_str());
    //LOG.setLogName(OL_PLUGIN_LOG_NAME);
    //LOG.setLevel(LOG_LEVEL_DEBUG);
    // ----------------------------------------------

    LOG.debug("entering %s", __FUNCTION__);
    int ret=0;
    char logText[512];
    logText[0] = 0;

    //
    // Initialize configuration.
    // -------------------------
    OutlookConfig* config = OutlookConfig::getInstance();
    //
    // 1. Generate configuration if error reading (may not exist if first time).
    //    Save, to ensure future calls to config.read().
    if (!config->read()) {
        LOG.debug("error reading user's config: generate default");
        config->createDefaultConfig();
        config->initializeVersionsAndUserAgent();
        config->save();
        sprintf(logText, INFO_CONFIG_GENERATED);
    }

    if (justRead) {
        return 0;
    }

    //
    // 2. Check if sw version has changed (client may have been upgraded).
    //    If config upgraded, save it to disk.
    if (config->checkToUpgrade()) {
        config->upgradeConfig();

        // Execute actions to upgrade the plugin from old version.
        upgradePlugin(config->getOldSwv(), config->getOldFunambolSwv());

        config->save();
        sprintf(logText, INFO_SWV_UPGRADED, config->getSwv(), config->getFunambolSwv().c_str());

        upgradeScheduledTask();
    }

    // Simply save to public var.
    isScheduledSync = isScheduled;


    //
    // Initialize LOG.
    // ---------------
    if (initLog(isScheduled)) {
        safeMessageBox(ERR_INIT_LOG);
        ret = 1;
    }

    HwndFunctions::initHwnd();
    if (strlen(logText) > 0) {
        LOG.info(logText);
    }

    // Checks is MS Outlook and Redemption.dll are installed.
    // PIM sources will be shown/hidden accordingly.
    initPIMSources();

    return ret;
}

void initPIMSources() {

    LOG.debug("entering %s", __FUNCTION__);
    bool isOutlookInstalled = false;
    bool isRedemptionInstalled = false;
    
    OutlookConfig* config = OutlookConfig::getInstance();
    StringBuffer redemptionPath(config->getWorkingDir());
    redemptionPath += "\\";
    redemptionPath += "Redemption.dll";
    
    char* olPath = config->readPropertyValue(OUTLOOK_EXE_REGKEY, "", HKEY_LOCAL_MACHINE);
    if (olPath && strlen(olPath)>0) {
        LOG.debug("Microsoft Outlook detected installed");
        isOutlookInstalled = true;
    }

    char* redPath = config->readPropertyValue(REDEMPTION_CLSID_REGKEY, "", HKEY_CLASSES_ROOT);
    if (redPath && strlen(redPath)>0) {
        LOG.debug("Redemption.dll detected already registered");
        isRedemptionInstalled = true;
    }

    //
    // Register Redemption.dll
    //
    if (isOutlookInstalled && !isRedemptionInstalled) {
        LOG.info("Registering Redemption.dll to the system");
        HRESULT hr = registerDLL(redemptionPath.c_str(), true);

        if (FAILED(hr)) {
            if (hr == E_ABORT) {
                LOG.error("Registration aborted: %s", redemptionPath.c_str());
            } else {
                LOG.error("Error registering DLL: %s (code 0x%x)", redemptionPath.c_str(), hr);
            }
            LOG.info("Redemption.dll not correctly registered: PIM sources will be disabled");
            isOutlookInstalled = false;
        }
    }

    //
    // Enable/disable PIM sources
    //
    bool configChanged = false;
    if (isOutlookInstalled) {
        LOG.debug("PIM sources are supported");
        configChanged |= config->safeAddSourceVisible(CONTACT_    , true);
        configChanged |= config->safeAddSourceVisible(APPOINTMENT_, true);
        configChanged |= config->safeAddSourceVisible(TASK_       , true);
        configChanged |= config->safeAddSourceVisible(NOTE_       , true);
    }
    else {
        LOG.debug("PIM sources are not supported");
        configChanged |= config->removeSourceVisible(CONTACT_);
        configChanged |= config->removeSourceVisible(APPOINTMENT_);
        configChanged |= config->removeSourceVisible(TASK_);
        configChanged |= config->removeSourceVisible(NOTE_);
    }
    if (configChanged) {
        LOG.debug("UI sources visible changed: saving config");
        config->sortSourceVisible();
        config->save();
    }


    // Sets the mappings folder for PIM sources.
    if (isOutlookInstalled) {
        PIMMappingStoreBuilder* pmsb = new PIMMappingStoreBuilder();
        MappingsManager::setBuilder(pmsb);
    }
}


/**
 * Initialize the LOG. Log File is placed under 'app data' directory for
 * current user, if directories not found they will be created.
 * Log file is reset and set level with this call.
 * @param reset  if scheduled, we don't reset the log
 * @return       0 if no errors
 */
int initLog(bool isScheduled) {
    LOG.debug("entering %s", __FUNCTION__);

    // Log path: get from config (under app data)
    OutlookConfig* config = OutlookConfig::getInstance();
    string logPath = config->getLogDir();
    logPath += "\\";
    logPath += OL_PLUGIN_LOG_NAME;

    // If first time, dirs don't exist under app data!
    if (writeToFile("\n", logPath, "a")) {
        if (makeDataDirs()) {               // <- warning: this assume we place the log under 'app data'
            safeMessageBox(getLastErrorMsg());
            return 1;
        }
    }

    LOG.setLogPath(config->getLogDir());
    LOG.setLogName(OL_PLUGIN_LOG_NAME);
    LOG.setLevel(config->getClientConfig().getLogLevel());

    string title = "Windows Sync Client opened";
    if (!isScheduled) {
        resetLog = true;    // Will reset when first sync starts
    }
    else {
        resetLog = false;   // Don't reset log on sched sync
        title += " *** Scheduled sync started ***";
    }
    LOG.info(title.c_str());

    return 0;
}




/**
 ***************************************************
 * Entry point to start the synchronization process.
 ***************************************************
 *
 * @return   0  OK, no errors.
 *           1  generic error.
 *           2  aborted by user (soft termination).
 *           3  Outlook fatal exception.
 *           4  Thread terminated (hard termination).
 *           5  aborted by user to avoid full-sync.
 */
int startSync() {

    // check updates to see if the client has to exit immediately
    if (checkForMandatoryUpdateBeforeStartingSync()) {
        return 0;
    }

    int ret           = 0;
    int sourcesActive = 0;
    int priority      = 0;
    WCHAR* wname      = NULL;
    string mutexName  = "";

    // Set the cache dir for SAPI sources
    StringBuffer sapiCacheDir = getSapiCacheDir();

    // The main sync report, to collect reports of all sources synced.
    SyncReport report;

    // Open current configuration: call initialize(0) if not called yet!
    // (reset abortSync flag)
    OutlookConfig* config = getConfig();
    config->setAbortSync(false);

    // Check if log size is too big (>10MB).
    /*
    if (!resetLog && (LOG.getLogSize() > MAX_LOG_SIZE)) {
        resetLog = true;
    }

    // Reset log (only the first sync)
    if (resetLog) {
        string title = PROGRAM_NAME;
        title += " v. ";
        title += config->getClientConfig().getSwv();
        title += " - LOG file";
        if (isScheduledSync) title += " (scheduled sync)";
        LOG.reset(title.c_str());
        resetLog = false;
    }
    */
    // BEGIN LOG ROTATE
    if (!LOG.rotateLogFile(
        config->getWindowsDeviceConfig().getLogSize(),
        config->getWindowsDeviceConfig().getLogNum()
        )) {
        // TODO
        /*
        WCHAR tmp[512];
        wsprintf(tmp, L"Unable to rotate log file: \"%s\".\nPlease check your user's permissions.", LOG.getLogPath()));
        MessageBox(NULL, tmp, WPROGRAM_NAME, MB_SETFOREGROUND | MB_OK);
        */
    }


    // Update log level: could be changed from initialize().
    LOG.setLevel(config->getClientConfig().getLogLevel());
    LOG.debug("Starting the Sync process...");

    if (isScheduledSync) {
        LOG.info(" *** Scheduled sync started ***");
    }

    // If here, this is the ONLY instance of sync process
    // -> set the scheduled flag on win registry.
    config->setScheduledSync(isScheduledSync);


    // Reads timeStamps from registry -> update the config.
    // (a scheduled sync could have completed with the UI open)
    config->readSourcesTimestamps();

    //
    // Set a low priority for this thread: different if normal/scheduled sync.
    //
    LOG.debug("Set a lower priority to the process");
    syncThread = GetCurrentThread();
    if (isScheduledSync)  priority = THREAD_PRIORITY_LOWEST;
    else                  priority = THREAD_PRIORITY_BELOW_NORMAL;
    if(!SetThreadPriority(syncThread, priority)) {
        DWORD code = GetLastError();
        char* msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_THREAD_PRIORITY, code, msg);
        delete [] msg;
        LOG.error(getLastErrorMsg());
        return WIN_ERR_GENERIC;
    }

    //
    // Set listeners
    //
    LOG.debug("Set listeners");
    OutlookSyncListener*       listener1 = new OutlookSyncListener      ();
    OutlookSyncSourceListener* listener2 = new OutlookSyncSourceListener();
    OutlookSyncStatusListener* listener3 = new OutlookSyncStatusListener();
    OutlookSyncItemListener*   listener4 = new OutlookSyncItemListener  ();
    OutlookTransportListener*  listener5 = new OutlookTransportListener ();

    setSyncListener      (listener1);
    setSyncSourceListener(listener2);
    setSyncStatusListener(listener3);
    setSyncItemListener  (listener4);
    setTransportListener (listener5);


    //
    // Create the array of SyncSource names (only if source enabled)
    // -------------------------------------------------------------
    LOG.debug("Creating SyncSources...");
    int sourcesCount = config->getSyncSourceConfigsCount();
    ArrayList sources;  // source names
    int j=0;
    bool syncingPIM = false;
    config->sortSourceVisible();
    const ArrayList& sourcesOrder = config->getSourcesVisible();
    for (int j=0; j<sourcesOrder.size(); j++) {
        for (int i=0; i<sourcesCount; i++) {
            const bool enabled = config->getSyncSourceConfig(i)->isEnabled();
            const bool isRefresh = config->getSyncSourceConfig(i)->getIsRefreshMode();
            if (enabled || isRefresh) {
                StringBuffer* name = (StringBuffer*)sourcesOrder.get(j);
                if (*name == config->getSyncSourceConfig(i)->getName()) {
                    // Here the right SyncSource is added to the array of sources to sync.
                    sources.add(*name);
                    if (isPIMSource(name->c_str())) {
                        syncingPIM = true;
                    }
                }
            }
        }
    }

    // Exit if no sources to sync
    if (sources.size() == 0) {
        //safeMessageBox(MSGBOX_NO_SOURCES_TO_SYNC);
        setError(WIN_ERR_NO_SOURCES, ERR_NO_SOURCES_TO_SYNC);
        ret = WIN_ERR_NO_SOURCES;
        goto finally;
    }

    for (int i=0; i< sources.size(); i++) {
        StringBuffer* name = (StringBuffer*)sources.get(i);
        if (isMediaSource(name->c_str())) {
            // If media hub not set remove from the source to sync if in background
            if (isScheduledSync && !isMediaHubFolderSet()) {
                sources.removeElementAt(i);
                i--;
                continue;
            }
            if (!isMediaHubFolderSet()) {
                HWND dd = HwndFunctions::getWindowHandle();
                ::SendMessage(dd, ID_MYMSG_CHECK_MEDIA_HUB_FOLDER, 0, 0);
                goto finally;
            }
        }
    }

    if (syncingPIM) {
        //
        // If not syncing PIM sources, Outlook is not even accessed.
        //
        try {
            ClientApplication * outlook = ClientApplication::getInstance(isScheduledSync);
        }
        catch (ClientException* e) {
            // Must set the errors, here could be a fatal exception
            setErrorF(0, e->getErrorMsg());
            bool display =
                !isScheduledSync && getConfig()->getWindowsDeviceConfig().getAttach()
                ||
                !getConfig()->getWindowsDeviceConfig().getAttach();
            if (display)
            {
                e->setExceptionData(e->getErrorMsg(), e->getErrorCode(), false, true);
                manageClientException(e);
                return 1;
            }
            else
            {
                return 2;
            }
        }
    }


    //
    // Create the mutex for sync process.
    // **********************************
    //
    // Refresh the 'beginSync' timestamp now, and save (only this value) to winreg.
    char buf[21];
    unsigned long timestamp = (unsigned long)time(NULL);
    timestampToAnchor(timestamp, buf);
    config->getAccessConfig().setBeginSync(timestamp);
    config->saveBeginSync();

    // - Use always a different mutex name, to avoid errors on pending mutexes (if sync drastically aborted).
    // - We need to know the mutex name from different plugin instances, so use the 'BeginSync' value
    //   that is re-written each time a sync process starts (write it here).
    mutexName = getSyncMutexName();
    LOG.debug("Creating the sync-mutex (\"%s\")", mutexName.c_str());
    syncMutex = CreateMutexA(NULL, TRUE, mutexName.c_str());
    if(!syncMutex){
        char* msg = readSystemErrorMsg();
        setErrorF(getLastErrorCode(), ERR_MUTEX_CREATE, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        ret = 1;
        goto finally;
    }
    if(GetLastError() == ERROR_ALREADY_EXISTS) {
        char* msg = readSystemErrorMsg();
        setErrorF(getLastErrorCode(), ERR_MUTEX_ALREADY_EXISTS, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        ret = 1;
        goto finally;
    }


    // --------------------------------------------------
    // Kick off the sync: one source at time
    for (int i=0; i< sources.size(); i++) {
        int res = 0;    // it's the sync result for this source
        StringBuffer* name = (StringBuffer*)sources.get(i);
        if (!name) continue;

        SyncSourceReport ssReport(name->c_str());
        WindowsSyncSourceConfig* wssconfig = config->getSyncSourceConfig(name->c_str());
        SyncSourceConfig* ssconfig = wssconfig->getCommonConfig();

        if (isMediaSource(name->c_str()))
        {
            // --- Media source ---
            SapiSyncSource* source;
            //if (strcmp(name->c_str(), PICTURE_) == 0 || strcmp(name->c_str(), VIDEO_) == 0) {
            //    source = new MediaSapiSyncSource(*ssconfig, ssReport, 0, 0, sapiCacheDir.c_str());    // filterDates are both disabled            
            //} else {
                source = new FileSapiSyncSource(*ssconfig, ssReport, 0, 0, sapiCacheDir.c_str());    // filterDates are both disabled                            
            //}
            res = synchronizeSapi(*source, report);
            report.addSyncSourceReport(source->getReport());
            delete source;
        }
        else
        {   // --- PIM source ---
            WString wname;
            wname = *name;
            WindowsSyncSource source(wname.c_str(), wssconfig);
            WindowsSyncClient winClient(source);
            res = synchronize(winClient, source);

            report.addSyncSourceReport(*source.getReport());
        }

        if (res) {
            // Update the sync report (global error for all syncs!)
            SyncSourceReport* ssr = report.getSyncSourceReport(name->c_str());
            report.setLastErrorCode(res);
            report.setLastErrorMsg (ssr->getLastErrorMsg());
            report.setLastErrorType(ERROR_TYPE_WINDOWS_CLIENT);
            LOG.error("Sync of %s completed with error %s%d: %s", name->c_str(), ssr->getLastErrorType(),
                                                        ssr->getLastErrorCode(), ssr->getLastErrorMsg());
        } else {
            LOG.info("Sync of %s completed successfully", name->c_str());
        }

        // for these codes, we stop all the queued syncs
        if (res == WIN_ERR_INVALID_CREDENTIALS ||
            res == WIN_ERR_PROXY_AUTH_REQUIRED ||
            res == WIN_ERR_WRONG_HOST_NAME) {
            break;
        }
    }
    // --------------------------------------------------

finally:

    // This is the global error, for all sources.
    // It is used to show popups in case of error!
    ret = report.getLastErrorCode();

    endSync();

    // Sync was canceled: fix the return code to avoid popups
    if (ret && config->getAbortSync()) {
        ret = WIN_ERR_SYNC_CANCELED;
        report.setLastErrorCode(ret);
        report.setLastErrorMsg("Sync canceled by the user");
    }

    // Don't want to bother the user with a popup for these errors
    // if it's an automatic sync
    if (isScheduledSync) {
        if ( ret == WIN_ERR_SERVER_QUOTA_EXCEEDED ||
             ret == WIN_ERR_LOCAL_STORAGE_FULL ) {
            ret = 0;
        }
    }

    // set the last global error
    config->setLastGlobalError(ret);
    config->save();

    // Print sync session results
    StringBuffer reportMsg;
    report.toString(reportMsg);
    LOG.info("\n%s", reportMsg.c_str());

    // check for updates (skip in case of network error / sync aborted)
    if (ret != WIN_ERR_SYNC_CANCELED  &&
        ret != WIN_ERR_NETWORK_ERROR  &&
        ret != WIN_ERR_WRONG_HOST_NAME) {
        if (checkUpdate()) {
            updateProcedure(HwndFunctions::getWindowHandle(), false);
        }
    }


#ifdef MALLOC_DEBUG
    printMemLeaks();
#endif
    return ret;
}


int synchronizeSapi(SapiSyncSource& sapiSource, SyncReport& report) {

    int ret = 0;
    SyncMode syncMode = syncModeCode(sapiSource.getConfig().getSync());
    OutlookConfig* config = getConfig();

    StringBuffer name = sapiSource.getConfig().getName();
    int sourceID = syncSourceNameToIndex(name);
    if (!sourceID) {
        return WIN_ERR_GENERIC;
    }

    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNCSOURCE_BEGIN, NULL, (LPARAM)sourceID);

    // ---------------------------------------------
    SapiSyncManager sapiManager(sapiSource, *config);
    ret = sapiManager.beginSync();
    config->saveSyncSourceConfig(name.c_str());
    if (ret != 0) {
        goto finally;
    }

    // UPLOAD
    if (syncMode == SYNC_TWO_WAY ||
        syncMode == SYNC_ONE_WAY_FROM_CLIENT) {
        if (report.getLastErrorCode() != WIN_ERR_SERVER_QUOTA_EXCEEDED) {
            ret = sapiManager.upload();
            config->saveSyncSourceConfig(name.c_str());
        } else {
            sapiSource.getReport().setState(SOURCE_ERROR);
            sapiSource.getReport().setLastErrorCode(ESSMServerQuotaExceeded);
            sapiSource.getReport().setLastErrorType(ERROR_TYPE_SAPI_SYNC_MANAGER);
            sapiSource.getReport().setLastErrorMsg("Upload skipped: quota exceeded for a previous source");
            LOG.info("%s", sapiSource.getReport().getLastErrorMsg());
        }
    }
    if (ret == ESSMCanceled ||
        ret == ESSMNetworkError ||
        ret == ESSMAuthenticationError) {
        goto finally;
    }

    // DOWNLOAD
    if (syncMode == SYNC_TWO_WAY ||
        syncMode == SYNC_ONE_WAY_FROM_SERVER) {
        if (report.getLastErrorCode() != WIN_ERR_LOCAL_STORAGE_FULL) {
            ret = sapiManager.download();
            config->saveSyncSourceConfig(name.c_str());
        } else {
            sapiSource.getReport().setState(SOURCE_ERROR);
            sapiSource.getReport().setLastErrorType(ERROR_TYPE_SAPI_SYNC_MANAGER);
            sapiSource.getReport().setLastErrorCode(ESSMClientQuotaExceeded);
            sapiSource.getReport().setLastErrorMsg("Download skipped: local storage full for a previous source");
            LOG.info("%s", sapiSource.getReport().getLastErrorMsg());
        }
    }
    if (ret == ESSMCanceled ||
        ret == ESSMNetworkError ||
        ret == ESSMAuthenticationError) {
        goto finally;
    }

    ret = sapiManager.endSync();
    // ---------------------------------------------

finally:

    if (ret) {
        // filter SapiSyncManager errors at client level (for UI popups)
        ret = manageSapiError(ret);
    }

    // set the last error for this source
    sapiSource.getConfig().setLastSourceError(ret);

    // SAVE CONFIG TO DISK.
    // Saves also the last sync time (now), used to refresh UI
    config->saveSyncSourceConfig(name.c_str());


    // enable/disable dynamic sources (check Server datastores)
    // The source name and the preferred data type must match.
    bool removedPictures = false;
    if (DYNAMICALLY_SHOW_PICTURES) {
        DataStore* dataStore = config->getServerDataStore(PICTURE_);
        if ( dataStore && !strcmp(dataStore->getRxPref()->getCTType(), OMA_MIME_TYPE) ) {
            config->safeAddSourceVisible(PICTURE_);
        }
        else {
            removedPictures = config->removeSourceVisible(PICTURE_);
        }
    }

    // Fire the SOURCE_STATE message to the UI, to tell the state of sources synced
    LPARAM sourceState = ret;
    if (config->getAbortSync()) {
        sourceState = WIN_ERR_SYNC_CANCELED;
    }
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNCSOURCE_END, (WPARAM)sourceID, sourceState);

    return ret;
}


int synchronize(WindowsSyncClient& winClient, WindowsSyncSource& source) {

    SyncReport* report = winClient.getSyncReport();
    if (!report) return -1;

    OutlookConfig* config = getConfig();
    StringBuffer name = source.getConfig().getName();
    int sourceID = syncSourceNameToIndex(name);
    if (!sourceID) {
        return -1;
    }

    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNCSOURCE_BEGIN, NULL, (LPARAM)sourceID);

    // ----------------------------------------------
    SyncSource* oneSourceArray[2];
    oneSourceArray[0] = &source;
    oneSourceArray[1] = NULL;
    int ret = winClient.sync(*config, oneSourceArray);
    // ----------------------------------------------


    // enable/disable pictures source (check Server datastores)
    // The source name and the preferred data type must match.
    bool removedPictures = false;
    if (DYNAMICALLY_SHOW_PICTURES) {
        DataStore* dataStore = config->getServerDataStore(PICTURE_);
        if ( dataStore && !strcmp(dataStore->getRxPref()->getCTType(), OMA_MIME_TYPE) ) {
            config->safeAddSourceVisible(PICTURE_);
        }
        else {
            removedPictures = config->removeSourceVisible(PICTURE_);
        }
    }

    //
    // Save configuration to win registry. (TBD: manage dirty flag!)
    // Note: source configs will not be saved if not successful...
    // Note: we MUST lock the buttons during the save(), to avoid users to cancel sync.
    //
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_REFRESH_STATUSBAR, NULL, (LPARAM)SBAR_ENDING_SYNC);
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_LOCK_BUTTONS,      NULL, NULL);
    LOG.debug("Saving configuration to winRegistry");
    config->save(report);


    // Fire the SOURCE_STATE message to the UI, to tell the state of sources synced
    LPARAM sourceState = ret;
    SyncSourceReport* ssReport = source.getReport();
    if (ssReport) {
        if ((ssReport->getState() != SOURCE_ERROR) && source.getConfig().getIsSynced()) {
            sourceState = WIN_ERR_NONE;
        }
    }
    if (config->getAbortSync()) {
        sourceState = WIN_ERR_SYNC_CANCELED;
    }

    // Finally: unlock buttons
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_SYNCSOURCE_END, (WPARAM)sourceID, sourceState);
    SendMessage(HwndFunctions::getWindowHandle(), ID_MYMSG_UNLOCK_BUTTONS, NULL, NULL);

    return ret;
}



/**
 * Common operations to end the sync process.
 * - Close Outlook instance
 * - Release mutex on sync
 * - Unset Listeners
 */
void endSync() {

    // Close Outlook instance
    closeOutlook();

    // Release mutex on sync (use the same handle of startSync!)
    if (syncMutex) {
        LOG.debug("Releasing sync-mutex...");
        if (!ReleaseMutex(syncMutex)) {
            char* msg = readSystemErrorMsg();
            setErrorF(getLastErrorCode(), ERR_MUTEX_NOT_RELEASED, msg);
            LOG.error(getLastErrorMsg());
            delete [] msg;
        }
        LOG.debug("Sync-mutex released - closing handle.");
        CloseHandle(syncMutex);
        syncMutex = NULL;
    }

    // Unset Listeners
    ManageListener::releaseAllListeners();
}




/**
 * Closing operation before exiting client DLL.
 */
int closeClient() {

    // Try closing Outlook session if not yet done.
    closeOutlook();

    // Delete OutlookConfig instance.
    if (OutlookConfig::isInstantiated()) {
        OutlookConfig* config = OutlookConfig::getInstance();
        if (config) {
            LOG.debug("Deleting OutlookConfig instance");
            delete config;
        }
        LOG.debug(DBG_CONFIG_CLOSED);
    }

    LOG.info(INFO_EXIT);
    return 0;
}




/**
 * If Outlook session is active, close it and clean-up shared objects.
 */
void closeOutlook() {

    if ( ClientApplication::isInstantiated() ) {

        LOG.debug("Closing Outlook...");
        ClientApplication* outlook;
        try {
            outlook = ClientApplication::getInstance();
            if (outlook) {
                LOG.debug("Deleting ClientApplication instance");
                delete outlook;
                outlook = NULL;
            }
        }
        catch (ClientException* e) {
            manageClientException(e);
            LOG.error(ERR_CLOSE_OUTLOOK);
        }
        // *** To catch unexpected exceptions... ***
        catch (...) {
            LOG.error(ERR_CLOSE_OUTLOOK);
        }
    }
}



/**
 * Check if a synchronization process is already running.
 * This is obtained checking the correspondent mutex, which is created
 * during the sync process.
 * @return  true if a sync is already running.
 */
bool checkSyncInProgress() {

    // ALWAYS get exact name from 'BeginSync' property.
    string mutexName = getSyncMutexName();

    //
    // Try opening the mutex:
    //
    HANDLE hMutex = syncMutex;
    if (!hMutex) {
        hMutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, mutexName.c_str());
    }

    // OpenMutex failed.
    if (hMutex == NULL) {
        if (GetLastError() == ERROR_FILE_NOT_FOUND) {
            // *** mutex not found *** -> no sync in progress.
            return false;
        }
        // Some other error
        char* msg = readSystemErrorMsg();
        setErrorF(getLastErrorCode(), ERR_MUTEX_OPEN, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        return false;
    }

    // *** mutex found *** -> sync in progress OR pending mutex?
    else {

        //
        // Try getting access to the mutex found:
        //
        DWORD dwWaitResult = WaitForSingleObject(hMutex, 0L);

        switch (dwWaitResult) {

            // This thread has obtained access: mutex free or previous thread was killed
            // -> release the mutex, no sync in progress.
            case WAIT_ABANDONED:
                LOG.debug(DBG_LAST_SYNC_ABORTED);
            case WAIT_OBJECT_0: {

                // ***TODO*** This has been commented cause not sure if correct to
                //            release a pending mutex (owner could try to release it)
                //if (!ReleaseMutex(hMutex)) {
                //    char* msg = readSystemErrorMsg();
                //    setErrorF(getLastErrorCode(), ERR_MUTEX_NOT_RELEASED, msg);
                //    LOG.error(getLastErrorMsg());
                //    delete [] msg;
                //}
                // ***TODO*** verify this: probably there's no need to close handle here
                //            (can lead to 2 close handle, if owner closes it just after this)
                //CloseHandle(hMutex);

                ReleaseMutex(hMutex);
                return false;
            }

            // Cannot get mutex ownership due to time-out (sync already in progress)
            case WAIT_TIMEOUT: {
                return true;
            }

            // Some error occurred (case WAIT_FAILED)
            default: {
                char* msg = readSystemErrorMsg();
                setError(getLastErrorCode(), msg);
                LOG.error(getLastErrorMsg());
                delete [] msg;
                return true;
            }
        }
    }
}



/**
 * This function is called to terminate the sync session (soft way).
 * We first try to "soft" terminate the thread, setting the correspondent
 * flag inside OutlookConfig. This is to correctly close the session.
 */
void softTerminateSync() {

    OutlookConfig* config = getConfig();
    config->setAbortSync(true);

    // try to break internet connection, closing interfaces
    // this will cause a transaction error, unblocking it, if connection is in use.
    HttpConnection::closeConnection();
    WinTransportAgent::closeConnection();

    LOG.debug(DBG_SYNC_ABORT_REQUEST);
}




/**
 * This function is called to terminate the sync session (hard way).
 * If softTerminateSync() did not work, use this function to terminate
 * the thread of synchronization.
 *
 * @param hSyncThread   the handle of sync thread to terminate
 * @return              0 if sync aborted successfully.
 *                      1 if could not terminate the sync thread.
 */
int hardTerminateSync(HANDLE hSyncThread) {

    // (code 4 = thread terminated)
    if (!TerminateThread(hSyncThread, WIN_ERR_THREAD_TERMINATED)) {
        char* msg = readSystemErrorMsg();
        setErrorF(WIN_ERR_THREAD_TERMINATED, ERR_THREAD_NOT_TERMINATED, msg);
        LOG.error(getLastErrorMsg());
        delete [] msg;
        return 1;
    }

    setErrorF(WIN_ERR_SYNC_CANCELED, INFO_SYNC_ABORTED_BY_USER);
    LOG.info(getLastErrorMsg());
    LOG.debug(DBG_THREAD_TERMINATED);
    return 0;
}



/**
 * This function is called to terminate the sync thread.
 * Like 'hardTerminateSync' but used internally by client
 * to terminate the syncThread (HANDLE set during startSync).
 *
 * @param code   the exit code for the sync thread
 * @return       0 if sync thread terminated successfully.
 *               1 if could not terminate the sync thread.
 */
int exitSyncThread(int code) {

    if (syncThread) {
        if (!TerminateThread(syncThread, code)) {
            char* msg = readSystemErrorMsg();
            setErrorF(getLastErrorCode(), ERR_THREAD_NOT_TERMINATED, msg);
            LOG.error(getLastErrorMsg());
            delete [] msg;
            return 1;
        }
        return 0;
    }
    return 1;
}



/**
 * --- DEPRECATED: cohoperative stop is implemented ---
 * Check if synchronization session has been intentionally aborted.
 * A flag 'abortSync' inside OutlookConfig singleton object is used to
 * indicate that the user wants to abort the sync.
 * The client periodically checks this flag, using this function.
 * @note  this is important to correctly close Outlook session, as the logoff must
 *        be done by the same thread that logged in... otherwise Outlook may become
 *        instable or could not respond.
 */
void checkAbortedSync() {

    OutlookConfig* config = getConfig();

    if (config->getAbortSync()) {
        LOG.info(INFO_SYNC_ABORTING);

        endSync();

        // Throw SyncException with code 2 (sync aborted by user)
        setErrorF(getLastErrorCode(), INFO_SYNC_ABORTED_BY_USER);
        LOG.info(getLastErrorMsg());
        config->setAbortSync(false);
        throwSyncException(getLastErrorMsg(), WIN_ERR_SYNC_CANCELED);
    }
}




/**
 * Returns the full Outlook path of the default folder, given the desired 'itemType'.
 *
 * @note  to correctly manage COM library, we delete the ClientApplication instance
 *        at the end of this method (it's called from UI config window, Details).
 *
 * @param itemType   the type of items for the folder to select (CONTACT/TASK/...)
 * @return           path of default folder (empty string if errors).
 */
wstring getDefaultFolderPath(const wstring& itemType) {

    wstring pathSelected = EMPTY_WSTRING;
    ClientApplication* outlook = NULL;
    ClientFolder*      folder  = NULL;

    try {
        outlook = ClientApplication::getInstance(false);
        if (outlook) {
            folder = outlook->getDefaultFolder(itemType);
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        pathSelected = EMPTY_WSTRING;
        goto finally;
    }

    if (folder) {
        pathSelected = folder->getPath();
    }

finally:
    // Delete ClientApplication: this is called from UI config, so we should
    // release the COM library to be correctly used by next thread.
    try {
        if (outlook) {
            delete outlook;
            outlook = NULL;
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
    }
    return pathSelected;
}



/**
 * This function displays an Outlook window on desktop, to browse for a
 * specific Outlook folder. If passed 'itemType' is not an empty string,
 * the user will have to select a folder of the correct type - otherwise
 * a warning+retry will be displayed.
 * The full path of folder will be returned.
 *
 * @note  to correctly manage COM library, we delete the ClientApplication instance
 *        at the end of this method (it's called from UI config window, Details).
 *
 * @param itemType   the type of items for the folder to select (CONTACT/TASK/...)
 * @return           path of folder selected (empty string if not selected).
 */
wstring pickOutlookFolder(const wstring& itemType) {

    wstring pathSelected = EMPTY_WSTRING;
    ClientApplication* outlook = NULL;
    ClientFolder*      folder  = NULL;


    // Pick the desired folder.
    try {
        outlook = ClientApplication::getInstance();
        if (outlook) {
            folder = outlook->pickFolder(itemType);
        }
    }
    catch (ClientException* e) {
        manageClientException(e);
        pathSelected = EMPTY_WSTRING;
        goto finally;
    }

    // Get folder's path.
    if (folder) {
        pathSelected = folder->getPath();
    }

finally:
    // Delete ClientApplication: this is called from UI config, so we should
    // release the COM library to be correctly used by next thread.
    try {
        closeOutlook();
    }
    catch (ClientException* e) {
        manageClientException(e);
    }

    return pathSelected;
}



/**
 * Set the scheduled task of plugin.
 * @param enable    true  = activate the scheduler
 *                  false = disable the scheduler (delete the task)
 * @param minutes   the repeating minutes of task
 * @return          0 if no errors
 */
int setScheduler(const bool enable, const int minutes) {

    //
    // Activate Windows scheduler: create or update the task.
    //
    if (enable) {

        int dayNum = SCHED_DURATION_DAYS;           // Fixed: task duration = 1 day.
        int minNum = SCHED_DEFAULT_REPEAT_MINS;     // Default every 15 min.
        if (minutes > 0) {
            minNum = minutes;
        }

        return setScheduleTask(EVERY_DAY, dayNum, minNum);
    }

    //
    // Delete the task (if any).
    //
    else {
        return deleteScheduleTask();
    }
}


/**
 * Get information about scheduled task of plugin.
 * @param minutes   [OUT] the repeating minutes of task
 * @return          true if task is active.
 *                  false if task is disabled or not existing.
 */
bool getScheduler(int* minutes) {

    bool active = false;
    int dayNum  = 0;
    int minNum  = 0;

    int ret = getScheduleTask(&active, &dayNum, &minNum);

    if (ret < 0) {
        // Not found OR errors
        return false;
    }

    if ((dayNum != SCHED_DURATION_DAYS) || (ret == 2)) {
        LOG.debug(DBG_SCHED_TASK_MANUALLY_CHANGED);
    }

    if (active == false) {
        // Task is disabled
        return false;
    }

    // If here, task found and active.
    *minutes = minNum;
    return true;
}



const char* getClientLastErrorMsg() {
    return getLastErrorMsg();
}

const int getClientLastErrorCode() {
    return getLastErrorCode();
}



/**
 * Operations to upgrade the plugin from 'oldVersion' to this version.
 * Should be called only once during plugin first start.
 */
void upgradePlugin(const int oldVersion, const int oldFunambolVersion) {

    LOG.debug("Upgrade client from version %d", oldFunambolVersion);

    // Upgrades from a version < v8 are no more supported
    if (oldFunambolVersion < 80000) {
        return;
    }

    if (oldFunambolVersion < 100000) {

        // Old version < 10.0.0: Client name has changed, was "Outlook Client"
        // move the cache files under %APPDATA% folder
        // from "OutlookClient" to "WindowsClient"
        makeDataDirs();

        // Get 'application data' folder for current user.
        WCHAR p[MAX_PATH_LENGTH];
        SHGetSpecialFolderPath(NULL, p, CSIDL_APPDATA, 0); 

        wstring oldDataPath(p);
        oldDataPath += TEXT("\\");
        oldDataPath += FUNAMBOL_DIR_NAME;
        oldDataPath += TEXT("\\");
        oldDataPath += TEXT("OutlookClient");

        WCHAR* dataPath = readAppDataPath();
        if (!dataPath) {
            return;
        }
        wstring newDataPath(dataPath);
        delete [] dataPath;

        // List of possible cache files to copy
        list<wstring> fileNames;
        fileNames.clear();
        fileNames.push_back(TEXT("\\appointment.db"));
        fileNames.push_back(TEXT("\\appointment_modified.db"));
        fileNames.push_back(TEXT("\\contact.db"));
        fileNames.push_back(TEXT("\\note.db"));
        fileNames.push_back(TEXT("\\task.db"));
        // (pictures cache is not preserved before v10)

        // Copy ALL cache files (*.db) to new location.
        wstring oldName, newName;
        list<wstring>::iterator it;
        for (it = fileNames.begin(); it != fileNames.end(); it++) {
            oldName = oldDataPath;  oldName += *it;
            newName = newDataPath;  newName += *it;
            CopyFile(oldName.c_str(), newName.c_str(), FALSE);
        }

        // Now we can remove the old cache dir with all its content.
        wstring oldPictureCache = oldDataPath;
        oldPictureCache += TEXT("\\item_cache");
        char* oldDir = toMultibyte(oldPictureCache.c_str());
        removeFileInDir(oldDir);
        RemoveDirectory(oldPictureCache.c_str());
        delete [] oldDir;

        oldDir = toMultibyte(oldDataPath.c_str());
        removeFileInDir(oldDir);
        RemoveDirectory(oldDataPath.c_str());
        delete [] oldDir;

        // Rename the scheduled task for this user only (we don't have more permissions)
        // was: "C:\Windows\Tasks\Funambol Outlook Client - <username>.job"
        wstring user;
        getWindowsUser(user);
        WCHAR winDir[MAX_PATH_LENGTH];
        GetWindowsDirectory(winDir, MAX_PATH_LENGTH);

        wstring oldTaskName(winDir);
        oldTaskName += TEXT("\\Tasks\\Funambol Outlook Sync Client - ");
        oldTaskName += user;
        oldTaskName += TEXT(".job");

        wstring name;
        getScheduledTaskName(name);     // Using this function, so it's always the correct name
        wstring newTaskName(winDir);
        newTaskName += TEXT("\\Tasks\\");
        newTaskName += name;
        newTaskName += TEXT(".job");

        MoveFile(oldTaskName.c_str(), newTaskName.c_str());
    }

    return;
}


void upgradeScheduledTask() {
    LOG.debug("entering %s", __FUNCTION__);

    //upgrade scheduled task
    setProgramNameForScheduledTask(WPROGRAM_NAME);
    bool active;
    int dayNum;
    int minNum;

    int res = getScheduleTask(&active, &dayNum, &minNum);
    if (res >= 0) {
        deleteScheduleTask();
        setProgramNameForScheduledTask(WPROGRAM_NAME);
        setScheduleTask(EVERY_DAY, dayNum, minNum);
    }
}

//int OpenMessageBox(HWND hwnd, UINT buttons, UINT msg){
int OpenMessageBox(HWND hwnd, UINT type, UINT msg){
    if (hwnd == NULL) {
        hwnd = HwndFunctions::getWindowHandle();
    }
    bool created = false;
    if(!hwnd){
        created = true;
        launchSyncClient();
		for (int i = 0; i < 3; i++) {
			hwnd = HwndFunctions::getWindowHandle();
			if (hwnd != NULL) {
				SetForegroundWindow(hwnd);
				break;
			} else {
				Sleep(1000);
			}
		}
    }

    //int ret = SendMessage(hwnd, ID_MYMSG_POPUP, buttons, msg);
    int ret = SendMessage(hwnd, ID_MYMSG_POPUP, type, NULL);

    if (created) {
        SendMessage(hwnd, WM_CLOSE, type, NULL);
    }
    return ret;
}

int updateProcedure(HWND hwnd, bool manual) {

    UpdateManager* up = getUpdateManager(CLIENT_PLATFORM, hwnd);
    up->setHwnd(hwnd);

    if (manual) {
        // Will start the upgrade even if it's not yet time to check (manual update)
        up->manualCheckForUpdates();
    }
    else {
        // Will start the upgrade if it's time to check
        up->checkForUpdates();
    }

    if (up->isNewVersionAvailable()) {
        return 1;
    }
    return 0;
}

bool isNewSwVersionAvailable() {

    UpdateManager* up = getUpdateManager(CLIENT_PLATFORM, NULL);
    return up->isNewVersionAvailable();
}

int checkUpdate() {

    UpdateManager* up = getUpdateManager(CLIENT_PLATFORM, NULL);
    return up->checkIsToUpdate();
}

bool checkForMandatoryUpdateBeforeStartingSync() {

    UpdateManager* up = getUpdateManager(CLIENT_PLATFORM, NULL);
    return up->checkForMandatoryUpdateBeforeStarting();
}

void launchSyncClient() {

    // Note: installDir of Windows Client is read from HKEY_LOCAL_MACHINE tree:
    OutlookConfig* config = getConfig();
    const char* dir = config->getWorkingDir();

    if (!dir || !strcmp(dir, "")) {
        return;
    }

    // program = "C:\...\FunambolClient.exe [param]"
    char* program = NULL;
    program = new char[strlen(dir) + strlen(PROGRAM_NAME_EXE) + 2];
    sprintf(program, "%s\\%s", dir, PROGRAM_NAME_EXE);

    STARTUPINFOA         si;
    PROCESS_INFORMATION  pi;
    DWORD                processId;
    DWORD                dwWaitRes = 0;
    DWORD                timeOut = 5 * 1000; // 10 secs of timeout

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    BOOL res = FALSE;

    //
    // Start the child process.
    //
    SetCurrentDirectoryA(dir);
    res = CreateProcessA(NULL,             // No module name (use command line).
                         program,
                         NULL,             // Process handle not inheritable.
                         NULL,             // Thread handle not inheritable.
                         FALSE,            // Set handle inheritance to FALSE.
                         0,                // No creation flags.
                         NULL,             // Use parent's environment block.
                         NULL,             // Use parent's starting directory.
                         &si,              // Pointer to STARTUPINFO structure.
                         &pi );            // Pointer to PROCESS_INFORMATION structure.

    // Save process ID!
    processId = pi.dwProcessId;

    dwWaitRes = WaitForSingleObject(pi.hProcess, timeOut);

    // Close process and thread handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (program) delete [] program;
}


__declspec(dllexport) StringBuffer getOutlookVersion() {

    StringBuffer name;

    ClientApplication* outlook = ClientApplication::getInstance();
    if (outlook) {
        wstring wName = outlook->getName();
        wName.append(TEXT(" (version = "));
        wName.append(outlook->getVersion());
        wName.append(TEXT(")"));
        name.convert(wName.c_str());
    }

    return name;
}

int manageSapiError(const int code) {

    ESapiSyncManagerError error = (ESapiSyncManagerError)code;

    switch (error) {
        case ESSMSuccess:
        {
            return WIN_ERR_NONE;                    // 0: OK
        }
        case ESSMCanceled:
        {
            return WIN_ERR_SYNC_CANCELED;           // 2: Sync aborted by the user
        }
        case ESSMNetworkError:
        {
            return WIN_ERR_NETWORK_ERROR;           // 2050
        }
        case ESSMAuthenticationError:
        {
            return WIN_ERR_INVALID_CREDENTIALS;     // 401
        }
        case ESSMServerQuotaExceeded:
        {
            return WIN_ERR_SERVER_QUOTA_EXCEEDED;   // 8
        }
        case ESSMClientQuotaExceeded:
        {
            return WIN_ERR_LOCAL_STORAGE_FULL;      // 9
        }        
        case ESSMSapiNotSupported:
        {
            return WIN_ERR_SAPI_NOT_SUPPORTED;      // 13
        }        
        case ESSMConfigError:
        case ESSMBeginSyncError:
        case ESSMEndSyncError:
        case ESSMGetItemError:
        case ESSMSetItemError:
        case ESSMGenericSyncError:
        case ESSMSapiError:
        default:
        {
            return WIN_ERR_GENERIC;                 // 1: generic error
        }
    }
}
