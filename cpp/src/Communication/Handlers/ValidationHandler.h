#pragma once

#include "RequestHandler.h"
#include "../../Models/ValidationResult.h"
#include <string>

class ValidationHandler : public RequestHandler
{
public:
    explicit ValidationHandler(FDAClient* client);
    
    void validateScript(const std::string& filePath);
    ValidationResult parse(const std::string& response);
};