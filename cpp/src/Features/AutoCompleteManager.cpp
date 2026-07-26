#include "AutoCompleteManager.h"
#include "ScintillaHelper.h"
#include "../Core/TokenStore.h"
#include "../Utils/Logger.h"
#include <algorithm>

void AutoCompleteManager::showSuggestions(char ch)
{
    
    if (!isalnum((unsigned char)ch) &&
        ch != '_' &&
        ch != '$' &&
        ch != '<' &&
        ch != '-')
    {
        return;
    }

    HWND editor = ScintillaHelper::getCurrentEditor();

    if (editor == nullptr)
    {
        Logger::error("[AUTOCOMPLETE] Editor handle missing");
        return;
    }

    std::string currentWord = ScintillaHelper::getCurrentWord(editor);

    Logger::info(
        "[AUTOCOMPLETE] Current Word : " + currentWord);

    if (currentWord.length() < 2)
        return;

    auto suggestions = AutoCompleteManager::getSuggestions(currentWord);

    Logger::info("[AUTOCOMPLETE] Suggestions count : " +
        std::to_string(suggestions.size()));

    for (const auto& suggestion : suggestions)
    {
        Logger::info("[AUTOCOMPLETE] " + suggestion);
    }

    if (suggestions.empty())
        return;

    ScintillaHelper::showAutoComplete(currentWord, suggestions);
}

static std::string toLower(const std::string& value)
{
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        });
    return result;
}

std::vector<std::string> AutoCompleteManager::getSuggestions(const std::string& prefix)
{
    std::vector<std::string> result;
    std::string lowerPrefix = toLower(prefix);

    auto addMatches = [&](const std::vector<std::string>& values)
        {
            for (const auto& value : values)
            {
                std::string lowerValue = toLower(value);
                
                if (lowerValue.find(lowerPrefix) == 0)
                {
                    result.push_back(value);
                }
            }
        };

    addMatches(TokenStore::getKeywords());
    addMatches(TokenStore::getFunctions());
    addMatches(TokenStore::getUserHooks());

    return result;
}