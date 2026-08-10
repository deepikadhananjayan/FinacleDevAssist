#include "TokenHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../Core/TokenStore.h"
#include "../../nlohmann/json.hpp"
#include "../../PluginDefinition.h"

using json = nlohmann::json;

TokenHandler::TokenHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void TokenHandler::getTokens()
{
    Logger::info("[TOKEN] Preparing request");

    json request;
    
    request["type"] = "GET_KEYWORDS_AND_USERHOOKS";

    std::string response;

    if (!client->sendRequest(request.dump(), response))
    {
        Logger::error("[TOKEN] Request failed");
        return;
    }

    Logger::info("[TOKEN] Response received");

    parse(response);

}

void TokenHandler::parse(const std::string& response)
{
    try
    {
        json result = json::parse(response);

        if (result["STATUS"] != "SUCCESS"){

            MessageBox(
                nppData._nppHandle,
                TEXT("Unexpected Error Occured."),
                TEXT("Finacle Dev Assist"),
                MB_OK | MB_ICONINFORMATION
            );

            Logger::error("[TOKEN] " + result["EXCEPTION"]);
            return;
        }

        std::vector<std::string> keywords;
        for (auto& item : result["keywords"])
        {
            keywords.push_back(item.get<std::string>());
        }

        std::vector<std::string> functions;
        for (auto& item : result["functions"])
        {
            functions.push_back(item.get<std::string>());
        }

        std::vector<std::string> userHooks;
        for (auto& item : result["userhooks"])
        {
            userHooks.push_back(item.get<std::string>());
        }

        TokenStore::setKeywords(keywords);
        TokenStore::setFunctions(functions);
        TokenStore::setUserHooks(userHooks);

        Logger::info("[TOKEN] Stored successfully");
    }
    catch (const std::exception& e)
    {
        Logger::error("[TOKEN] Parse failed : " + std::string(e.what()));
    }
}