#pragma once

#include <string>
#include <vector>

class FDAConfig
{
public:
    static bool load();
    static std::string getJavaHost();
    static int getJavaPort();
    static const std::vector<std::string>& getFIEnvironments();

private:
    static std::string wideToString(const std::wstring& value);
    static std::string javaHost;
    static int javaPort;
    static std::vector<std::string> fiEnvironments;
};