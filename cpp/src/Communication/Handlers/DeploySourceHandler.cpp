#include "DeploySourceHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../PluginDefinition.h"
#include "../../Models/DeploySourceData.h"

using json = nlohmann::json;

DeploySourceHandler::DeploySourceHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void DeploySourceHandler::deploy(const DeploySourceData& data)
{
    Logger::info("[DEPLOY] Preparing request");

    json request;

    request["type"] = "DEPLOY_CUSTOM_MENU";
    request["filesPath"] = data.folderPath;
    request["environment"] = "c24." + data.environmentName;

    std::string response;

    if (!client->sendRequest(request.dump(), response))
    {
        Logger::error("[DEPLOY] Request failed");
        return;
    }

    Logger::info("[DEPLOY] Response received");

    parse(response);
}

void DeploySourceHandler::parse(const std::string& response)
{
    try
    {
        json result = json::parse(response);

        if (result["STATUS"] == "NET_CNT_EXCP")
        {
            std::string exceptionMessage = result["EXCEPTION"].get<std::string>();

            MessageBoxA(
                nppData._nppHandle,
                exceptionMessage.c_str(),
                "Finacle Dev Assist",
                MB_OK | MB_ICONWARNING
            );

            Logger::error("[DEPLOY] " + exceptionMessage);
            return;
        }
        
        if (result["STATUS"] != "SUCCESS")
        {
            MessageBox(
                nppData._nppHandle,
                TEXT("Unexpected Error Occured."),
                TEXT("Finacle Dev Assist"),
                MB_OK | MB_ICONINFORMATION
            );

            Logger::error("[DEPLOY] " + result["EXCEPTION"]);
            return;
        }

        MessageBox(
            nppData._nppHandle,
            TEXT("Deployed Successfully."),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );

        Logger::info("[DEPLOY] Deployed successfully");
    }
    catch (const std::exception& e)
    {
        Logger::error("[DEPLOY] Parse failed : " + std::string(e.what()));
    }
}