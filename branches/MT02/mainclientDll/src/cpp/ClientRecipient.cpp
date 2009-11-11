#include "outlook/ClientRecipient.h"

ClientRecipient::ClientRecipient() {
}

ClientRecipient::ClientRecipient(const std::wstring & namedEmail) {
    parseNamedEmail(namedEmail);
    pSafeRecipient = NULL;
}

ClientRecipient::ClientRecipient(const std::wstring & name, const std::wstring & email) {
    setName(name);
    setEmail(email);
    pSafeRecipient = NULL;
}

ClientRecipient::ClientRecipient(Redemption::ISafeRecipientPtr recPtr) {
    pSafeRecipient = recPtr;
    read();
}

std::wstring ClientRecipient::getName() const {
    return name;
}

std::wstring ClientRecipient::getEmail() const {
    return email;
}

std::wstring ClientRecipient::getNamedEmail() const {
    if (name.length() > 0) {
        return name + L" <" + email + L">";
    } else {
        return email;
    }
}

int ClientRecipient::getStatus() const {
    return status;
}

void ClientRecipient::setName(const std::wstring & value) {
    name = value;
}

void ClientRecipient::setEmail(const std::wstring & value) {
    email = value;
}

bool ClientRecipient::read() {
    name = L"";
    email = L"";
    bool success = false;

    if (pSafeRecipient)
    {
        // The '!(!' is not a mistake.  operator! is overloaded, but bool conversion is not
        if (!(!pSafeRecipient->GetName()) && wcscmp((WCHAR*)pSafeRecipient->GetName(), L"Unknown"))
        {
            if (!(!pSafeRecipient->GetAddress())) {

                if (!(!pSafeRecipient->GetName())) {
                    name = (WCHAR*)pSafeRecipient->GetName();
                }
                email = (WCHAR*)pSafeRecipient->GetAddress();
                success = true;

            } else if (!(!pSafeRecipient->GetName())) {

                // The name can sometimes be the combination of name and email
                std::wstring value = (WCHAR*)pSafeRecipient->GetName();
                success = parseNamedEmail(value);

            }
        }

        //status = pSafeRecipient->MeetingResponseStatus;
        //status = pSafeRecipient->GetMeetingResponseStatus();
        //status = pSafeRecipient->GetTrackingStatus();
    }

    return success;
}

bool ClientRecipient::parseNamedEmail(const std::wstring & namedEmail) {

    bool success = false;

    size_t npos = std::wstring::npos;
    size_t leftParen = namedEmail.find_last_of(L'(');
    size_t rightParen = namedEmail.find_last_of(L')');

    // If its not formatted in the way we expect, we cant handle it
    if (leftParen != npos && rightParen != npos && rightParen == namedEmail.size()-1) {
        name = namedEmail.substr(0, leftParen);
        email = namedEmail.substr(leftParen+1,rightParen-leftParen-1);
        success = true;
    } else {
        size_t leftParen = namedEmail.find_last_of(L'<');
        size_t rightParen = namedEmail.find_last_of(L'>');
        // If its not formatted in the way we expect, we cant handle it
        if (leftParen != npos && rightParen != npos && rightParen == namedEmail.size()-1) {
            name = namedEmail.substr(0, leftParen);
            email = namedEmail.substr(leftParen+1,rightParen-leftParen-1);
            success = true;
        }
    }

    return success;
}
