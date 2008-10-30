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

#ifndef INCL_CLIENTTASK
#define INCL_CLIENTTASK

/** @cond OLPLUGIN */
/** @addtogroup outlook_items */
/** @{ */

#include "outlook/defs.h"
#include "outlook/ClientItem.h"
#include "outlook/ClientRecurrence.h"

#include <string>



/**
 * Wraps the Outlook task object.
 * Implements methods to get/set specific properties for this 
 * object type.
 * Contains a ClientRecurrence object that describes the
 * recurrence pattern.
 */
class ClientTask : public ClientItem {

private:

    /// Pointer to microsoft outlook objects.
    _TaskItemPtr  pTask;

    /// Pointer to Redemption safe objects.
    Redemption::ISafeTaskItemPtr pSafeTask;

    /// Internal Object: to manage recurrence pattern.
    ClientRecurrence recPattern;


    void createSafeTaskInstance();
    void initializeRecPattern();


    bool isSecureProperty (const std::wstring& propertyName);
    bool isComplexProperty(const std::wstring& propertyName);

    const std::wstring getSafeProperty      (const std::wstring& propertyName);
    const std::wstring getComplexProperty   (const std::wstring& propertyName);

    int setComplexProperty   (const std::wstring& propertyName, const std::wstring& propertyValue);

public:

    /// Constructor
    ClientTask();
    ClientTask(const ClientTask& c);
    ClientTask operator=(const ClientTask& c);

    /// Destructor
    ~ClientTask();


    /// Set a COM pointer to this object: refresh all members.
    void setCOMPtr(_TaskItemPtr& ptr, const std::wstring& itemID);
    void setCOMPtr(_TaskItemPtr& ptr);

    /// Returns a reference to the internal COM pointer.
    _TaskItemPtr& getCOMPtr();
    
    /// Returns a pointer to the internal ClientRecurrence object (NULL if not recurring).
    ClientRecurrence* getRecPattern();

    //
    // Client operations on the item.
    //
    int saveItem();
    int deleteItem();
    ClientItem* copyItem();
    //int moveItem(ClientFolder* destFolder);

};

/** @} */
/** @endcond */
#endif
