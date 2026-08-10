#pragma once

#include <string>

struct FIRequestData
{
    std::string environment;
    std::string method;
    std::string contentType;
    std::string accept;
    std::string body;
};