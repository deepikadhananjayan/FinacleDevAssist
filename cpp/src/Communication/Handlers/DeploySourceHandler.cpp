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

        Logger::info("[DEPLOY] Response JSON : " + result.dump());

        std::string status = result.value("STATUS", "");

        if (status == "NET_CNT_EXCP")
        {
            std::string exceptionMessage =
                result.value("EXCEPTION", "Network connection failed.");

            MessageBoxA(
                nppData._nppHandle,
                exceptionMessage.c_str(),
                "Finacle Dev Assist",
                MB_OK | MB_ICONWARNING
            );

            Logger::error("[DEPLOY] " + exceptionMessage);
            return;
        }

        if (status != "SUCCESS")
        {
            std::string exceptionMessage =
                result.value("EXCEPTION", "Unexpected Error Occurred.");

            MessageBoxA(
                nppData._nppHandle,
                exceptionMessage.c_str(),
                "Finacle Dev Assist",
                MB_OK | MB_ICONERROR
            );

            Logger::error("[DEPLOY] " + exceptionMessage);
            return;
        }

        MessageBoxA(
            nppData._nppHandle,
            "Deployed Successfully.",
            "Finacle Dev Assist",
            MB_OK | MB_ICONINFORMATION
        );

        Logger::info("[DEPLOY] Deployed successfully");
    }
    catch (const json::parse_error& e)
    {
        Logger::error("[DEPLOY] JSON parse failed : " + std::string(e.what()));
    }
    catch (const json::type_error& e)
    {
        Logger::error("[DEPLOY] JSON type error : " + std::string(e.what()));
    }
    catch (const std::exception& e)
    {
        Logger::error("[DEPLOY] Unexpected error : " + std::string(e.what()));
    }
}