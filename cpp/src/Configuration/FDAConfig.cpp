#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_
#include "FDAConfig.h"
#include "../Utils/Logger.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <shlobj.h>
#include "../Utils/FDACrypto.h"
#pragma comment(lib, "shell32.lib")

// -------------------------------------------------------
// Statics
// -------------------------------------------------------
std::string              FDAConfig::propertyFilePath;
std::string              FDAConfig::javaHost = "127.0.0.1";
int                      FDAConfig::javaPort = 0;
std::vector<std::string> FDAConfig::fiEnvironments;
std::vector<FDAProperty> FDAConfig::properties;

// -------------------------------------------------------
// C24 environment key helpers
// -------------------------------------------------------
static const char* kC24Fields[] = {
    "host", "port", "username", "password", "bankId", "bePath", "fePath"
};

bool FDAConfig::isC24Field(const std::string& field)
{
    for (const char* f : kC24Fields)
        if (field == f) return true;
    return false;
}

bool FDAConfig::isC24EnvKey(const std::string& key, std::string& outName, std::string& outField)
{
    const std::string prefix = "c24.";
    if (key.rfind(prefix, 0) != 0) return false;

    std::string rest = key.substr(prefix.length());
    size_t lastDot = rest.find_last_of('.');
    if (lastDot == std::string::npos) return false;

    outName = rest.substr(0, lastDot);
    outField = rest.substr(lastDot + 1);

    if (outName.empty()) return false;
    if (!isC24Field(outField)) return false;

    return true;
}

// -------------------------------------------------------
// Permission rules — all centralized here
// -------------------------------------------------------
void FDAConfig::setPropertyPermissions(FDAProperty& prop)
{
    if (prop.key == "java.host" || prop.key == "java.port")
    {
        prop.editable = false;
        prop.deletable = false;
        return;
    }

    if (prop.key.rfind("fi.", 0) == 0)
    {
        prop.editable = true;
        prop.deletable = true;
        return;
    }

    if (prop.key == "c24.output.dir")
    {
        prop.editable = true;
        prop.deletable = false;
        return;
    }

    if (isC24EnvironmentKey(prop.key))
    {
        prop.editable = true;
        prop.deletable = true;
        return;
    }

    // Unknown / future properties default to read-only
    prop.editable = false;
    prop.deletable = false;
}

bool FDAConfig::isC24EnvironmentKey(const std::string& key)
{
    std::string name, field;
    return isC24EnvKey(key, name, field);
}

// -------------------------------------------------------
// Protected key check
// -------------------------------------------------------
bool FDAConfig::isProtected(const std::string& key)
{
    return (key == "java.host" || key == "java.port");
}

// -------------------------------------------------------
// Validation
// -------------------------------------------------------
int FDAConfig::validateKey(const std::string& key)
{
    if (key.find('=') != std::string::npos)
    {
        Logger::error("[CONFIG] Invalid key — must not contain '=' : " + key);
        return 3;
    }

    return 0;
}

bool FDAConfig::validateValue(const std::string& value)
{
    if (value.empty())
    {
        Logger::error("[CONFIG] Value must not be empty");
        return false;
    }

    return true;
}

static std::wstring getUserFdaDir()
{
    PWSTR path = nullptr;
    std::wstring result;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Profile, 0, nullptr, &path)))
    {
        result = std::wstring(path) + L"\\FDA\\";
        CoTaskMemFree(path);
    }

    return result;
}

// -------------------------------------------------------
// load() — populates properties[]
// -------------------------------------------------------
bool FDAConfig::load()
{
    std::wstring userFdaDir = getUserFdaDir();
    if (userFdaDir.empty())
    {
        Logger::error("[CONFIG] Unable to determine user profile directory");
        return false;
    }

    if (!CreateDirectoryW(userFdaDir.c_str(), nullptr))
    {
        DWORD err = GetLastError();
        if (err != ERROR_ALREADY_EXISTS)
        {
            Logger::error("[CONFIG] Failed to create FDA directory, error : " + std::to_string(err));
            return false;
        }
    }

    std::wstring propertyFile = userFdaDir + L"fdaplugin.properties";
    propertyFilePath = wideToString(propertyFile);

    std::ifstream existingCheck(propertyFilePath);
    bool alreadyExists = existingCheck.is_open();
    existingCheck.close();

    if (!alreadyExists)
    {
        std::string outputDir = wideToString(getDefaultDownloadsPath());

        std::ofstream newFile(propertyFilePath, std::ios::binary);
        if (newFile.is_open())
        {
            newFile << "java.port=52000\r\n";
            newFile << "java.host=127.0.0.1\r\n";
            newFile << "c24.output.dir=" << outputDir << "\r\n";
            newFile.close();
            Logger::info("[CONFIG] Created default properties file : " + propertyFilePath);
        }
        else
        {
            Logger::error("[CONFIG] Failed to create default properties file");
            return false;
        }
    }

    Logger::info("[CONFIG] Loading : " + propertyFilePath);

    std::ifstream file(propertyFilePath);

    if (!file.is_open())
    {
        Logger::error("[CONFIG] fdaplugin.properties not found");
        return false;
    }

    Logger::info("[CONFIG] Reading fdaplugin.properties");

    fiEnvironments.clear();
    properties.clear();

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        size_t firstChar = line.find_first_not_of(" \t\r\n");
        if (firstChar == std::string::npos)
            continue;

        if (line[firstChar] == '#')
            continue;

        size_t separator = line.find('=');
        if (separator == std::string::npos)
            continue;

        std::string key = line.substr(0, separator);
        std::string value = line.substr(separator + 1);

        size_t ks = key.find_first_not_of(" \t");
        size_t ke = key.find_last_not_of(" \t");
        if (ks != std::string::npos)
            key = key.substr(ks, ke - ks + 1);

        size_t vs = value.find_first_not_of(" \t");
        size_t ve = value.find_last_not_of(" \t\r\n");
        if (vs != std::string::npos)
            value = value.substr(vs, ve - vs + 1);
        else
            value = "";

        if (key == "java.host")
            javaHost = value;
        else if (key == "java.port")
            javaPort = std::stoi(value);

        if (key.rfind("fi.", 0) == 0)
        {
            fiEnvironments.push_back(key);
            Logger::info("[CONFIG] FI Environment : " + key);
        }

        FDAProperty prop;
        prop.key = key;
        prop.value = value;
        setPropertyPermissions(prop);
        properties.push_back(prop);
    }

    file.close();

    Logger::info("[CONFIG] Host : " + javaHost);
    Logger::info("[CONFIG] Port : " + std::to_string(javaPort));

    return true;
}

// -------------------------------------------------------
// reload() — internal, called after add/update/delete
// -------------------------------------------------------
void FDAConfig::reload()
{
    fiEnvironments.clear();
    properties.clear();

    std::ifstream file(propertyFilePath);

    if (!file.is_open())
    {
        Logger::error("[CONFIG] Reload failed — fdaplugin.properties not found");
        return;
    }

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty()) continue;

        size_t firstChar = line.find_first_not_of(" \t\r\n");
        if (firstChar == std::string::npos || line[firstChar] == '#')
            continue;

        size_t separator = line.find('=');
        if (separator == std::string::npos) continue;

        std::string key = line.substr(0, separator);
        std::string value = line.substr(separator + 1);

        size_t ks = key.find_first_not_of(" \t");
        size_t ke = key.find_last_not_of(" \t");
        if (ks != std::string::npos) key = key.substr(ks, ke - ks + 1);

        size_t vs = value.find_first_not_of(" \t");
        size_t ve = value.find_last_not_of(" \t\r\n");
        if (vs != std::string::npos) value = value.substr(vs, ve - vs + 1);
        else value = "";

        if (key == "java.host") javaHost = value;
        else if (key == "java.port") javaPort = std::stoi(value);

        if (key.rfind("fi.", 0) == 0)
            fiEnvironments.push_back(key);

        FDAProperty prop;
        prop.key = key;
        prop.value = value;
        setPropertyPermissions(prop);
        properties.push_back(prop);
    }

    file.close();
    Logger::info("[CONFIG] Reloaded successfully");
}

// -------------------------------------------------------
// addProperty()
// -------------------------------------------------------
int FDAConfig::addProperty(const std::string& key, const std::string& value)
{
    if (isProtected(key))
    {
        Logger::error("[CONFIG] addProperty rejected — protected key : " + key);
        return -1;
    }

    int validKey = validateKey(key);

    if (validKey != 0)
    {
        return validKey;
    }

    if (!validateValue(value))
        return 4;

    // Duplicate check
    for (const FDAProperty& p : properties)
    {
        if (p.key == key)
        {
            Logger::error("[CONFIG] addProperty rejected — key already exists : " + key);
            return 5;
        }
    }

    Logger::info("[CONFIG] addProperty — path : " + propertyFilePath);

    std::ofstream file(propertyFilePath, std::ios::app | std::ios::binary);

    if (!file.is_open())
    {
        Logger::error("[CONFIG] addProperty failed — cannot open : " + propertyFilePath
            + " error : " + std::to_string(errno));
        return errno;
    }

    file << "\r\n" << key << "=" << value;
    file.flush();
    file.close();

    Logger::info("[CONFIG] Added : " + key + "=" + value);

    reload();
    return 0;
}

// -------------------------------------------------------
// updateProperty() — modifies only the matching line
// -------------------------------------------------------
int FDAConfig::updateProperty(const std::string& key, const std::string& value)
{
    if (isProtected(key))
    {
        Logger::error("[CONFIG] updateProperty rejected — protected key : " + key);
        return -1;
    }

    // Check editable via properties list
    bool found = false;
    for (const FDAProperty& p : properties)
    {
        if (p.key == key)
        {
            if (!p.editable)
            {
                Logger::error("[CONFIG] updateProperty rejected — not editable : " + key);
                return 6;
            }
            found = true;
            break;
        }
    }

    if (!found)
    {
        Logger::error("[CONFIG] updateProperty rejected — key not found : " + key);
        return 7;
    }

    if (!validateValue(value))
        return 4;

    // Read all lines
    std::ifstream inFile(propertyFilePath);
    if (!inFile.is_open())
    {
        Logger::error("[CONFIG] updateProperty failed — cannot read file");
        return errno;
    }

    std::vector<std::string> lines;
    std::string line;
    bool updated = false;

    while (std::getline(inFile, line))
    {
        // Check if this line is the target property
        size_t separator = line.find('=');
        if (separator != std::string::npos)
        {
            std::string lineKey = line.substr(0, separator);

            // Trim key
            size_t ks = lineKey.find_first_not_of(" \t");
            size_t ke = lineKey.find_last_not_of(" \t");
            if (ks != std::string::npos)
                lineKey = lineKey.substr(ks, ke - ks + 1);

            if (lineKey == key)
            {
                line = key + "=" + value;
                updated = true;
            }
        }

        lines.push_back(line);
    }

    inFile.close();

    if (!updated)
    {
        Logger::error("[CONFIG] updateProperty failed — key not found in file : " + key);
        return 7;
    }

    // Write back
    std::ofstream outFile(propertyFilePath, std::ios::binary);
    if (!outFile.is_open())
    {
        Logger::error("[CONFIG] updateProperty failed — cannot write : " + propertyFilePath
            + " error : " + std::to_string(errno));
        return errno;
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        outFile << lines[i];
        if (i < lines.size() - 1)
            outFile << "\r\n";
    }
    outFile.flush();
    outFile.close();

    Logger::info("[CONFIG] Updated : " + key + "=" + value);

    reload();
    return 0;
}

// -------------------------------------------------------
// deleteProperty() — removes only the matching line
// -------------------------------------------------------
int FDAConfig::deleteProperty(const std::string& key)
{
    if (isProtected(key))
    {
        Logger::error("[CONFIG] deleteProperty rejected — protected key : " + key);
        return -1;
    }

    // Check deletable via properties list
    bool found = false;
    for (const FDAProperty& p : properties)
    {
        if (p.key == key)
        {
            if (!p.deletable)
            {
                Logger::error("[CONFIG] deleteProperty rejected — not deletable : " + key);
                return 8;
            }
            found = true;
            break;
        }
    }

    if (!found)
    {
        Logger::error("[CONFIG] deleteProperty rejected — key not found : " + key);
        return 7;
    }

    // Read all lines, skip the target
    std::ifstream inFile(propertyFilePath);
    if (!inFile.is_open())
    {
        Logger::error("[CONFIG] deleteProperty failed — cannot read file");
        return errno;
    }

    std::vector<std::string> lines;
    std::string line;
    bool deleted = false;

    while (std::getline(inFile, line))
    {
        size_t separator = line.find('=');
        if (separator != std::string::npos)
        {
            std::string lineKey = line.substr(0, separator);

            size_t ks = lineKey.find_first_not_of(" \t");
            size_t ke = lineKey.find_last_not_of(" \t");
            if (ks != std::string::npos)
                lineKey = lineKey.substr(ks, ke - ks + 1);

            if (lineKey == key)
            {
                deleted = true;
                continue; // skip this line
            }
        }

        lines.push_back(line);
    }

    inFile.close();

    if (!deleted)
    {
        Logger::error("[CONFIG] deleteProperty failed — key not found in file : " + key);
        return 7;
    }

    // Write back
    std::ofstream outFile(propertyFilePath, std::ios::binary);
    if (!outFile.is_open())
    {
        Logger::error("[CONFIG] deleteProperty failed — cannot write : " + propertyFilePath
            + " error : " + std::to_string(errno));
        return errno;
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        outFile << lines[i];
        if (i < lines.size() - 1)
            outFile << "\r\n";
    }
    outFile.flush();
    outFile.close();

    Logger::info("[CONFIG] Deleted : " + key);

    reload();
    return 0;
}

// -------------------------------------------------------
// getC24EnvironmentNames()
// -------------------------------------------------------
std::vector<std::string> FDAConfig::getC24EnvironmentNames()
{
    std::vector<std::string> names;

    for (const FDAProperty& p : properties)
    {
        std::string name, field;
        if (isC24EnvKey(p.key, name, field))
        {
            if (std::find(names.begin(), names.end(), name) == names.end())
                names.push_back(name);
        }
    }

    return names;
}

// -------------------------------------------------------
// getC24Environment()
// -------------------------------------------------------
bool FDAConfig::getC24Environment(const std::string& name, FDAC24Environment& outEnv, bool& decryptFailed)
{
    bool found = false;
    decryptFailed = false;
    outEnv = FDAC24Environment();
    outEnv.name = name;

    for (const FDAProperty& p : properties)
    {
        std::string envName, field;
        if (isC24EnvKey(p.key, envName, field) && envName == name)
        {
            found = true;
            if (field == "host")          outEnv.host = p.value;
            else if (field == "port")     outEnv.port = p.value;
            else if (field == "username")
            {
                std::string dec = FDACrypto::decrypt(p.value);
                if (dec.empty() && !p.value.empty()) decryptFailed = true;
                outEnv.username = dec;
            }
            else if (field == "password")
            {
                std::string dec = FDACrypto::decrypt(p.value);
                if (dec.empty() && !p.value.empty()) decryptFailed = true;
                outEnv.password = dec;
            }
            else if (field == "bankId")   outEnv.bankId = p.value;
            else if (field == "bePath")   outEnv.bePath = p.value;
            else if (field == "fePath")   outEnv.fePath = p.value;
        }
    }

    return found;
}

// -------------------------------------------------------
// addC24Environment()
// -------------------------------------------------------
int FDAConfig::addC24Environment(const FDAC24Environment& env)
{
    if (env.name.empty())
    {
        Logger::error("[CONFIG] addC24Environment rejected — empty environment name");
        return 9;
    }

    if (env.name.find('=') != std::string::npos || env.name.find('.') != std::string::npos)
    {
        Logger::error("[CONFIG] addC24Environment rejected — invalid characters in name : " + env.name);
        return 10;
    }

    std::vector<std::string> existing = getC24EnvironmentNames();
    for (const std::string& n : existing)
    {
        if (n == env.name)
        {
            Logger::error("[CONFIG] addC24Environment rejected — environment already exists : " + env.name);
            return 5;
        }
    }

    if (!validateValue(env.host) || !validateValue(env.port) ||
        !validateValue(env.username) || !validateValue(env.password) ||
        !validateValue(env.bankId) || !validateValue(env.bePath) ||
        !validateValue(env.fePath))
    {
        return 4;
    }

    std::ofstream file(propertyFilePath, std::ios::app | std::ios::binary);
    if (!file.is_open())
    {
        Logger::error("[CONFIG] addC24Environment failed — cannot open : " + propertyFilePath
            + " error : " + std::to_string(errno));
        return errno;
    }

    const std::string base = "c24." + env.name + ".";
    file << "\r\n" << base << "host=" << env.host;
    file << "\r\n" << base << "port=" << env.port;
    file << "\r\n" << base << "username=" << FDACrypto::encrypt(env.username);
    file << "\r\n" << base << "password=" << FDACrypto::encrypt(env.password);
    file << "\r\n" << base << "bankId=" << env.bankId;
    file << "\r\n" << base << "bePath=" << env.bePath;
    file << "\r\n" << base << "fePath=" << env.fePath;
    file.flush();
    file.close();

    Logger::info("[CONFIG] Added C24 environment : " + env.name);

    reload();
    return 0;
}

// -------------------------------------------------------
// updateC24Environment()
// -------------------------------------------------------
int FDAConfig::updateC24Environment(const FDAC24Environment& env)
{
    std::vector<std::string> existing = getC24EnvironmentNames();
    bool found = false;
    for (const std::string& n : existing) if (n == env.name) { found = true; break; }

    if (!found)
    {
        Logger::error("[CONFIG] updateC24Environment rejected — environment not found : " + env.name);
        return 7;
    }

    if (!validateValue(env.host) || !validateValue(env.port) ||
        !validateValue(env.username) || !validateValue(env.password) ||
        !validateValue(env.bankId) || !validateValue(env.bePath) ||
        !validateValue(env.fePath))
    {
        return 4;
    }

    std::ifstream inFile(propertyFilePath);
    if (!inFile.is_open())
    {
        Logger::error("[CONFIG] updateC24Environment failed — cannot read file");
        return errno;
    }

    std::vector<std::string> lines;
    std::string line;
    const std::string base = "c24." + env.name + ".";

    while (std::getline(inFile, line))
    {
        size_t separator = line.find('=');
        if (separator != std::string::npos)
        {
            std::string lineKey = line.substr(0, separator);
            size_t ks = lineKey.find_first_not_of(" \t");
            size_t ke = lineKey.find_last_not_of(" \t");
            if (ks != std::string::npos) lineKey = lineKey.substr(ks, ke - ks + 1);

            if (lineKey == base + "host")          line = base + "host=" + env.host;
            else if (lineKey == base + "port")     line = base + "port=" + env.port;
            else if (lineKey == base + "username") line = base + "username=" + FDACrypto::encrypt(env.username);
            else if (lineKey == base + "password") line = base + "password=" + FDACrypto::encrypt(env.password);
            else if (lineKey == base + "bankId")   line = base + "bankId=" + env.bankId;
            else if (lineKey == base + "bePath")   line = base + "bePath=" + env.bePath;
            else if (lineKey == base + "fePath")   line = base + "fePath=" + env.fePath;
        }
        lines.push_back(line);
    }
    inFile.close();

    std::ofstream outFile(propertyFilePath, std::ios::binary);
    if (!outFile.is_open())
    {
        Logger::error("[CONFIG] updateC24Environment failed — cannot write : " + propertyFilePath
            + " error : " + std::to_string(errno));
        return errno;
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        outFile << lines[i];
        if (i < lines.size() - 1) outFile << "\r\n";
    }
    outFile.flush();
    outFile.close();

    Logger::info("[CONFIG] Updated C24 environment : " + env.name);

    reload();
    return 0;
}

// -------------------------------------------------------
// deleteC24Environment()
// -------------------------------------------------------
int FDAConfig::deleteC24Environment(const std::string& name)
{
    std::vector<std::string> existing = getC24EnvironmentNames();
    bool found = false;
    for (const std::string& n : existing) if (n == name) { found = true; break; }

    if (!found)
    {
        Logger::error("[CONFIG] deleteC24Environment rejected — environment not found : " + name);
        return 7;
    }

    std::ifstream inFile(propertyFilePath);
    if (!inFile.is_open())
    {
        Logger::error("[CONFIG] deleteC24Environment failed — cannot read file");
        return errno;
    }

    std::vector<std::string> lines;
    std::string line;
    const std::string base = "c24." + name + ".";

    while (std::getline(inFile, line))
    {
        size_t separator = line.find('=');
        if (separator != std::string::npos)
        {
            std::string lineKey = line.substr(0, separator);
            size_t ks = lineKey.find_first_not_of(" \t");
            size_t ke = lineKey.find_last_not_of(" \t");
            if (ks != std::string::npos) lineKey = lineKey.substr(ks, ke - ks + 1);

            if (lineKey.rfind(base, 0) == 0)
                continue; // skip — part of this environment
        }
        lines.push_back(line);
    }
    inFile.close();

    std::ofstream outFile(propertyFilePath, std::ios::binary);
    if (!outFile.is_open())
    {
        Logger::error("[CONFIG] deleteC24Environment failed — cannot write : " + propertyFilePath
            + " error : " + std::to_string(errno));
        return errno;
    }

    for (size_t i = 0; i < lines.size(); ++i)
    {
        outFile << lines[i];
        if (i < lines.size() - 1) outFile << "\r\n";
    }
    outFile.flush();
    outFile.close();

    Logger::info("[CONFIG] Deleted C24 environment : " + name);

    reload();
    return 0;
}

// -------------------------------------------------------
// Getters
// -------------------------------------------------------
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

const std::vector<FDAProperty>& FDAConfig::getProperties()
{
    return properties;
}

std::wstring FDAConfig::getDefaultDownloadsPath()
{
    PWSTR path = nullptr;
    std::wstring result;

    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Downloads, 0, nullptr, &path)))
    {
        result = path;
        CoTaskMemFree(path);
    }

    return result;
}

// -------------------------------------------------------
// wideToString()
// -------------------------------------------------------
std::string FDAConfig::wideToString(const std::wstring& value)
{
    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        value.c_str(), -1,
        nullptr, 0,
        nullptr, nullptr
    );

    if (size <= 0) return "";

    std::string result(size - 1, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0,
        value.c_str(), -1,
        &result[0], size,
        nullptr, nullptr
    );

    return result;
}