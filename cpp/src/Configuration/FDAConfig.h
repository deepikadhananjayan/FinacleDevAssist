#pragma once

#include <string>

class FDAConfig
{
public:
    static bool load();
    static std::string getJavaHost();
    static int getJavaPort();

private:
    static std::string wideToString(const std::wstring& value);
    static std::string javaHost;
    static int javaPort;
};