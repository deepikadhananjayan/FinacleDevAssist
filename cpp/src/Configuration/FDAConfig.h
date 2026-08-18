#pragma once

#include <string>
#include <vector>

struct FDAProperty
{
    std::string key;
    std::string value;
    bool editable;
    bool deletable;
};

struct FDAC24Environment
{
    std::string name;
    std::string host;
    std::string port;
    std::string username;
    std::string password;
    std::string bankId;
    std::string bePath;
    std::string fePath;
};

class FDAConfig
{
public:
    static bool load();
    static std::string getJavaHost();
    static int getJavaPort();
    static const std::vector<std::string>& getFIEnvironments();
    static const std::vector<FDAProperty>& getProperties();
    static bool isC24EnvironmentKey(const std::string& key);

    static int addProperty(const std::string& key, const std::string& value);
    static int updateProperty(const std::string& key, const std::string& value);
    static int deleteProperty(const std::string& key);

    // C24 environment group operations
    static std::vector<std::string> getC24EnvironmentNames();
    static bool getC24Environment(const std::string& name, FDAC24Environment& outEnv);
    static int addC24Environment(const FDAC24Environment& env);
    static int updateC24Environment(const FDAC24Environment& env);
    static int deleteC24Environment(const std::string& name);

private:
    static std::vector<FDAProperty> properties;
    static std::string              propertyFilePath;
    static std::string              javaHost;
    static int                      javaPort;
    static std::vector<std::string> fiEnvironments;

    static std::string  wideToString(const std::wstring& value);
    static void         setPropertyPermissions(FDAProperty& prop);
    static int          validateKey(const std::string& key);
    static bool         validateValue(const std::string& value);
    static bool         isProtected(const std::string& key);
    static void         reload();
    static std::wstring getDefaultDownloadsPath();

    // C24 helpers
    static bool isC24Field(const std::string& field);
    static bool isC24EnvKey(const std::string& key, std::string& outName, std::string& outField);
};