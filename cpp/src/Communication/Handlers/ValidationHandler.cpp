#include "ValidationHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../DockingFeature/ValidationPanel.h"

using json = nlohmann::json;

ValidationHandler::ValidationHandler(FDAClient* client)
    : RequestHandler(client)
{
}

void ValidationHandler::validateScript(const std::string& filePath)
{
    Logger::info("[VALIDATION] Preparing request");

    json request;

    request["type"] = "VALIDATE_SCRIPT";
    request["filePath"] = filePath;

    std::string response;

    if (!client->sendRequest(request.dump(), response))
    {
        Logger::error("[VALIDATION] Request failed");
        return;
    }

    Logger::info("[VALIDATION] Response received");

    ValidationResult result = parse(response);

    if (result.noErrors)
    {
        MessageBox(
            NULL,
            TEXT(
                "No Errors or Warnings were detected based on the current validation checks.\n\n"
                "Note: Finacle Dev Assist is currently under development.\n"
                "Additional validation rules and improvements will be added in future updates."
            ),
            TEXT("Finacle Dev Assist"),
            MB_OK | MB_ICONINFORMATION
        );
        return;
    }

    ValidationPanel::showValidationResults(result);
}

ValidationResult ValidationHandler::parse(const std::string& response)
{
    ValidationResult result;

    try
    {
        json data = json::parse(response);

        if (data["STATUS"] != "SUCCESS")
        {
            result.excpOccr = true;
            return result;
        }

        for (const auto& error : data["errors"])
        {
            Issue issue;

            issue.line = error["line"].get<int>();
            issue.message = error["message"].get<std::string>();
            issue.type = IssueType::SCR_ERROR;

            Logger::info(
                std::to_string(issue.line) + " " +
                issue.message +
                " -> ERROR"
            );

            result.errors.push_back(issue);
        }

        for (const auto& warning : data["warnings"])
        {
            Issue issue;

            issue.line = warning["line"].get<int>();
            issue.message = warning["message"].get<std::string>();
            issue.type = IssueType::SCR_WARNING;

            Logger::info(
                std::to_string(issue.line) + " " +
                issue.message +
                " -> WARNING"
            );

            result.warnings.push_back(issue);
        }

        if (result.errors.empty() && result.warnings.empty())
        {
            result.noErrors = true;

            Logger::info("No Errors");
        }

        Logger::info("[VALIDATION] Validated Successfully");
    }
    catch (const std::exception& e)
    {
        Logger::error("[VALIDATION] Parse failed : " + std::string(e.what()));
    }

    return result;
}