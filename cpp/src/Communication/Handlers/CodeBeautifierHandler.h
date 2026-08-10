#pragma once

#include <string>
#include "RequestHandler.h"
#include "../../Models/BeautifyData.h"

class CodeBeautifierHandler : public RequestHandler
{
public:
    explicit CodeBeautifierHandler(FDAClient* client);

    void beautify(const BeautifyData& data);
    void parse(const std::string& response);
private:
    bool hasSelection = false;
};