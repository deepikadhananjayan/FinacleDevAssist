#pragma once

#include <vector>
#include <string>
#include <mutex>

class TokenStore
{
public:
    static void setKeywords(const std::vector<std::string>& values);
    static void setFunctions(const std::vector<std::string>& values);
    static void setUserHooks(const std::vector<std::string>& values);

    static std::vector<std::string> getKeywords();
    static std::vector<std::string> getFunctions();
    static std::vector<std::string> getUserHooks();

private:
    static std::vector<std::string> keywords;
    static std::vector<std::string> functions;
    static std::vector<std::string> userHooks;
    static std::mutex mutex;
};   