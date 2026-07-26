#pragma once

#include <string>
#include <vector>

class AutoCompleteManager
{
public:
    static void showSuggestions(char ch);
    static std::vector<std::string> getSuggestions(const std::string& prefix);
};