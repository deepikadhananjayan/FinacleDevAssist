#pragma once

#include <string>
#include "RequestHandler.h"
#include "../../Models/FIRequestData.h"

class FIExecutionHandler : public RequestHandler
{
public:
    explicit FIExecutionHandler(FDAClient* client);

    void executeRequest(const FIRequestData& data);
    void parse(const std::string& response);
};