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

        Logger::info("[TOKEN] Response JSON : " + result.dump());

        if (!result.contains("STATUS"))
        {
            Logger::error("[TOKEN] STATUS missing");
            return;
        }

        Logger::info("[TOKEN] STATUS type : " + std::string(result["STATUS"].type_name()));

        if (!result["STATUS"].is_string())
        {
            Logger::error("[TOKEN] STATUS has unexpected type : " + std::string(result["STATUS"].type_name()));
            return;
        }

        if (result["STATUS"] != "SUCCESS")
        {
            MessageBox(
                nppData._nppHandle,
                TEXT("Unexpected Error Occurred."),
                TEXT("Finacle Dev Assist"),
                MB_OK | MB_ICONINFORMATION
            );

            if (result.contains("EXCEPTION"))
            {
                if (result["EXCEPTION"].is_string())
                {
                    std::string exceptionMessage = result["EXCEPTION"].get<std::string>();
                    Logger::error("[TOKEN] " + exceptionMessage);
                }
                else
                {
                    Logger::error("[TOKEN] EXCEPTION has unexpected type : " + std::string(result["EXCEPTION"].type_name()));
                }
            }
            else
            {
                Logger::error("[TOKEN] EXCEPTION missing");
            }

            return;
        }

        Logger::info("[TOKEN] SUCCESS received");

        if (!result.contains("keywords"))
        {
            Logger::error("[TOKEN] keywords missing");
            return;
        }

        if (!result["keywords"].is_array())
        {
            Logger::error("[TOKEN] keywords has unexpected type : " + std::string(result["keywords"].type_name()));
            return;
        }

        std::vector<std::string> keywords;

        for (const auto& item : result["keywords"])
        {
            if (!item.is_string())
            {
                Logger::error("[TOKEN] Invalid keyword type : " + std::string(item.type_name()));
                continue;
            }

            keywords.push_back(item.get<std::string>());
        }

        if (!result.contains("functions"))
        {
            Logger::error("[TOKEN] functions missing");
            return;
        }

        if (!result["functions"].is_array())
        {
            Logger::error("[TOKEN] functions has unexpected type : " + std::string(result["functions"].type_name()));
            return;
        }

        std::vector<std::string> functions;

        for (const auto& item : result["functions"])
        {
            if (!item.is_string())
            {
                Logger::error("[TOKEN] Invalid function type : " + std::string(item.type_name()));
                continue;
            }

            functions.push_back(item.get<std::string>());
        }

        if (!result.contains("userhooks"))
        {
            Logger::error("[TOKEN] userhooks missing");
            return;
        }

        if (!result["userhooks"].is_array())
        {
            Logger::error("[TOKEN] userhooks has unexpected type : " + std::string(result["userhooks"].type_name()));
            return;
        }

        std::vector<std::string> userHooks;

        for (const auto& item : result["userhooks"])
        {
            if (!item.is_string())
            {
                Logger::error("[TOKEN] Invalid userhook type : " + std::string(item.type_name()));
                continue;
            }

            userHooks.push_back(item.get<std::string>());
        }

        TokenStore::setKeywords(keywords);
        TokenStore::setFunctions(functions);
        TokenStore::setUserHooks(userHooks);

        Logger::info("[TOKEN] Stored successfully");
    }
    catch (const json::parse_error& e)
    {
        Logger::error("[TOKEN] JSON parse failed : " + std::string(e.what()));
    }
    catch (const json::type_error& e)
    {
        Logger::error("[TOKEN] JSON type error : " + std::string(e.what()));
    }
    catch (const std::exception& e)
    {
        Logger::error("[TOKEN] Unexpected error : " + std::string(e.what()));
    }
}