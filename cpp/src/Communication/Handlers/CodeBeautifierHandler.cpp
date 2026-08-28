#include "CodeBeautifierHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../Features/ScintillaHelper.h"
#include "../../PluginDefinition.h"

using json = nlohmann::json;

CodeBeautifierHandler::CodeBeautifierHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void CodeBeautifierHandler::beautify(const BeautifyData& data)
{
    Logger::info("[BEAUTIFY] Preparing request");

    json request;

    request["type"] = "BEAUTIFY_CODE";
    request["contentType"] = data.contentType;
    request["content"] = data.content;
    hasSelection = data.hasSelection;

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

        Logger::info("[BEAUTIFY] Response JSON : " + result.dump());

        std::string status = result.value("STATUS", "");

        if (status.empty())
        {
            Logger::error("[BEAUTIFY] STATUS missing");

            MessageBoxA(
                nppData._nppHandle,
                "Unexpected Error Occurred.",
                "Finacle Dev Assist",
                MB_OK | MB_ICONERROR
            );

            return;
        }

        if (status != "SUCCESS")
        {
            std::string exceptionMessage =
                result.value("EXCEPTION", "Unexpected Error Occurred.");

            MessageBoxA(
                nppData._nppHandle,
                "Syntax error was found while beautifying the code.",
                "Finacle Dev Assist",
                MB_OK | MB_ICONERROR
            );

            Logger::error("[BEAUTIFY] " + exceptionMessage);
            return;
        }

        Logger::info("[BEAUTIFY] SUCCESS received");

        if (!result.contains("beautifiedCode"))
        {
            Logger::error("[BEAUTIFY] beautifiedCode missing");

            MessageBoxA(
                nppData._nppHandle,
                "Unexpected Error Occurred.",
                "Finacle Dev Assist",
                MB_OK | MB_ICONERROR
            );

            return;
        }

        if (!result["beautifiedCode"].is_string())
        {
            Logger::error("[BEAUTIFY] beautifiedCode has invalid type : " + std::string(result["beautifiedCode"].type_name()));

            MessageBoxA(
                nppData._nppHandle,
                "Unexpected Error Occurred.",
                "Finacle Dev Assist",
                MB_OK | MB_ICONERROR
            );

            return;
        }

        std::string beautifiedCode = result["beautifiedCode"].get<std::string>();

        if (hasSelection)
        {
            ScintillaHelper::replaceSelectedContent(beautifiedCode);
        }
        else
        {
            ScintillaHelper::replaceCurrentDocument(beautifiedCode);
        }

        Logger::info("[BEAUTIFY] Code Beautified successfully");
    }
    catch (const json::parse_error& e)
    {
        Logger::error("[BEAUTIFY] JSON parse failed : " + std::string(e.what()));
    }
    catch (const json::type_error& e)
    {
        Logger::error("[BEAUTIFY] JSON type error : " + std::string(e.what()));
    }
    catch (const std::exception& e)
    {
        Logger::error("[BEAUTIFY] Unexpected error : " + std::string(e.what()));
    }
}