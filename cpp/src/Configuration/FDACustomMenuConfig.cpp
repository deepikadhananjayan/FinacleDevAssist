#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <fstream>
#include "FDACustomMenuConfig.h"
#include "../Utils/Logger.h"
#include "../Core/FDAApplication.h"
#include "../nlohmann/json.hpp"
#include "../Utils/StringConvert.h"

std::vector<std::string> FDACustomMenuConfig::menuTypes;
std::vector<std::string> FDACustomMenuConfig::generateDesigns;
std::vector<std::string> FDACustomMenuConfig::fieldPlacements;
std::vector<std::string> FDACustomMenuConfig::fieldTypes;
std::vector<std::string> FDACustomMenuConfig::searchers;

bool FDACustomMenuConfig::load()
{
    wchar_t dllPath[MAX_PATH] = { 0 };
    if (!GetModuleFileNameW(FDAApplication::getModuleHandle(), dllPath, MAX_PATH))
    {
        Logger::error("[CUSTOM_MENU_CONFIG] Unable to determine plugin location");
        return false;
    }

    std::wstring pluginPath(dllPath);
    size_t pos = pluginPath.find_last_of(L'\\');
    if (pos == std::wstring::npos)
    {
        Logger::error("[CUSTOM_MENU_CONFIG] Invalid plugin path");
        return false;
    }
    pluginPath = pluginPath.substr(0, pos + 1);

    // NOTE: filename assumed as customMenuConfig.json — rename here (and in
    // your actual deployed file) if you want a different name.
    std::wstring configFile = pluginPath + L"customMenuConfig.json";
    std::string configPath = wideToUtf8(configFile);

    std::ifstream file(configPath);
    if (!file.is_open())
    {
        Logger::error("[CUSTOM_MENU_CONFIG] customMenuConfig.json not found : " + configPath);
        return false;
    }

    nlohmann::json j;
    try
    {
        file >> j;
    }
    catch (const std::exception& e)
    {
        Logger::error(std::string("[CUSTOM_MENU_CONFIG] JSON parse error : ") + e.what());
        return false;
    }
    file.close();

    menuTypes.clear();
    generateDesigns.clear();
    fieldPlacements.clear();
    fieldTypes.clear();
    searchers.clear();

    if (j.contains("menuType") && j["menuType"].is_array())
        for (const auto& v : j["menuType"]) menuTypes.push_back(v.get<std::string>());

    if (j.contains("generateDesign") && j["generateDesign"].is_array())
        for (const auto& v : j["generateDesign"]) generateDesigns.push_back(v.get<std::string>());

    if (j.contains("fieldPlacement") && j["fieldPlacement"].is_array())
        for (const auto& v : j["fieldPlacement"]) fieldPlacements.push_back(v.get<std::string>());

    if (j.contains("fieldType") && j["fieldType"].is_array())
    {
        for (const auto& v : j["fieldType"])
        {
            std::string t = v.get<std::string>();
            // FILE is handled separately (added conditionally per Menu Type),
            // never part of the base list even if present in the file.
            if (t != "FILE")
                fieldTypes.push_back(t);
        }
    }

    if (j.contains("searcher") && j["searcher"].is_array())
        for (const auto& v : j["searcher"]) searchers.push_back(v.get<std::string>());

    Logger::info("[CUSTOM_MENU_CONFIG] Loaded : " + configPath);
    return true;
}

const std::vector<std::string>& FDACustomMenuConfig::getMenuTypes()       { return menuTypes; }
const std::vector<std::string>& FDACustomMenuConfig::getGenerateDesigns() { return generateDesigns; }
const std::vector<std::string>& FDACustomMenuConfig::getFieldPlacements() { return fieldPlacements; }
const std::vector<std::string>& FDACustomMenuConfig::getFieldTypes()      { return fieldTypes; }
const std::vector<std::string>& FDACustomMenuConfig::getSearchers()       { return searchers; }