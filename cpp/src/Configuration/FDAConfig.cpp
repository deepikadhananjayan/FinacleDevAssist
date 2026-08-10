#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include "FDAConfig.h"
#include "../Utils/Logger.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include <Windows.h>
#include <fstream>

std::string FDAConfig::javaHost = "127.0.0.1";
int FDAConfig::javaPort = 0;
std::vector<std::string> FDAConfig::fiEnvironments;

bool FDAConfig::load()
{
    wchar_t dllPath[MAX_PATH] = { 0 };

    if (!GetModuleFileNameW(FDAApplication::getModuleHandle(), dllPath, MAX_PATH))
    {
        Logger::error("[CONFIG] Unable to determine plugin location");
        return false;
    }

    std::wstring pluginPath(dllPath);
    size_t pos = pluginPath.find_last_of(L'\\');

    if (pos == std::wstring::npos)
    {
        Logger::error("[CONFIG] Invalid plugin path");
        return false;
    }

    pluginPath = pluginPath.substr(0, pos + 1);
    std::wstring propertyFile = pluginPath + L"fdaplugin.properties";
    std::string propertyFileString = wideToString(propertyFile);

    Logger::info("[CONFIG] Loading : " + propertyFileString);

    std::ifstream file(propertyFileString);

    if (!file.is_open())
    {
        Logger::error("[CONFIG] fdaplugin.properties not found");
        return false;
    }

    Logger::info("[CONFIG] Reading fdaplugin.properties");
    
    fiEnvironments.clear();
    std::string line;
    while (std::getline(file, line))
    {
        if (line.find("java.host=") == 0)
        {
            javaHost = line.substr(10);
        }
        else if (line.find("java.port=") == 0)
        {
            javaPort = std::stoi(line.substr(10));
        }
        else if (line.find("fi.") == 0)
        {
            size_t separator = line.find('=');

            if (separator != std::string::npos)
            {
                std::string environment = line.substr(0, separator);
                fiEnvironments.push_back(environment);

                Logger::info("[CONFIG] FI Environment : " + environment);
            }
        }
    }

    file.close();

    Logger::info("[CONFIG] Host : " + javaHost);
    Logger::info("[CONFIG] Port : " + std::to_string(javaPort));

    return true;
}

std::string FDAConfig::wideToString(const std::wstring& value)
{
    int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        value.c_str(),
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
    );

    if (size <= 0)
        return "";

    std::string result(size - 1, '\0');

    WideCharToMultiByte(
        CP_UTF8,
        0,
        value.c_str(),
        -1,
        &result[0],
        size,
        nullptr,
        nullptr
    );

    return result;
}

std::string FDAConfig::getJavaHost()
{
    return javaHost;
}

int FDAConfig::getJavaPort()
{
    return javaPort;
}

const std::vector<std::string>& FDAConfig::getFIEnvironments()
{
    return fiEnvironments;
}