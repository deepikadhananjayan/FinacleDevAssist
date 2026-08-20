#pragma once

#include <string>
#include "RequestHandler.h"
#include "../../Models/CustomMenuModel.h"

class GenerateCustomMenuHandler : public RequestHandler
{
public:
    explicit GenerateCustomMenuHandler(FDAClient* client);

    void generateMenu(const CustomMenuData& data);
    void parse(const std::string& response);
};