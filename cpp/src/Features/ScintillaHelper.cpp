#include "ScintillaHelper.h"
#include "../PluginDefinition.h"
#include "../Utils/Logger.h"
#include <algorithm>

HWND ScintillaHelper::getCurrentEditor()
{
    int current = 0;

    SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&current);

    if (current == 0)
        return nppData._scintillaMainHandle;

    return nppData._scintillaSecondHandle;
}

std::string ScintillaHelper::getCurrentWord(HWND editor)
{
    int pos = (int)::SendMessage(editor, SCI_GETCURRENTPOS, 0, 0);
    int start = pos;

    while (start > 0)
    {
        char c = (char)::SendMessage(editor, SCI_GETCHARAT, start - 1, 0);

        if (!isalnum((unsigned char)c) &&
            c != '_' &&
            c != '<' &&
            c != '-')
        {
            break;
        }

        start--;
    }

    int length = pos - start;
    if (length <= 0)
        return "";

    Sci_TextRangeFull tr{};

    tr.chrg.cpMin = start;
    tr.chrg.cpMax = pos;

    std::vector<char> buffer(length + 1);

    tr.lpstrText = buffer.data();

    ::SendMessage(editor, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

    return std::string(tr.lpstrText);
}


void ScintillaHelper::showAutoComplete(const std::string& prefix, const std::vector<std::string>& suggestions)
{
    HWND editor = getCurrentEditor();

    if (editor == nullptr)
    {
        Logger::error("[SCINTILLA] Could not get the Editor");
        return;
    }

    if (suggestions.empty())
    {
        return;
    }

    std::string list;
    for (size_t i = 0; i < suggestions.size(); ++i)
    {
        list += suggestions[i];
        if (i != suggestions.size() - 1)
            list += "|";
    }

    char separator = '|';

    ::SendMessage(editor, SCI_AUTOCSETSEPARATOR, (WPARAM)separator, 0);
    ::SendMessage(editor, SCI_AUTOCSETIGNORECASE, TRUE, 0);
    ::SendMessage(editor, SCI_AUTOCSHOW, (WPARAM)prefix.length(), (LPARAM)list.c_str());
}

std::string ScintillaHelper::getCurrentFilePath()
{
    TCHAR filePath[MAX_PATH] = { 0 };

    ::SendMessage(
        nppData._nppHandle,
        NPPM_GETFULLCURRENTPATH,
        (WPARAM)MAX_PATH,
        (LPARAM)filePath
    );

    if (filePath[0] == 0)
    {
        Logger::error("[SCINTILLA] No file opened");
        return "-1|No File Opened";
    }

#ifdef UNICODE

    int size = WideCharToMultiByte(
        CP_UTF8,
        0,
        filePath,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    std::string path(size - 1, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        filePath,
        -1,
        &path[0],
        size,
        NULL,
        NULL
    );

#else

    std::string path(filePath);

#endif

    Logger::info("[SCINTILLA] Current File Path : " + path);

    return path;
}

bool ScintillaHelper::isScriptFile(const std::string& filePath)
{
    size_t dotPos = filePath.find_last_of('.');

    if (dotPos == std::string::npos)
    {
        return false;
    }

    std::string extension = filePath.substr(dotPos);

    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char c)
        {
            return static_cast<char>(std::tolower(c));
        }
    );

    return extension == ".scr";
}

std::wstring ScintillaHelper::getCurrentEditorFont()
{
    HWND editor = getCurrentEditor();

    if (editor == nullptr)
    {
        return L"Segoe UI";
    }

    // Scintilla returns font names in UTF-8 encoding
    char fontName[128] = { 0 };

    ::SendMessage(
        editor,
        SCI_STYLEGETFONT,
        STYLE_DEFAULT,
        (LPARAM)fontName
    );

    // Check if empty
    if (fontName[0] == 0)
    {
        return L"Segoe UI";
    }

    // Convert UTF-8 to Wide Char
    wchar_t fontNameW[128] = { 0 };

    int result = MultiByteToWideChar(
        CP_UTF8,                // CodePage: UTF-8
        0,                      // dwFlags
        fontName,               // lpMultiByteStr
        -1,                     // cbMultiByte
        fontNameW,              // lpWideCharStr
        128                     // cchWideChar
    );

    if (result == 0)
    {
        // Conversion failed, fallback
        return L"Segoe UI";
    }

    return std::wstring(fontNameW);
}

int ScintillaHelper::getCurrentEditorFontSize()
{
    HWND editor = getCurrentEditor();

    if (editor == nullptr)
    {
        return 10;
    }

    // Get size in hundredths of a point (e.g., 1050 = 10.5 pt)
    int fontSizeHundredths = (int)::SendMessage(
        editor,
        SCI_STYLEGETSIZEFRACTIONAL,
        STYLE_DEFAULT,
        0
    );

    // Validate range (100 = 1pt, 4000 = 40pt)
    if (fontSizeHundredths < 100 || fontSizeHundredths > 4000)
    {
        return 10;
    }

    // Return integer points (truncates 10.9 to 10)
    // Or use: return (fontSizeHundredths + 50) / 100; for rounding
    return fontSizeHundredths / 100;
}