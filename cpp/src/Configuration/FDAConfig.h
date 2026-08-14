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

class FDAConfig
{
public:
    static bool load();
    static std::string getJavaHost();
    static int getJavaPort();
    static const std::vector<std::string>& getFIEnvironments();
    static const std::vector<FDAProperty>& getProperties();

    static int addProperty(const std::string& key, const std::string& value);
    static int updateProperty(const std::string& key, const std::string& value);
    static int deleteProperty(const std::string& key);

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
};