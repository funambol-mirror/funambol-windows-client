
#include "customization.h"

const char * DLLCustomization::updateUrl = "http://sync.emailsrvr.com/update-server/update.php";

// Source defaults
const char * DLLCustomization::sourceDefaultEncoding = "";

const bool   DLLCustomization::sourceNotesDefaultSif    = false;
const char * DLLCustomization::sourceNotesSifUri        = "snote";
const char * DLLCustomization::sourceNotesVnoteUri      = "notes-shared";

const bool   DLLCustomization::sourceTasksDefaultSif    = false;
const char * DLLCustomization::sourceTasksSifUri        = "stask";
const char * DLLCustomization::sourceTasksVcalUri       = "tasks-shared";

const bool   DLLCustomization::sourceCalendarDefaultSif = false;
const char * DLLCustomization::sourceCalendarSifUri     = "snote";
const char * DLLCustomization::sourceCalendarVcalUri    = "calendar-shared";

const bool   DLLCustomization::sourceContactsDefaultSif = false;
const char * DLLCustomization::sourceContactsSifUri     = "snote";
const char * DLLCustomization::sourceContactsVcardUri   = "contacts-shared";