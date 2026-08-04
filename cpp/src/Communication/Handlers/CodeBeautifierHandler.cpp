#include "CodeBeautifierHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../Features/ScintillaHelper.h"

using json = nlohmann::json;

CodeBeautifierHandler::CodeBeautifierHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void CodeBeautifierHandler::beautify(const std::string& filePath)
{
    Logger::info("[BEAUTIFY] Preparing request");

    json request;

    request["type"] = "BEAUTIFY_CODE";
    request["filePath"] = filePath;

    std::string response;

    if (!client->sendRequest(request.dump(), response))
    {
        Logger::error("[BEAUTIFY] Request failed");
        return;
    }

    Logger::info("[BEAUTIFY] Response received");

    parse(response);

}

void CodeBeautifierHandler::parse(const std::string& response)
{
    try
    {
        json result = json::parse(response);

        if (result["STATUS"] != "SUCCESS") {

            MessageBox(
                NULL,
                TEXT("Unexpected Error Occured."),
                TEXT("Finacle Dev Assist"),
                MB_OK | MB_ICONINFORMATION
            );

            Logger::error("[BEAUTIFY] " + result["EXCEPTION"]);
            return;
        }

        ScintillaHelper::replaceCurrentDocument(result["beautifiedCode"]);
        
        Logger::info("[BEAUTIFY] Code Beautified successfully");
    }
    catch (const std::exception& e)
    {
        Logger::error("[BEAUTIFY] Parse failed : " + std::string(e.what()));
    }
}