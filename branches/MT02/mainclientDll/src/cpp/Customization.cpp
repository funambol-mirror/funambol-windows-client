
#include "customization.h"

const char * DLLCustomization::updateUrl = "http://sync.emailsrvr.com/update-server/update.php";

// Source defaults
const char * DLLCustomization::sourceDefaultEncoding = "";

const bool   DLLCustomization::sourceNotesDefaultSif    = true;
const char * DLLCustomization::sourceNotesSifUri        = "snote";
const char * DLLCustomization::sourceNotesVnoteUri      = "note";

const bool   DLLCustomization::sourceTasksDefaultSif    = true;
const char * DLLCustomization::sourceTasksSifUri        = "stask";
const char * DLLCustomization::sourceTasksVcalUri       = "task";

const bool   DLLCustomization::sourceCalendarDefaultSif = false;
const char * DLLCustomization::sourceCalendarSifUri     = "sevent";
const char * DLLCustomization::sourceCalendarVcalUri    = "event";

const bool   DLLCustomization::sourceContactsDefaultSif = false;
const char * DLLCustomization::sourceContactsSifUri     = "scard";
const char * DLLCustomization::sourceContactsVcardUri   = "card";