#pragma once

#include "RequestHandler.h"
#include <string>

class CodeBeautifierHandler : public RequestHandler
{
public:
    explicit CodeBeautifierHandler(FDAClient* client);

    void beautify(const std::string& filePath);
    void parse(const std::string& response);
};