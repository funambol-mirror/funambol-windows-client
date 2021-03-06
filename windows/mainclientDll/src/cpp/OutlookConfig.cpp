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

#include "base/stringUtils.h"
#include "spdm/dmtree.h"
#include "spdm/DMTreeFactory.h"
#include "base/debug.h"
#include "spds/spdsutils.h"
#include "OutlookConfig.h"
#include "DefaultWinConfigFactory.h"
#include "winmaincpp.h"
#include "utils.h"
#include "DateFilter.h"
#include "customization.h"

#include "outlook/ClientApplication.h"
#include "outlook/ClientException.h"
#include "base/adapter/PlatformAdapter.h"

#include "UpdateManager.h"

using namespace std;

#define PROPERTY_ATTACH                "attach"
#define PROPERTY_LOG_NUM               "logNum"
#define PROPERTY_LOG_SIZE              "logSize"

//
// Init static pointer.
//
OutlookConfig* OutlookConfig::pinstance = NULL;


/**
 * Method to get the sole instance of OutlookConfig
 */
OutlookConfig* OutlookConfig::getInstance() {
    if (pinstance == NULL) {
        PlatformAdapter::init(PLUGIN_ROOT_CONTEXT);
        pinstance = new OutlookConfig;
    }
    return pinstance;
}

/// Returns true if static instance is not NULL.
bool OutlookConfig::isInstantiated() {
    return (pinstance ? true : false);
}


/// Constructor
//OutlookConfig::OutlookConfig() : DMTClientConfig(PLUGIN_ROOT_CONTEXT) {
OutlookConfig::OutlookConfig() : updaterConfig(PLUGIN_ROOT_CONTEXT), oneWayRemoval(false) {
    
    DMTClientConfig::initialize();
    winDC                 = NULL;
    workingDir            = NULL;
    logDir                = NULL;
    upgraded              = false;
    oldSwv                = 0;
}

/// Destructor
OutlookConfig::~OutlookConfig() {

    if (workingDir) {
        delete [] workingDir;
        workingDir = NULL;
    }
    if (logDir) {
        delete [] logDir;
        logDir = NULL;
    }
    pinstance = NULL;
}


const ArrayList& OutlookConfig::getSourcesVisible() {
    return sourcesVisible;
}


// ------------------------- Read properties from win registry -------------------------
/**
 * Read the configuration from Windows registry into this object. 
 * This method overrides 'DMTClientConfig::read()'.
 * 'DMTClientConfig::read()' is first called to read all common properties, then
 * specific SyncSource properties are retrieved.
 *
 * A separate 'winSourceConfigs' array is used to store all specific SS config, common
 * props are linked to original 'sourceConfigs' array (no copy!).
 *
 * @return TRUE if no errors
 */
bool OutlookConfig::read() {

    LOG.debug("entering %s", __FUNCTION__);
    unsigned int i=0;

    // Read timezone info.
    readCurrentTimezone();

    //
    // Read common properties
    //
    resetError();
    DMTClientConfig::read();

    // Check on the value of swv. If empty the config is not existing / corrupted.
    StringBuffer currentSwv(getClientConfig().getSwv());
    if (currentSwv.empty()) {
        return false;
    }


    // This param is not read by DMT (it's Client defined).
    // It's defaulted to swv in case it's not found.
    funambolSwv = readFunambolSwv(HKEY_CURRENT_USER);

    // Username/Password are stored encrypted.
    decryptPrivateData();


    // Set the appointments filters by date
    SyncSourceConfig* ssc = getSyncSourceConfig(APPOINTMENT_);
    if (ssc) {
        bool err = false;
        int val = ssc->getIntProperty(PROPERTY_FILTER_DATE_DIRECTION, &err);
        appointmentsDateFilter.setDirection((DateFilter::FilterDirection)val);

        val = ssc->getIntProperty(PROPERTY_FILTER_DATE_LOWER, &err);
        appointmentsDateFilter.setRelativeLowerDate((DateFilter::RelativeLowerDate)val);
    }

    // Set the CTCap for PIM sources
    readPIMSourcesCTCap();


    // Reads the list of sources visible.
    readSourcesVisible();

    // Current working dir: read 'installDir' from HKLM
    char* installPath = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_INSTALLDIR, HKEY_LOCAL_MACHINE);
    if (!installPath || strlen(installPath) == 0) {
        LOG.error(ERR_INSTALL_DIR);
        char msg[100];
        sprintf(msg, ERR_HKLM_KEYNOTFOUND, PROPERTY_INSTALLDIR);
        safeMessageBox(msg);
        exit(1);
    }
    setWorkingDir(installPath);


    // Current data dir: for LOG file and temporary files (no win registry).
    WCHAR* wlogPath = readAppDataPath();
    if (!wlogPath) {
        LOG.error(getLastErrorMsg());
        return false;
    }
    char* logPath = toMultibyte(wlogPath);
    setLogDir(logPath);


    // Reset abortSync flag
    setToAbort(false);


    if (installPath) delete [] installPath;
    if (logPath)     delete [] logPath;
    if (wlogPath)    delete [] wlogPath;
    return true;
}

void OutlookConfig::readPIMSourcesCTCap() {

    SyncSourceConfig* ssc = getSyncSourceConfig(CONTACT_);
    if (ssc) {
        ArrayList* p = getVCardProperties();
        ssc->addCtCap(p, "text/x-vcard", "2.1");
        delete p;
    }

    ssc = getSyncSourceConfig(APPOINTMENT_);
    if (ssc) {
        ArrayList* p = getVCalendarProperties();
        ssc->addCtCap(p, "text/x-vcalendar", "1.0");
        delete p;
    }

    ssc = getSyncSourceConfig(TASK_);
    if (ssc) {
        ArrayList* p = getVTodoProperties();
        ssc->addCtCap(p, "text/x-vcalendar", "1.0");
        delete p;
    }

    ssc = getSyncSourceConfig(NOTE_);
    if (ssc) {
        ArrayList* p = getNoteProperties();
        ssc->addCtCap(p, "text/x-vcalendar", "1.0");
        p->clear();
		p = getVNoteProperties();
		ssc->addCtCap(p, "text/x-vnote", "1.1"); 
        delete p;
    }
}



/**
 * Reads all sources timestamps from win registry and set
 * all values into configuration.
 */
void OutlookConfig::readSourcesTimestamps() {

    if (!open()) {
        return;
    }

    for (unsigned int i=0; i<sourceConfigsCount; i++) {
        ManagementNode* node = sourcesNode->getChild(i);
        if (node) {
            // This sets only variables that the library uses internally, like anchors 
            readSourceVars(i, *sourcesNode, *node);
        }
    }
}


/**
 * Reads only "sync" properties of each source, to win registry.
 */
void OutlookConfig::readSyncModes() {

    if (!open()) {
        return;
    }

    for (unsigned int i=0; i<sourceConfigsCount; i++) {
        ManagementNode* node = sourcesNode->getChild(i);
        if (node) {
            char* tmp = node->readPropertyValue(PROPERTY_SOURCE_SYNC);
            sourceConfigs[i].setSync(tmp);
            delete [] tmp;

            tmp = node->readPropertyValue(PROPERTY_SOURCE_ENABLED);
            sourceConfigs[i].setIsEnabled(strcmp(tmp, "0")? true:false);    // Set true if any value different from "0" (also if empty);
            delete [] tmp;
        }
    }
}


bool OutlookConfig::fixSyncModes() {

    bool ret = false;
    for (unsigned int i=0; i<sourceConfigsCount; i++) {

        SyncSourceConfig* ssc = getSyncSourceConfig(i);
        if (!ssc) continue;

        const char* name = ssc->getName();
        const char* sync = ssc->getSync();

        SyncMode code = syncModeCode(sync);
        if (isPIMSource(name) && isFullSyncMode(code)) {

            LOG.debug("Restoring default syncmode for source %s (was %s)", name, sync);
            ret = true;
            
            if (!strcmp(name, CONTACT_)) { 
                ssc->setSync(DEFAULT_CONTACTS_SYNC_MODE); 
            } 
            else if (!strcmp(name, APPOINTMENT_)) { 
                ssc->setSync(DEFAULT_APPOINTMENTS_SYNC_MODE); 
            } 
            else if (!strcmp(name, TASK_)) { 
                ssc->setSync(DEFAULT_TASKS_SYNC_MODE); 
            } 
            else if (!strcmp(name, NOTE_)) { 
                ssc->setSync(DEFAULT_NOTES_SYNC_MODE); 
            }
        }
    }
    return ret;
}


/**
 * Populate 'currentTimezone' structure, reading values from TIME_ZONE_INFORMATION
 * and also directly from Windows Registry.
 * @note  some mandatory informations cannot be retrieved from Win32 API calls,
 *        so we have to get them from HKLM keys.
 * @return 0 if no errors
 */
int OutlookConfig::readCurrentTimezone() {

    //
    // Get all known info from TIME_ZONE_INFORMATION.
    //
    TIME_ZONE_INFORMATION tzInfo;
    DWORD tzID = GetTimeZoneInformation(&tzInfo);
    if (tzID == TIME_ZONE_ID_DAYLIGHT) {
        currentTimezone.isDaylight = true;
    }
    else {
        currentTimezone.isDaylight = false;
    }
    currentTimezone.bias         = tzInfo.Bias;
    currentTimezone.daylightBias = tzInfo.DaylightBias;
    currentTimezone.daylightDate = tzInfo.DaylightDate;
    currentTimezone.daylightName = tzInfo.DaylightName;
    currentTimezone.standardBias = tzInfo.StandardBias;
    currentTimezone.standardDate = tzInfo.StandardDate;
    currentTimezone.standardName = tzInfo.StandardName;

    //
    // Now go directly to Win Registry keys and get the 
    // other mandatory informations.
    //
    bool found = false;
    HKEY hkTimeZones;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, TIMEZONE_CONTEXT, 0, KEY_READ, &hkTimeZones) == ERROR_SUCCESS) {
        HKEY  hkTimeZone;
        DWORD dwIndex = 0;
        WCHAR keyName[DIM_MANAGEMENT_PATH];
        DWORD keyNameLenght = DIM_MANAGEMENT_PATH;

        // Scan all timezones, searching for the current one.
        while (RegEnumKey(hkTimeZones, dwIndex++, keyName, keyNameLenght) != ERROR_NO_MORE_ITEMS) {
            if (RegOpenKeyEx(hkTimeZones, keyName, 0, KEY_READ, &hkTimeZone) == ERROR_SUCCESS) {

                WCHAR stdName[DIM_MANAGEMENT_PATH];
                DWORD dwDataSize = DIM_MANAGEMENT_PATH * sizeof(WCHAR);
                RegQueryValueEx(hkTimeZone, L"Std", NULL, NULL, (BYTE*)&stdName, &dwDataSize);
                if (!wcscmp(stdName, currentTimezone.standardName.c_str())) {
                    found = true;

                    // Get Index
                    DWORD dwTimeZoneIndex;
                    dwDataSize = sizeof(DWORD);
                    RegQueryValueEx(hkTimeZone, L"Index", NULL, NULL, (BYTE*)&dwTimeZoneIndex, &dwDataSize);

                    // Get Display name
                    WCHAR displayName[DIM_MANAGEMENT_PATH];
                    dwDataSize = DIM_MANAGEMENT_PATH * sizeof(WCHAR);
                    RegQueryValueEx(hkTimeZone, L"Display", NULL, NULL, (BYTE*)&displayName, &dwDataSize);

                    // Set properties to currentTimezone struct.
                    currentTimezone.index       = dwTimeZoneIndex;
                    currentTimezone.displayName = displayName;
                    currentTimezone.keyName     = keyName;

                    RegCloseKey(hkTimeZone);
                    break;
                }
            }
            keyNameLenght = DIM_MANAGEMENT_PATH;
            RegCloseKey(hkTimeZone);
        }
        RegCloseKey(hkTimeZones);
    }
    else {
        return 1;
    }

    if (!found) {
        LOG.info("Error reading the timezone info from Win Registry");
        return 1;
    }

    LOG.debug("Current Timezone = %ls", currentTimezone.displayName.c_str());
    return 0;
}


void OutlookConfig::readSourcesVisible(HKEY rootKey) {

    sourcesVisible.clear();

    // Read the (comma separated) source names
    const char* tmp = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_SOURCE_ORDER, rootKey);
    StringBuffer sources(tmp);
    delete [] tmp;

    if (sources.empty()) {
        // this is called the first time when the registry is
        sources = SOURCE_ORDER_IN_REGISTRY;
    }
    // Get the source names, and add them to the sourceVisible array
    if (!sources.empty()) {
        ArrayList tokens;
        sources.split(tokens, ",");

        for (int i=0; i<tokens.size(); i++) {
            StringBuffer* token = (StringBuffer*)tokens.get(i);
            safeAddSourceVisible(*token);
        }
    }

}


bool OutlookConfig::safeAddSourceVisible(const char* sourceName, bool onlyIfDefault) {

    if (onlyIfDefault) {
        StringBuffer defaultSources = SOURCE_ORDER_IN_REGISTRY;
        if (defaultSources.find(sourceName) == StringBuffer::npos) {
            // Not found in the default source list at installation time
            return false;
        }
    }

    if (!sourceName || !strlen(sourceName)) {
        // Invalid source name
        return false;
    }

    for (int i=0; i<sourcesVisible.size(); i++) {
        StringBuffer* element = (StringBuffer*)sourcesVisible.get(i);
        if (*element == sourceName) {
            // found: don't add
            return false;
        }
    }

    // not found: add
    StringBuffer source(sourceName);
    sourcesVisible.add(source);
    return true;
}


bool OutlookConfig::removeSourceVisible(const char* sourceName) {

    for (int i=0; i<sourcesVisible.size(); i++) {
        StringBuffer* element = (StringBuffer*)sourcesVisible.get(i);
        if (*element == sourceName) {
            // found: remove
            sourcesVisible.removeElementAt(i);
            return true;
        }
    }

    // not found
    return false;
}



// ---------------------------- Save properties to win registry ----------------------------
bool OutlookConfig::save() {

    LOG.debug("entering %s", __FUNCTION__);

    bool ret = false;
    LOG.debug(DBG_WRITING_CONFIG_TO_DM);

    if (!open()) {
        return false;
    }

    // Save the appointments filters by date
    SyncSourceConfig* ssc = getSyncSourceConfig(APPOINTMENT_);
    if (ssc) {
        ssc->setIntProperty(PROPERTY_FILTER_DATE_DIRECTION, appointmentsDateFilter.getDirection());
        ssc->setIntProperty(PROPERTY_FILTER_DATE_LOWER, appointmentsDateFilter.getRelativeLowerDate());
    }

    // Username/Password are stored encrypted.
    encryptPrivateData();

    //
    // Save ALL!
    // ---------
    DMTClientConfig::save();

    // If asked, we need to return clear data...
    decryptPrivateData();

    // Saves the Funambol sw version
    saveFunambolSwv();

    // Saves the list of sources visible.
    saveSourcesVisible();

    // Must update the syncurl in the updater module if empty
    UpdateManager* up = getUpdateManager(CLIENT_PLATFORM, NULL);
    if (getUpdaterConfig().getUrlCheck().empty()) {
        up->setURLCheck(getSyncURL());
    }

    resetError();
    return true;
}


/**
 * Save only "sync" properties of each source, to win registry.
 */
void OutlookConfig::saveSyncModes() {

    if (!sourcesNode) {
        open();
    }

    ManagementNode* node = NULL;
    for(unsigned int i=0; i<sourceConfigsCount; ++i) {
        node = sourcesNode->getChild(i);
        if (node) {
            node->setPropertyValue(PROPERTY_SOURCE_SYNC,    sourceConfigs[i].getSync());
            node->setPropertyValue(PROPERTY_SOURCE_ENABLED, sourceConfigs[i].isEnabled() ? "1":"0");
        }
        node = NULL;
    }
}



void OutlookConfig::savePropertyValue(const StringBuffer& context, const StringBuffer& name, const StringBuffer& value) {

    ManagementNode* node = NULL;
    DMTree* dmt = DMTreeFactory::getDMTree(PLUGIN_ROOT_CONTEXT);
    if (!dmt) goto finally;

    node = dmt->readManagementNode(context.c_str());
    if (!node) goto finally;

    node->setPropertyValue(name.c_str(), value.c_str());

finally:
    if (dmt)   delete dmt;
    if (node)  delete node;
    return;
}

void OutlookConfig::deletePropertyValue(const char* context, const char* name) {

    if (!context || !name) {
        return;
    }

    ManagementNode* node = NULL;
    DMTree* dmt = DMTreeFactory::getDMTree(PLUGIN_ROOT_CONTEXT);
    if (!dmt) goto finally;

    node = dmt->readManagementNode(context);
    if (!node) goto finally;

    node->deleteProperty(name);

finally:
    if (dmt)   delete dmt;
    if (node)  delete node;
}



void OutlookConfig::saveBeginSync() {

    char buf[32];
    timestampToAnchor(getAccessConfig().getBeginSync(), buf);

    StringBuffer context;
    context.sprintf("%s%s%s", PLUGIN_ROOT_CONTEXT, CONTEXT_SPDS_SYNCML, CONTEXT_EXT);
    savePropertyValue(context, PROPERTY_SYNC_BEGIN, buf);
}

void OutlookConfig::saveFunambolSwv() {

    StringBuffer context;
    context.sprintf("%s%s%s", PLUGIN_ROOT_CONTEXT, CONTEXT_SPDS_SYNCML, CONTEXT_DEV_DETAIL);
    savePropertyValue(context, PROPERTY_FUNAMBOL_SWV, funambolSwv);
}


void OutlookConfig::saveSourcesVisible() {

    // Joins all the source names in a comma separated string
    StringBuffer sources;
    sources.join(sourcesVisible, ",");

    // Saves to registry
    savePropertyValue(PLUGIN_ROOT_CONTEXT, PROPERTY_SOURCE_ORDER, sources);
}



// ------------------------------ Get/Set objects ----------------------------------

void OutlookConfig::setWorkingDir(const char* v) {
    if (v) {
        if (workingDir) {
            delete [] workingDir;
        }
        workingDir = stringdup(v);
    }
}
const char* OutlookConfig::getWorkingDir() const {
    return workingDir;
}

void OutlookConfig::setLogDir(const char* v) {
    if (v) {
        if (logDir) {
            delete [] logDir;
        }
        logDir = stringdup(v);
    }
}
const char* OutlookConfig::getLogDir() const {
    return logDir;
}

void OutlookConfig::setFunambolSwv(const StringBuffer& v) {
    funambolSwv = v;
}


const StringBuffer& OutlookConfig::getFunambolSwv() {
    return funambolSwv;
}


/**
 * Save the value to win registry (HKCU), because it can 
 * be required from a different instance of plugin.
 */
void OutlookConfig::setScheduledSync(const bool v) {
    
    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    char value[2];
    sprintf(value, "%d", v);

    // Save value.
    dmt = DMTreeFactory::getDMTree(PLUGIN_ROOT_CONTEXT);
    if (!dmt) return;
    node = dmt->readManagementNode(PLUGIN_ROOT_CONTEXT);
    if (!node) return;
    node->setPropertyValue(PROPERTY_SCHEDULED_SYNC, value);

    delete dmt;
    delete node;
}
/**
 * Retrieve from win registry (HKCU).
 */
const bool OutlookConfig::getScheduledSync() const {

    DMTree* dmt = NULL;
    ManagementNode* node = NULL;

    // Get value.
    dmt = DMTreeFactory::getDMTree(PLUGIN_ROOT_CONTEXT);
    if (!dmt)   return false;
    node = dmt->readManagementNode(PLUGIN_ROOT_CONTEXT);
    if (!node)  return false;
    char* value = node->readPropertyValue(PROPERTY_SCHEDULED_SYNC);
    delete dmt;
    delete node;
    if (!value) return false;

    bool ret = false;
    if (!strcmp(value, "1")) {
        ret = true;
    }
    if (value) {
        delete [] value;
    }
    return ret;
}


/**
 * Returns a pointer to the currentTimezone internal structure.
 */
const TimeZoneInformation* OutlookConfig::getCurrentTimezone() const {
    return &currentTimezone;
}




// ------------------------------ Other methods ----------------------------------
/**
 * Creates a default configuration object.
 * Uses DefaultWinConfigFactory methods to populate config objects.
 */
void OutlookConfig::createDefaultConfig() {

    LOG.debug("entering %s", __FUNCTION__);

    //
    // AccessConfig
    //
    AccessConfig* ac = DefaultWinConfigFactory::getAccessConfig();
    setAccessConfig(*ac);
    delete ac;

    //
    // DeviceConfig
    //
    DeviceConfig * dc = DefaultWinConfigFactory::getDeviceConfig();
    DMTClientConfig::setDeviceConfig(*dc);
    WindowsDeviceConfig* wdc = DefaultWinConfigFactory::getWindowsDeviceConfig(DMTClientConfig::getDeviceConfig());
    setDeviceConfig(*wdc);
    delete dc;

    //
    // SapiConfig
    //
    SapiConfig* sapiConfig = DefaultWinConfigFactory::getSapiConfig();
    setSapiConfig(*sapiConfig);
    delete sapiConfig;


    // Set a unique deviceID = "fol-<pcName>:<userName>"
    setUniqueDevID();


    //
    // Create 'sourceConfig' array for ALL sources (even if not visible).
    // 
    // If config not existing for some sources, it will be created inside 'setSyncSourceConfig()'
    // Create sources alphabetically sorted, because this will be the order of 
    // nodes inside Win registry (and they must match)!
    WCHAR* sourceNames[7] = {APPOINTMENT, CONTACT, FILES, NOTE, PICTURE, TASK, VIDEO};
    for (int i=0; i<7; i++) {
        WCHAR* wname = sourceNames[i];
        SyncSourceConfig* sc = DefaultWinConfigFactory::getSyncSourceConfig(wname);
        setSyncSourceConfig(*sc);
        delete sc;
    }

    
    // Reset flags
    setToAbort(false);

    // Read the sources visible list (if specified from HKLM keys: customers builds)
    readSourcesVisible(HKEY_LOCAL_MACHINE);
    

    // Current working dir: read 'installDir' from HKLM
    char* installPath = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_INSTALLDIR, HKEY_LOCAL_MACHINE);
    if (!installPath || strlen(installPath) == 0) {
        LOG.error(ERR_INSTALL_DIR);
        return;
    }
    setWorkingDir(installPath);


    // Current data dir: for LOG file and temporary files (no win registry).
    WCHAR* wlogPath = readAppDataPath();
    if (!wlogPath) {
        LOG.error(getLastErrorMsg());
        return;
    }
    char* logPath = toMultibyte(wlogPath);
    setLogDir(logPath);
    
    // set the sapi mediaHub path in the right source config and delete the temp node
    char* mediaHubPath = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_MEDIAHUB_PATH, HKEY_CURRENT_USER);
    if (mediaHubPath  && strcmp(mediaHubPath, "") != 0) {
        SyncSourceConfig* sc = getSyncSourceConfig(PICTURE_);
        sc->setProperty(PROPERTY_MEDIAHUB_PATH, mediaHubPath);
        sc = DMTClientConfig::getSyncSourceConfig(VIDEO_);
        sc->setProperty(PROPERTY_MEDIAHUB_PATH, mediaHubPath);
        sc = DMTClientConfig::getSyncSourceConfig(FILES_);
        sc->setProperty(PROPERTY_MEDIAHUB_PATH, mediaHubPath);

        ManagementNode* n = NULL;
        DMTree* d = DMTreeFactory::getDMTree(PLUGIN_ROOT_CONTEXT);
        if (d) {
            n = d->readManagementNode(PLUGIN_ROOT_CONTEXT);
            if (n) {
                n->deleteProperty(PROPERTY_MEDIAHUB_PATH);
                delete n;
                delete d;
            }
        }
    }

    if (installPath)  delete [] installPath;
    if (logPath)      delete [] logPath;
    if (wlogPath)     delete [] wlogPath;
    if (mediaHubPath) delete [] mediaHubPath;
}



/**
 * Checks whether the config has to be upgraded to a new version.
 * It checks the difference between:
 *   swv from HKLM: this is set by installer, current version
 *   swv from HKCU: previous version of this config
 *
 * @return  'true' if the config needs to be upgraded.
 *          'false' if no upgrade is necessary.
 */
bool OutlookConfig::checkToUpgrade() {

    LOG.debug("entering %s", __FUNCTION__);

    bool ret = false;
    const char* newSwv = readCurrentSwv();
    const char* oldSwv = getClientConfig().getSwv();
    if (strcmp(oldSwv, newSwv)) {
        ret = true;
    }
    if (newSwv) delete [] newSwv;
    return ret;
}


void OutlookConfig::initializeVersionsAndUserAgent() {

    // Backup old Swv and save the new one.
    oldSwv = getBuildNumberFromVersion(getClientConfig().getSwv());
    const char* newSwv = readCurrentSwv();
    getClientConfig().setSwv(newSwv);

    // Backup old Funambol product Swv and save the new one.
    oldFunambolSwv = getBuildNumberFromVersion(getFunambolSwv().c_str());
    StringBuffer funambolNewSwv = readFunambolSwv(HKEY_LOCAL_MACHINE);
    setFunambolSwv(funambolNewSwv);

    if (DLLCustomization::shouldFakeOldFunambolSwv) {
        oldFunambolSwv = DLLCustomization::fakeOldFunambolSwv;
    }

    // Set the new User Agent = "Funambol Windows Sync Client v. x.y.z"
    // It is a fixed value even for branded builds.
    StringBuffer ua(FUNAMBOL_USER_AGENT);
    ua += " v. ";
    ua += funambolNewSwv;
    accessConfig.setUserAgent(ua.c_str());

    delete [] newSwv;
}


/**
 * ---- Update config with values from HKLM (set by installer) ----
 * This is useful when the client has just been upgraded to a new version,
 * only some properties (like 'swv' and 'userAgent') must be corrected.
 */
void OutlookConfig::upgradeConfig() {
    
    LOG.debug("entering %s", __FUNCTION__);

    // always!
    initializeVersionsAndUserAgent();

    // Old version < 8.2.0
    if (oldFunambolSwv < 80200) {

        // Pictures source added.
        if (!addSyncSourceConfig(PICTURE_)) {
            LOG.error("upgradeConfig - error adding the config for %s source", PICTURE_);
        }

        // added SyncSource boolean 'enabled' to enable/disable a source
        // without losing the sync direction information.
        // If syncmode = none -> disable ssource.
        for (unsigned int i=0; i<sourceConfigsCount; i++) {
            SyncSourceConfig* sc = getSyncSourceConfig(i);
            if (sc) {
                StringBuffer syncMode = sc->getSync();
                if (syncMode == "none") { 
                    sc->setIsEnabled(false);
                    sc->setSync("two-way");    // Just as a default, if was disabled
                }
                else { 
                    sc->setIsEnabled(true); 
                }
            }
        }
    }

    // Old version < 8.7.0
    if (oldFunambolSwv < 80700) {
        getAccessConfig().setMaxMsgSize(MAX_SYNCML_MSG_SIZE);       // now it's 125K
        getAccessConfig().setResponseTimeout(RESPONSE_TIMEOUT);     // now it's 15min

		//
		// SIF-E and SIF-T deprecation. Changed the default in the upgrade from sif to vcalendar
		//
		SyncSourceConfig* ssc = getSyncSourceConfig(APPOINTMENT_);
        if (ssc) {
			// Don't change the source URI. If we were using "scal", it will be preserved during
			// upgrade. This is required to keep the anchors Server side and avoid a 1st time slow-sync.
            // ssc->setURI("event");
			ssc->setType("text/x-vcalendar");
            ssc->setVersion("1.0");
            ssc->setEncoding("bin");
			ssc->setSupportedTypes("text/x-vcalendar:1.0,text/x-s4j-sife:1.0");
			
        }

		ssc = getSyncSourceConfig(TASK_);
        if (ssc) {
			// Don't change the source URI. If we were using "stask", it will be preserved during
			// upgrade. This is required to keep the anchors Server side and avoid a 1st time slow-sync.
            // ssc->setURI("task");
			ssc->setType("text/x-vcalendar");
            ssc->setVersion("1.0");
            ssc->setEncoding("bin");
			ssc->setSupportedTypes("text/x-vcalendar:1.0,text/x-s4j-sife:1.0");
        }

		getAccessConfig().setCompression(ENABLE_COMPRESSION);
    }

	// Old version < 9.0.0
    if (oldFunambolSwv < 90000) {

        // Changed the syncModes param for all sources
        SyncSourceConfig* ssc = getSyncSourceConfig(CONTACT_);
        if (ssc) ssc->setSyncModes(CONTACTS_DEVINFO_SYNC_MODES); 

        ssc = getSyncSourceConfig(APPOINTMENT_);
        if (ssc) ssc->setSyncModes(APPOINTMENTS_DEVINFO_SYNC_MODES); 

        ssc = getSyncSourceConfig(TASK_);
        if (ssc) ssc->setSyncModes(TASKS_DEVINFO_SYNC_MODES); 

        ssc = getSyncSourceConfig(NOTE_);
        if (ssc) ssc->setSyncModes(NOTES_DEVINFO_SYNC_MODES); 

        ssc = getSyncSourceConfig(PICTURE_);
        if (ssc) ssc->setSyncModes(PICTURES_DEVINFO_SYNC_MODES); 

        // One-way syncmodes have been removed for PIM sources.
        // (the action of setting the default syncmode and force a slow is done below)
        for (unsigned int i=0; i<sourceConfigsCount; i++) {
            ssc = getSyncSourceConfig(i);
            if (ssc) {
                const char* name = ssc->getName();
                if (isPIMSource(name)) {
                    const char* modeInUse = ssc->getSync();
                    if (!strcmp(modeInUse, "one-way-client") ||                 // that's the old style syncmode
                        !strcmp(modeInUse, "one-way-server") ||                 // that's the old style syncmode
                        !strcmp(modeInUse, SYNC_MODE_ONE_WAY_FROM_CLIENT) ||
                        !strcmp(modeInUse, SYNC_MODE_ONE_WAY_FROM_SERVER)) {
                        oneWayRemoval = true;
                        break;
                    }
                }
            }
        }       
    }
    
    // Old version < 10.0.0
    if (oldFunambolSwv < 100000) {
        
        // Update pictures parameters: 
        //   - keep syncmode (1-way-from-server), isEnabled, endTstamp (for UI state)
        //   - reset last (now it's used for uploads!)
        //   - add new SAPI params
        //   - remove obsolete keys (folderPath, useSubfolders)
        SyncSourceConfig* sc = getSyncSourceConfig(PICTURE_);
        if (sc) {
            sc->setLast             (0);
            sc->setSyncModes        (PICTURES_DEVINFO_SYNC_MODES);
            sc->setType             ("image/*");      
            sc->setSupportedTypes   ("application/*");   
            sc->setProperty         (PROPERTY_DOWNLOAD_LAST_TIME_STAMP,     "0");
            sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_CLIENT, -1);
            sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_SERVER, -1);
            sc->setProperty         (PROPERTY_EXTENSION,                    PICT_EXTENSION);
            sc->setProperty         (PROPERTY_MEDIAHUB_PATH,                "");
            sc->setProperty         (PROPERTY_LOCAL_QUOTA_STORAGE,          SAPI_LOCAL_QUOTA_STORAGE);
            sc->setIntProperty      (PROPERTY_SYNC_ITEM_MAX_SIZE,           SAPI_MAX_PICTURE_SIZE);
            sc->removeProperty      (PROPERTY_FOLDER_PATH);
            sc->removeProperty      (PROPERTY_USE_SUBFOLDERS);
        }
        StringBuffer path(PLUGIN_ROOT_CONTEXT CONTEXT_SPDS_SOURCES "/" PICTURE_);
        deletePropertyValue(path.c_str(), PROPERTY_FOLDER_PATH);
        deletePropertyValue(path.c_str(), PROPERTY_USE_SUBFOLDERS);

        safeAddSourceVisible(PICTURE_);

        // Videos source added.
        if (!addSyncSourceConfig(VIDEO_)) {
            LOG.error("upgradeConfig - error adding the config for %s source", VIDEO_);
        }
        safeAddSourceVisible(VIDEO_);

         // Files source added.
        if (!addSyncSourceConfig(FILES_)) {
            LOG.error("upgradeConfig - error adding the config for %s source", FILES_);
        }
        safeAddSourceVisible(FILES_);

        // SapiConfig added.
        SapiConfig* sapiConfig = DefaultWinConfigFactory::getSapiConfig();
        setSapiConfig(*sapiConfig);
        delete sapiConfig;

        // set the sapi mediaHub path in the right source config and delete the temp node
        char* mediaHubPath = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_MEDIAHUB_PATH, HKEY_CURRENT_USER);
        if (mediaHubPath  && strcmp(mediaHubPath, "") != 0) {
            SyncSourceConfig* sc = DMTClientConfig::getSyncSourceConfig(PICTURE_);
            sc->setProperty(PROPERTY_MEDIAHUB_PATH, mediaHubPath);
            sc = DMTClientConfig::getSyncSourceConfig(VIDEO_);
            sc->setProperty(PROPERTY_MEDIAHUB_PATH, mediaHubPath);
            sc = DMTClientConfig::getSyncSourceConfig(FILES_);
            sc->setProperty(PROPERTY_MEDIAHUB_PATH, mediaHubPath);

            deletePropertyValue(PLUGIN_ROOT_CONTEXT, PROPERTY_MEDIAHUB_PATH);
        }
        if (mediaHubPath) { delete [] mediaHubPath; }
    }

    if (oldFunambolSwv < 100002) {

        // Added config params
        DeviceConfig& dc = getClientConfig();
        dc.setAutoSync               (DEFAULT_AUTO_SYNC);
	    dc.setDataplanExpirationDate (0L);
	    dc.setNetworkWarning         (false);

        // Added source allowed flag
        for (unsigned int i=0; i<sourceConfigsCount; i++) {
            SyncSourceConfig* sc = getSyncSourceConfig(i);
            if (sc) {
                sc->setIsAllowed(true);
            }
        }
    }


    // ALWAYS - if a syncmode currently unavailable was in use, 
    // the source is disabled and the default is set + last anchor reset (SLOW).
    for (unsigned int i=0; i<sourceConfigsCount; i++) {
        SyncSourceConfig* sc = getSyncSourceConfig(i);
        if (sc) {
            const char* modeInUse = sc->getSync();
            StringBuffer modes = sc->getSyncModes();
            if (modes.find(modeInUse) == StringBuffer::npos) {
                sc->setSync(getDefaultSyncMode(sc->getName()));
                sc->setIsEnabled(false);
                sc->setLast(0);
            }
        }
    }

    // ALWAYS force the GET of Server capabilities at next sync.
    // (to make sure all server caps are parsed, even the new ones) 
    setServerLastSyncURL("");
        
    // Set the flag to specify that config has been upgraded.
    upgraded = true;

    // delete the updater tree when the upgrade has been finished
    ManagementNode* n = NULL;
    DMTree* d = DMTreeFactory::getDMTree(PLUGIN_ROOT_CONTEXT);
    if (d) {
        n = d->readManagementNode(PLUGIN_ROOT_CONTEXT);
        if (n) {
            n->deletePropertyNode(CONTEXT_UPDATER);
            delete n;
        }
        delete d;
    }
}


/**
 * Returns true if config has been upgraded from a previous version.
 */
bool OutlookConfig::isUpgraded() {
    return upgraded;
}

/**
 * Returns the old installed swv (for upgrades). 
 * '0' if not an upgrade.
 */
int OutlookConfig::getOldSwv() {
    return oldSwv;
}

int OutlookConfig::getOldFunambolSwv() {
    return oldFunambolSwv;
}


/**
 * Returns the current software version, read it from HKLM registry.
 * This value is set and updated ONLY by installer.
 * Returns a new allocated buffer, must be deleted by the caller.
 */
char* OutlookConfig::readCurrentSwv() {
    return readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_SOFTWARE_VERSION, HKEY_LOCAL_MACHINE);
}

StringBuffer OutlookConfig::readFunambolSwv(HKEY rootKey) {

    StringBuffer ret;
    const char* value = NULL;

    if (rootKey == HKEY_CURRENT_USER) {
        StringBuffer context;
        context.sprintf("%s%s%s", SOFTWARE_ROOT_CONTEXT, CONTEXT_SPDS_SYNCML, CONTEXT_DEV_DETAIL);
        value = readPropertyValue(context, PROPERTY_FUNAMBOL_SWV, HKEY_CURRENT_USER);
        if (!value || strlen(value)==0) {

            // 'funambol_swv' is not found
            const char* customer = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_CUSTOMER, HKEY_LOCAL_MACHINE);
            if (customer && strlen(customer)>0) {
                // current funambol_swv is an acceptable value for customers builds (swv could be 1.0.0 for example)
                LOG.debug("Customer = %s", customer);
                ret = "8.0.0";
            }
            else {
                // It's an old Funambol build: use the swv.
                value = readPropertyValue(context, PROPERTY_SOFTWARE_VERSION, HKEY_CURRENT_USER);
            }
            delete [] customer;
        }
    }

    else if (rootKey == HKEY_LOCAL_MACHINE) {
        // this is the default value if the 'funambol_swv' is not found.
        ret = getSwv();
        value = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_FUNAMBOL_SWV, rootKey);
    }

    if (value && strlen(value)>0) {
        ret = value;
    }
    delete [] value;
    return ret;
}



/**
 * Creates and set a unique 'devID' property for current configuration.
 * The devID depends on:
 *   %1: the local machine name
 *   %2: the Windows current user name
 *   %3: the current Outlook profile name (now disabled)
 *
 * If one of these parameters has changed, the devID generated is different.
 * The devID property is then set into current configuration, in
 * the format "fol-%1:%2:%3" where %1 %2 %3 are the parameters already described.
 * Parameters are also encoded in base64.
 *
 * @return  0 if no errors occurred
 */
int OutlookConfig::setUniqueDevID() {
    
    int ret = 0, len = 0;
    DWORD code = 0;
    char* msg  = NULL;
    DWORD bufSize = 128;
    char computerName[128], userName[128];
    //ClientApplication* ol;
    wstring wprofileName;

    //
    // NetBIOS name of the local computer.
    //
    if (!GetComputerNameA(computerName, &bufSize)) {
        code = GetLastError();
        msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_MACHINE_NAME, code, msg);
        delete [] msg;
        return 1;
    }
    len += bufSize;

    //
    // UserName of the owner of current thread.
    //
    bufSize = 128;
    if (!GetUserNameA(userName, &bufSize)) {
        code = GetLastError();
        msg = readSystemErrorMsg(code);
        setErrorF(getLastErrorCode(), ERR_USER_NAME, code, msg);
        delete [] msg;
        return 1;
    }
    len += bufSize;

    //
    // Name of current Outlook profile used (SHOULD be logged in!) 
    // ------------------ disabled by now -----------------------
    //try {
    //    ol = ClientApplication::getInstance();
    //    wprofileName = ol->getCurrentProfileName();
    //}
    //catch (ClientException* e) {
    //    manageClientException(e);
    //    ret = 1;
    //}
    //char* profileName = toMultibyte(wprofileName.c_str());
    //len += strlen(profileName);


    //
    // compose devID property -> encode to b64.
    //
    char* id = new char[len+2];
    sprintf(id, "%s:%s", computerName, userName);
    len = strlen(id);
    char* enc = new char[(len/3 + 1)*4 + 1];
    len = b64_encode(enc, id, len);
    enc[len] = 0;

    char* devID = new char[len+5];
    sprintf(devID, "%s-%s", DEVICE_ID_PREFIX, enc);


    // Set it to configuration.
    getClientConfig().setDevID(devID);
    LOG.info(INFO_CONFIG_DEVID_SAVED, devID);


    //if (profileName)  delete [] profileName;
    if (id)           delete [] id;
    if (enc)          delete [] enc;
    if (devID)        delete [] devID;

    return ret;
}



/*
 * Returns the value of the given property, from rootKey tree (read only).
 * The value is returned as a new char array and must be freed by the user.
 *
 * @param context      : full context of key, under rootKey
 * @param propertyName : name of property to retrieve
 * @param rootKey      : one of  HKEY_LOCAL_MACHINE
 *                               HKEY_CLASSES_ROOT
 *                               HKEY_CURRENT_USER
 *                               HKEY_USERS
 *                               ...
 * @return               the property value (new allocated buffer) - 
 *                       if key not found, returns an empty string.
 */
char* OutlookConfig::readPropertyValue(const char* contextA, const char* propertyNameA, HKEY rootKey) {
    
    DWORD res = 0;  	
    long  err = 0;
    ULONG dim = 0;
    HKEY  key = NULL;
    char* ret = NULL;

    // Need to convert all '/' into '\'.
    char* fullContextA = stringdup(contextA);
    toWindows(fullContextA);

    WCHAR* fullContext = toWideChar(fullContextA);
    WCHAR* propertyName = toWideChar(propertyNameA);

    RegCreateKeyEx(
            rootKey,
            fullContext,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_READ,                           // Read only: could be from a limited rights user.
            NULL,
            &key,
            &res
            );

    if (key == 0) {
        setErrorF(ERR_INVALID_CONTEXT, ERR_INVALID_REG_PATH, fullContextA);
        goto finally;
    }

    // Get value length
    err = RegQueryValueEx(
            key,
            propertyName,
            NULL,
            NULL,  // we currently support only strings
            NULL,
            &dim
            );

    if (err == ERROR_SUCCESS) {
		if (dim > 0) {
            TCHAR* buf = new TCHAR[dim + 1];

			err = RegQueryValueEx(
					key,
					propertyName,
					NULL,
					NULL,  // we currently support only strings
					(UCHAR*)buf,
					&dim 
                    );
            if (err == ERROR_SUCCESS) {
                ret = toMultibyte(buf);                
            }
            delete [] buf;
		}
    }

finally:

    if (!ret) {
        // Always return an empty string if key not found!
        ret = stringdup(EMPTY_STRING);
    }
    if (fullContext) {
        delete [] fullContext;
    }
    if (fullContextA) {
        delete [] fullContextA;
    }
    if (propertyName) {
        delete [] propertyName;
    }
    if (key != 0) {
        RegCloseKey(key);
    }
    return ret;
}



/*
// DEPRECATED: portal build is now a normal build.
bool OutlookConfig::checkPortalBuild() {
    
    bool ret = false;

    char* portal = readPropertyValue(SOFTWARE_ROOT_CONTEXT, PROPERTY_SP, HKEY_LOCAL_MACHINE);
    if (portal && *portal == '1') {
        ret = true;
    }

    if (portal) delete [] portal;
    return ret;
}
*/


/**
 * Decrypt private data (Username/Password/Proxy username/Proxy password).
 * Data is stored encrypted (B64(DES(data)) since version 6.0.9.
 * Data must be in clear text into the config, as config is used by API.
 * NOTE: data MUST be encrypted in registry, upgrades from v6 or older clients
 *       is no more supported, since v8.7.
 *
 * @return  0 if data decrypted.
 *          1 if error
 */
int OutlookConfig::decryptPrivateData() {

    // Check previous version installed.
    int funambolVersion = getBuildNumberFromVersion(getFunambolSwv().c_str());
    const char* pass_key = NULL;
    
    // Username
    char* decData = decryptData(accessConfig.getUsername(), pass_key);
    if (decData) {
        accessConfig.setUsername(decData);
        delete [] decData; decData = NULL;
    }
    // Password
    decData = decryptData(accessConfig.getPassword(), pass_key);
    if (decData) {
        accessConfig.setPassword(decData);
        delete [] decData; decData = NULL;
    }
    // Proxy username
    decData = decryptData(accessConfig.getProxyUsername(), pass_key);
    if (decData) {
        accessConfig.setProxyUsername(decData);
        delete [] decData; decData = NULL;
    }
    // Proxy password
    decData = decryptData(accessConfig.getProxyPassword(), pass_key);
    if (decData) {
        accessConfig.setProxyPassword(decData);
        delete [] decData; decData = NULL;
    }
    return 0;
}


/**
 * Encrypt private data (Username/Password/Proxy username/Proxy password).
 * Data is stored encrypted (B64(DES(data)) since version 6.0.9.
 * Note:
 * Data must be in clear text into the config, as config is used by API, 
 * so this method should be called only during save() operation.
 */
void OutlookConfig::encryptPrivateData() {

    const char* pass_key = NULL;

    // Username
    char* encData = encryptData(accessConfig.getUsername(), pass_key);
    if (encData) {
        accessConfig.setUsername(encData);
        delete [] encData; encData = NULL;
    }
    // Password
    encData = encryptData(accessConfig.getPassword(), pass_key);
    if (encData) {
        accessConfig.setPassword(encData);
        delete [] encData; encData = NULL;
    }
    // Proxy username
    encData = encryptData(accessConfig.getProxyUsername(), pass_key);
    if (encData) {
        accessConfig.setProxyUsername(encData);
        delete [] encData; encData = NULL;
    }
    // Proxy password
    encData = encryptData(accessConfig.getProxyPassword(), pass_key);
    if (encData) {
        accessConfig.setProxyPassword(encData);
        delete [] encData; encData = NULL;
    }
}

/**
* Load data from the update tree all the configuration parameters.
* It populates also the currentVersion of the UpdateConfig class.
* At the moment "refresh" is not used
*/
BOOL OutlookConfig::readUpdaterConfig(bool refresh) {
    
    return updaterConfig.read();
}

/**
* Save data into the registry
*/
void OutlookConfig::storeUpdaterConfig(){
    
    updaterConfig.save();
}

UpdaterConfig& OutlookConfig::getUpdaterConfig() {
    return updaterConfig;
}

void OutlookConfig::resetUpdaterConfig() {

    updaterConfig.createDefaultConfig();
    updaterConfig.save();
}

void OutlookConfig::setDeviceConfig(const WindowsDeviceConfig & wdc)
{
    WindowsDeviceConfig * temp = winDC;
    winDC = new WindowsDeviceConfig(wdc);
    if (temp)
        delete temp;
}

WindowsDeviceConfig & OutlookConfig::getWindowsDeviceConfig()
{
    return *winDC;
}

WindowsDeviceConfig & OutlookConfig::getDeviceConfig()
{
    return *winDC;
}

/*
 * Save Device Config properties in DMTree.
 * Device properties are placed in 3 nodes under syncML node
 * (DevInfo - DevDetail - Ext)
 *
 * @param n: the 'syncml' node (parent node)
 */
void OutlookConfig::saveDeviceConfig(ManagementNode& n, bool server) {

    if (server) {
        DMTClientConfig::saveDeviceConfig(n, true);
    } else {
        DMTClientConfig::saveDeviceConfig(n, false);

        ManagementNode* node;
        char nodeName[DIM_MANAGEMENT_PATH];

        char syncMLContext[DIM_MANAGEMENT_PATH];
        char* fn = n.createFullName();
        sprintf(syncMLContext, "%s", fn);
        delete [] fn;

        //
        // Ext properties (other misc props)
        //
        sprintf(nodeName, "%s%s", syncMLContext, CONTEXT_EXT);
        node = dmt->readManagementNode(nodeName);
        if (node) {
            char * tmp = new char[10];
            sprintf(tmp, "%i", winDC->getLogNum());
            node->setPropertyValue(PROPERTY_LOG_NUM,tmp);
            delete [] tmp;

            tmp = new char[10];
            sprintf(tmp, "%i", winDC->getLogSize());
            node->setPropertyValue(PROPERTY_LOG_SIZE,tmp);
            delete [] tmp;

            tmp = (winDC->getAttach() ? "1" : "0");
            node->setPropertyValue(PROPERTY_ATTACH,tmp);
            delete node;
            node = NULL;
        }
    }
}

bool OutlookConfig::readDeviceConfig(ManagementNode& n, bool server)
{
    if (server) {
        return DMTClientConfig::readDeviceConfig(n, true);
    } else {

        bool ret = DMTClientConfig::readDeviceConfig(n);
        if (!ret)
            return ret;

        if (winDC)
        {
            delete winDC;
        }
        winDC = new WindowsDeviceConfig(DMTClientConfig::getDeviceConfig());

        char nodeName[DIM_MANAGEMENT_PATH];
        nodeName[0] = 0;
        ManagementNode* node;

        char syncMLContext[DIM_MANAGEMENT_PATH];
        char* fn = n.createFullName();
        sprintf(syncMLContext, "%s", fn);
        delete [] fn;

        //
        // Ext properties (other misc props)
        //
        sprintf(nodeName, "%s%s", syncMLContext, CONTEXT_EXT);
        node = dmt->readManagementNode(nodeName);
        if (node) {
            char * tmp;
            tmp = node->readPropertyValue(PROPERTY_ATTACH);
            winDC->setAttach((*tmp == '1') ? true : false);
            delete [] tmp;

            tmp = node->readPropertyValue(PROPERTY_LOG_NUM);
            int x = atoi(tmp);
            winDC->setLogNum(x);
            delete [] tmp;

            tmp = node->readPropertyValue(PROPERTY_LOG_SIZE);
            x = atoi(tmp);
            winDC->setLogSize(x);
            delete [] tmp;

            delete node;
            node = NULL;
        }
        else {
            return false;
        }
        return true;
    }
}

bool OutlookConfig::addSyncSourceConfig(const char* sourceName) {

    if (!sourceName || !strlen(sourceName)) {
        return false;
    }

    SyncSourceConfig* ssc = DefaultConfigFactory::getSyncSourceConfig(sourceName);
    bool ret = setSyncSourceConfig(*ssc);  // adds if not existing
    delete ssc;

    return ret;
}