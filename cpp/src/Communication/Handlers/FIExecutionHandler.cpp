#include "FIExecutionHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../Features/ScintillaHelper.h"
#include "../../Notepad_plus_msgs.h"
#include "../../PluginDefinition.h"
#include "../../menuCmdID.h"

using json = nlohmann::json;

FIExecutionHandler::FIExecutionHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void FIExecutionHandler::executeRequest(const FIRequestData& data)
{
    Logger::info("[FI] Preparing request");

    json request;
    json headers;

    headers["Content-Type"] = data.contentType;
    headers["Accept"] = data.accept;

    request["type"] = "EXECUTE_FI_REQUEST";
    request["environment"] = data.environment;
    request["method"] = data.method;
    request["headers"] = headers;
    request["body"] = data.body;

    std::string response;

    if (!client->sendRequest(request.dump(), response))
    {
        Logger::error("[FI] Request failed");
        return;
    }

    Logger::info("[FI] Response received");

    parse(response);

}

void FIExecutionHandler::parse(const std::string& response)
{
    try
    {
        json result = json::parse(response);

        if (result["STATUS"] == "EXCEPTION")
        {
            std::string exceptionMessage = result["EXCEPTION"].get<std::string>();

            MessageBoxA(
                nppData._nppHandle,
                exceptionMessage.c_str(),
                "Finacle Dev Assist",
                MB_OK | MB_ICONWARNING
            );

            Logger::error("[FI] " + exceptionMessage);
            return;
        }

        // Open a new file next to the current tab
        ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

        // Get the handle of the newly opened editor
        HWND hNewEditor = ScintillaHelper::getCurrentEditor();

        if (hNewEditor == nullptr)
        {
            Logger::error("[FI] Failed to get new editor handle.");
            return;
        }

        std::string fiResponse = result["response"].get<std::string>();

        // Set the content into the new file
        ::SendMessage(hNewEditor, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(fiResponse.c_str()));

        // Move cursor to top
        ::SendMessage(hNewEditor, SCI_GOTOPOS, 0, 0);

        Logger::info("[FI] FI Response Showed in New File.");
    }
    catch (const std::exception& e)
    {
        Logger::error("[FI] Parse failed : " + std::string(e.what()));
    }
}