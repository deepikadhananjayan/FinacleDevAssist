#include "GenerateCustomMenuHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../PluginDefinition.h"

using json = nlohmann::json;

GenerateCustomMenuHandler::GenerateCustomMenuHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void GenerateCustomMenuHandler::generateMenu(const CustomMenuData& data)
{
    Logger::info("[C24] Preparing request");

    json request;

    request["type"] = "GENERATE_CUSTOM_MENU";
    request["menuName"] = data.menuName;
    request["menuDescription"] = data.menuDescription;
    request["menuType"] = data.menuType;

    request["generateDesign"] = json::array();

    for (const std::string& design : data.generateDesign)
    {
        request["generateDesign"].push_back(design);
    }

    request["fields"] = json::array();

    for (const Field& field : data.fields)
    {
        json fieldJson;

        fieldJson["pageType"] = field.fieldPlacement;
        fieldJson["type"] = field.fieldType;
        fieldJson["label"] = field.fieldLabel;
        fieldJson["id"] = field.fieldId;
        fieldJson["description"] = field.fieldDescription;
        fieldJson["searcher"] = field.searcher;
        fieldJson["disabled"] = field.isDisabled;
        fieldJson["mandatory"] = field.isMandatory;

        fieldJson["options"] = json::array();

        for (const Option& option : field.options)
        {
            json optionJson;

            optionJson["value"] = option.value;
            optionJson["label"] = option.label;

            fieldJson["options"].push_back(optionJson);
        }

        request["fields"].push_back(fieldJson);
    }

    Logger::info("[C24] Request : " + request.dump());

    std::string response;

    if (!client->sendRequest(request.dump(), response))
    {
        Logger::error("[C24] Failed to send request");
        return;
    }

    Logger::info("[C24] Response : " + response);

    parse(response);
}

void GenerateCustomMenuHandler::parse(const std::string& response)
{
    try
    {
        json result = json::parse(response);

        if (result["STATUS"] != "SUCCESS") {

            MessageBox(
                nppData._nppHandle,
                TEXT("Unexpected Error Occured."),
                TEXT("Finacle Dev Assist"),
                MB_OK | MB_ICONINFORMATION
            );

            Logger::error("[C24] " + result["EXCEPTION"]);
            return;
        }
    }
    catch (const std::exception& e)
    {
        Logger::error("[C24] Parse failed : " + std::string(e.what()));
    }
}