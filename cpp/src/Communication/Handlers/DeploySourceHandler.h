#pragma once

#include "RequestHandler.h"
#include "../../Models/DeploySourceData.h"

class DeploySourceHandler : public RequestHandler
{
public:
    explicit DeploySourceHandler(FDAClient* client);

    void deploy(const DeploySourceData& data);
    void parse(const std::string& response);
};   