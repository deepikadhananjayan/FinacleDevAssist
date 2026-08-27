#include "ValidationHandler.h"
#include "../FDAClient.h"
#include "../../Utils/Logger.h"
#include "../../nlohmann/json.hpp"
#include "../../DockingFeature/ValidationPanel.h"
#include "../../PluginDefinition.h"

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
        Logger::info("[VALIDATION] Clearing the Previous Errors, if no errors found");
        ValidationPanel::hidePanel();

        MessageBox(
            nppData._nppHandle,
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

        Logger::info("[VALIDATION] Response JSON : " + data.dump());

        if (!data.contains("STATUS"))
        {
            Logger::error("[VALIDATION] STATUS missing");
            result.excpOccr = true;
            return result;
        }

        Logger::info("[VALIDATION] STATUS type : " + std::string(data["STATUS"].type_name()));

        if (!data["STATUS"].is_string())
        {
            Logger::error("[VALIDATION] STATUS has unexpected type : " + std::string(data["STATUS"].type_name()));
            result.excpOccr = true;
            return result;
        }

        if (data["STATUS"] != "SUCCESS")
        {
            result.excpOccr = true;

            if (data.contains("EXCEPTION"))
            {
                if (data["EXCEPTION"].is_string())
                {
                    std::string exceptionMessage = data["EXCEPTION"].get<std::string>();
                    Logger::error("[VALIDATION] " + exceptionMessage);
                }
                else
                {
                    Logger::error("[VALIDATION] EXCEPTION has unexpected type : " + std::string(data["EXCEPTION"].type_name()));
                }
            }
            else
            {
                Logger::error("[VALIDATION] EXCEPTION missing");
            }

            return result;
        }

        Logger::info("[VALIDATION] SUCCESS received");

        if (data.contains("errors"))
        {
            if (!data["errors"].is_array())
            {
                Logger::error("[VALIDATION] errors has unexpected type : " + std::string(data["errors"].type_name()));
                result.excpOccr = true;
                return result;
            }

            for (const auto& error : data["errors"])
            {
                if (!error.contains("line") || !error.contains("message"))
                {
                    Logger::error("[VALIDATION] Invalid error entry");
                    continue;
                }

                if (!error["line"].is_number_integer())
                {
                    Logger::error("[VALIDATION] Error line has unexpected type : " + std::string(error["line"].type_name()));
                    continue;
                }

                if (!error["message"].is_string())
                {
                    Logger::error("[VALIDATION] Error message has unexpected type : " + std::string(error["message"].type_name()));
                    continue;
                }

                Issue issue;
                issue.line = error["line"].get<int>();
                issue.message = error["message"].get<std::string>();
                issue.type = IssueType::SCR_ERROR;

                result.errors.push_back(issue);
            }
        }
        else
        {
            Logger::info("[VALIDATION] errors field missing");
        }

        if (data.contains("warnings"))
        {
            if (!data["warnings"].is_array())
            {
                Logger::error("[VALIDATION] warnings has unexpected type : " + std::string(data["warnings"].type_name()));
                result.excpOccr = true;
                return result;
            }

            for (const auto& warning : data["warnings"])
            {
                if (!warning.contains("line") || !warning.contains("message"))
                {
                    Logger::error("[VALIDATION] Invalid warning entry");
                    continue;
                }

                if (!warning["line"].is_number_integer())
                {
                    Logger::error("[VALIDATION] Warning line has unexpected type : " + std::string(warning["line"].type_name()));
                    continue;
                }

                if (!warning["message"].is_string())
                {
                    Logger::error("[VALIDATION] Warning message has unexpected type : " + std::string(warning["message"].type_name()));
                    continue;
                }

                Issue issue;
                issue.line = warning["line"].get<int>();
                issue.message = warning["message"].get<std::string>();
                issue.type = IssueType::SCR_WARNING;

                result.warnings.push_back(issue);
            }
        }
        else
        {
            Logger::info("[VALIDATION] warnings field missing");
        }

        if (result.errors.empty() && result.warnings.empty())
        {
            result.noErrors = true;
            Logger::info("[VALIDATION] No Errors or Warnings");
        }

        Logger::info("[VALIDATION] Validated Successfully");
    }
    catch (const json::parse_error& e)
    {
        Logger::error("[VALIDATION] JSON parse failed : " + std::string(e.what()));
    }
    catch (const json::type_error& e)
    {
        Logger::error("[VALIDATION] JSON type error : " + std::string(e.what()));
    }
    catch (const std::exception& e)
    {
        Logger::error("[VALIDATION] Unexpected error : " + std::string(e.what()));
    }

    return result;
}