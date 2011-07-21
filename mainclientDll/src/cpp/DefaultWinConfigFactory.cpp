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

#include "base/util/utils.h"
#include "winmaincpp.h"
#include "customization.h"
#include "DefaultWinConfigFactory.h"
#include "DateFilter.h"
#include <string>

#include "sapi/SapiSyncSource.h"
#include "sapi/FileSapiSyncSource.h"
#include "utils.h"

using namespace std;


DefaultWinConfigFactory::DefaultWinConfigFactory() : DefaultConfigFactory() {
}

DefaultWinConfigFactory::~DefaultWinConfigFactory() {
}


AccessConfig* DefaultWinConfigFactory::getAccessConfig() {

    AccessConfig* ac = new AccessConfig();

    ac->setUsername             (DEFAULT_USERNAME);
    ac->setPassword             (DEFAULT_PASSWORD);
    ac->setSyncURL              (DEFAULT_URL);
    ac->setFirstTimeSyncMode    (SYNC_NONE);
    ac->setUseProxy             (FALSE);
    ac->setProxyHost            ("");
    ac->setProxyPort            (8080);
    ac->setProxyUsername        ("");
    ac->setProxyPassword        ("");
    ac->setBeginSync            (0);
    ac->setEndSync              (0);
    ac->setServerAuthRequired   (FALSE);
    ac->setClientAuthType       ("syncml:auth-basic");
    ac->setServerAuthType       ("syncml:auth-basic");
    ac->setServerPWD            ("funambol");
    ac->setServerID             ("funambol");
    ac->setServerNonce          ("");
    ac->setClientNonce          ("");
    ac->setMaxMsgSize           (MAX_SYNCML_MSG_SIZE);                  // 125kB = ~50 contacts
    ac->setReadBufferSize       (0);
    ac->setUserAgent            (PROGRAM_NAME);                         // This is replaced during config.upgradeConfig()
    ac->setCheckConn            (FALSE);
    ac->setResponseTimeout      (RESPONSE_TIMEOUT);                     // [seconds] timeout on server = 15 min
	ac->setCompression			(ENABLE_COMPRESSION);
    return ac;
}

DeviceConfig* DefaultWinConfigFactory::getDeviceConfig() {

    DeviceConfig* dc = new DeviceConfig();
    dc->setMan                  ("Funambol");
    dc->setMod                  (PROGRAM_NAME);
    dc->setOem                  ("");
    dc->setFwv                  ("");
    dc->setSwv                  ("");                                   // This is replaced during config.upgradeConfig()
    dc->setHwv                  ("");
    dc->setDevID                ("fol-default");                        // This is generated unique with config.setUniqueDevID()
    dc->setDevType              ("workstation");
    dc->setDsV                  ("");
    dc->setUtc                  (TRUE);
    dc->setLoSupport            (TRUE);
    dc->setNocSupport           (TRUE);
    dc->setLogLevel             (LOG_LEVEL_INFO);
    dc->setMaxObjSize           (0);
    dc->setDevInfHash           ("");

	dc->setAutoSync				  (DEFAULT_AUTO_SYNC);			        // auto-sync parameter from server (V.10+)
	dc->setDataplanExpirationDate (0L);									// dataplan expiration date parameter from server (V.10+)
	dc->setNetworkWarning         (false);		                        // network warning does not make sense on desktop clients

    return dc;
}

WindowsDeviceConfig* DefaultWinConfigFactory::getWindowsDeviceConfig(DeviceConfig & dc) {

    WindowsDeviceConfig* wdc = new WindowsDeviceConfig(dc);

    wdc->setLogNum               (10);
    wdc->setLogSize              (1);
    wdc->setAttach               (false);

    return wdc;
}


SapiConfig* DefaultWinConfigFactory::getSapiConfig() {

    SapiConfig* c = new SapiConfig();

    c->setRequestTimeout        (SAPI_HTTP_REQUEST_TIMEOUT);      // 30 sec
    c->setResponseTimeout       (SAPI_HTTP_RESPONSE_TIMEOUT);     // 30 sec
    c->setUploadChunkSize       (SAPI_HTTP_UPLOAD_CHUNK_SIZE);    // 30 KByte
    c->setDownloadChunkSize     (SAPI_HTTP_DOWNLOAD_CHUNK_SIZE);  // 30 KByte
    c->setMaxRetriesOnError     (SAPI_MAX_RETRY_ON_ERROR);        // retry 2 times if network error
    c->setSleepTimeOnRetry      (SAPI_SLEEP_TIME_ON_RETRY);       // wait 500 millisec before retry
    c->setMinDataSizeOnRetry    (SAPI_MIN_DATA_SIZE_ON_RETRY);    // 10 KBytes

    return c;
}


SyncSourceConfig* DefaultWinConfigFactory::getSyncSourceConfig(const wstring& wname) {

    SyncSourceConfig* sc = new SyncSourceConfig();
    char* name = toMultibyte(wname.c_str());

    sc->setName                 (name);
    sc->setEncoding             (DLLCustomization::sourceDefaultEncoding);
    sc->setLast                 (0);
    sc->setEncryption           ("");

    if (wname == CONTACT) {
        sc->setSync             (DEFAULT_CONTACTS_SYNC_MODE);
        sc->setSyncModes        (CONTACTS_DEVINFO_SYNC_MODES);
        // since 7.1.2: default is vCard. Both still supported (for backw compaibility).
        sc->setURI              (DLLCustomization::sourceContactsVcardUri);
        sc->setType             ("text/x-vcard");
        sc->setVersion          ("2.1");
        sc->setSupportedTypes   ("text/x-vcard:2.1,text/x-s4j-sifc:1.0");
        sc->setIsEnabled        (CONTACT_SOURCE_ENABLED);
		sc->setIsAllowed		(CONTACT_SOURCE_ALLOWED); // allowed param (v.10+)
    }
    else if (wname == APPOINTMENT) {
        sc->setSync             (DEFAULT_APPOINTMENTS_SYNC_MODE);
        sc->setSyncModes        (APPOINTMENTS_DEVINFO_SYNC_MODES);
        sc->setURI              (DLLCustomization::sourceCalendarVcalUri);
        sc->setType             ("text/x-vcalendar");
        sc->setVersion          ("1.0");
        sc->setSupportedTypes   ("text/x-vcalendar:1.0,text/x-s4j-sife:1.0");
        sc->setIsEnabled        (APPOINTMENT_SOURCE_ENABLED);
		sc->setIsAllowed		(APPOINTMENT_SOURCE_ALLOWED); // allowed param (v.10+)
    }
    else if (wname == TASK) {
        sc->setSync             (DEFAULT_TASKS_SYNC_MODE);
        sc->setSyncModes        (TASKS_DEVINFO_SYNC_MODES);
        sc->setURI              (DLLCustomization::sourceTasksVcalUri);
        sc->setType             ("text/x-vcalendar");
        sc->setVersion          ("1.0");
        sc->setSupportedTypes   ("text/x-vcalendar:1.0,text/x-s4j-sift:1.0");
        sc->setIsEnabled        (TASK_SOURCE_ENABLED);
		sc->setIsAllowed		(TASK_SOURCE_ALLOWED); // allowed param (v.10+)
    }
    else if (wname == NOTE) {
        sc->setSync             (DEFAULT_NOTES_SYNC_MODE);
        sc->setSyncModes        (NOTES_DEVINFO_SYNC_MODES);
        if (DLLCustomization::sourceNotesDefaultSif) {
            sc->setURI              (DLLCustomization::sourceNotesSifUri);
            sc->setType             ("text/x-s4j-sifn");
            sc->setVersion          ("1.0");
            sc->setEncoding         ("b64");
            sc->setSupportedTypes   ("text/x-s4j-sifn:1.0,text/x-vnote:1.1");
        } else {
            sc->setURI              (DLLCustomization::sourceNotesVnoteUri);
            sc->setType             ("text/x-vnote");
            sc->setVersion          ("1.0");
            sc->setSupportedTypes   ("text/x-vnote:1.1");
        }
        sc->setIsEnabled        (NOTE_SOURCE_ENABLED);
		sc->setIsAllowed		(NOTE_SOURCE_ALLOWED); // allowed param (v.10+)
    }

    // SAPI
    else if (wname == PICTURE) {
        sc->setSync             (DEFAULT_PICTURES_SYNC_MODE);
        sc->setSyncModes        (PICTURES_DEVINFO_SYNC_MODES);
        sc->setURI              (DLLCustomization::sourcePicturesUri);
        sc->setType             ("image/*");      
        sc->setVersion          ("");
        sc->setEncoding         ("bin");                                 // not really used, as it's detected from each item received
        sc->setSupportedTypes   ("application/*");
        sc->setIsEnabled        (PICTURE_SOURCE_ENABLED);
		sc->setIsAllowed        (PICTURE_SOURCE_ALLOWED);				// allowed param (v.10+)
        sc->setProperty         (PROPERTY_DOWNLOAD_LAST_TIME_STAMP,     "0");
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_CLIENT, -1);
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_SERVER, -1);
        sc->setProperty         (PROPERTY_EXTENSION,                     PICT_EXTENSION);
        sc->setProperty         (PROPERTY_MEDIAHUB_PATH,                 "");  
        sc->setProperty         (PROPERTY_LOCAL_QUOTA_STORAGE,           SAPI_LOCAL_QUOTA_STORAGE);
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_MAX_SIZE,           SAPI_MAX_PICTURE_SIZE);   // 0 = unlimited
    }
    else if (wname == VIDEO){
        sc->setSync             (DEFAULT_VIDEOS_SYNC_MODE);
        sc->setSyncModes        (VIDEOS_DEVINFO_SYNC_MODES);
        sc->setURI              (DLLCustomization::sourceVideosUri);
        sc->setType             ("video/*");      
        sc->setVersion          ("");
        sc->setEncoding         ("bin");                                 // not really used, as it's detected from each item received
        sc->setSupportedTypes   ("application/*");
        sc->setIsEnabled        (VIDEO_SOURCE_ENABLED);
		sc->setIsAllowed        (VIDEO_SOURCE_ALLOWED);
        sc->setProperty         (PROPERTY_DOWNLOAD_LAST_TIME_STAMP,     "0");
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_CLIENT, -1);
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_SERVER, -1);
        sc->setProperty         (PROPERTY_EXTENSION,                    VIDEO_EXTENSION);
        sc->setProperty         (PROPERTY_MEDIAHUB_PATH,                "");  
        sc->setProperty         (PROPERTY_LOCAL_QUOTA_STORAGE,          SAPI_LOCAL_QUOTA_STORAGE);
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_MAX_SIZE,           SAPI_MAX_VIDEO_SIZE);   // 100 MB
    }
    else if (wname == FILES){
        sc->setSync             (DEFAULT_FILES_SYNC_MODE);
        sc->setSyncModes        (FILES_DEVINFO_SYNC_MODES);
        sc->setURI              (DLLCustomization::sourceFilesUri);
        sc->setType             ("application/*");      
        sc->setVersion          ("");
        sc->setEncoding         ("bin");                                 // not really used, as it's detected from each item received
        sc->setSupportedTypes   ("application/*");
        sc->setIsEnabled        (FILE_SOURCE_ENABLED); 
		sc->setIsAllowed        (FILE_SOURCE_ALLOWED); 
        sc->setProperty         (PROPERTY_DOWNLOAD_LAST_TIME_STAMP,     "0");
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_CLIENT, -1);
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_NUMBER_FROM_SERVER, -1);
        sc->setProperty         (PROPERTY_EXTENSION,                    FILE_EXTENSION);
        sc->setProperty         (PROPERTY_MEDIAHUB_PATH,                "");  
        sc->setProperty         (PROPERTY_LOCAL_QUOTA_STORAGE,          SAPI_LOCAL_QUOTA_STORAGE);
        sc->setIntProperty      (PROPERTY_SYNC_ITEM_MAX_SIZE,           SAPI_MAX_FILE_SIZE);   // 0 = unlimited
    }

    if (name) delete [] name;
    return sc;
}




WindowsSyncSourceConfig* DefaultWinConfigFactory::getWinSyncSourceConfig(const wstring& wname, SyncSourceConfig* sc) {

    WindowsSyncSourceConfig* wsc = new WindowsSyncSourceConfig(sc);

    StringBuffer name;
    name.convert(wname.c_str());
    if (isPIMSource(name.c_str())) {
        // Only PIM!
        wsc->setUseSubfolders(DLLCustomization::defaultUseSubfolders);
        wsc->setFolderPath("");
    }

    wsc->setEndTimestamp(0);

    // Date filtering, set defaults.
    if (wname == APPOINTMENT) {
        DateFilter& filter = wsc->getDateFilter();
        filter.setDirection(DateFilter::DIR_OUT);
        filter.setRelativeLowerDate(DateFilter::LAST_MONTH);
        filter.setUpperDate(NULL);
    }

    return wsc;
}

