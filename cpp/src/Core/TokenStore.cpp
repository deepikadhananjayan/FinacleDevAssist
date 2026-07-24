#include "TokenStore.h"

std::vector<std::string> TokenStore::keywords;
std::vector<std::string> TokenStore::functions;
std::vector<std::string> TokenStore::userHooks;
std::mutex TokenStore::mutex;

void TokenStore::setKeywords(const std::vector<std::string>& values)
{
    std::lock_guard<std::mutex> lock(mutex);
    keywords = values;
}

void TokenStore::setFunctions(const std::vector<std::string>& values)
{
    std::lock_guard<std::mutex> lock(mutex);
    functions = values;
}

void TokenStore::setUserHooks(const std::vector<std::string>& values)
{
    std::lock_guard<std::mutex> lock(mutex);
    userHooks = values;
}

std::vector<std::string> TokenStore::getKeywords()
{
    std::lock_guard<std::mutex> lock(mutex);
    return keywords;
}

std::vector<std::string> TokenStore::getFunctions()
{
    std::lock_guard<std::mutex> lock(mutex);
    return functions;
}

std::vector<std::string> TokenStore::getUserHooks()
{
    std::lock_guard<std::mutex> lock(mutex);
    return userHooks;
}   