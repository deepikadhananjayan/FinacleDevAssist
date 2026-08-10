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

std::string ScintillaHelper::getSelectedOrAllText()
{
    HWND editor = getCurrentEditor();

    if (editor == nullptr)
    {
        Logger::error("[SCINTILLA] Could not get the Editor");
        return "";
    }

    int start = static_cast<int>(::SendMessage(editor, SCI_GETSELECTIONSTART, 0, 0));
    int end = static_cast<int>(::SendMessage(editor, SCI_GETSELECTIONEND, 0, 0));

    int length = 0;

    if (start != end)
    {
        length = end - start;
    }
    else
    {
        start = 0;
        length = static_cast<int>(::SendMessage(editor, SCI_GETLENGTH, 0, 0));
    }

    if (length <= 0)
        return "";

    Sci_TextRangeFull tr{};
    tr.chrg.cpMin = start;
    tr.chrg.cpMax = start + length;

    std::vector<char> buffer(length + 1, 0);
    tr.lpstrText = buffer.data();

    ::SendMessage(editor, SCI_GETTEXTRANGEFULL, 0, reinterpret_cast<LPARAM>(&tr));

    buffer.back() = '\0';

    return std::string(buffer.data());
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

std::string ScintillaHelper::getFileExtension(const std::string& filePath)
{
    size_t dotPos = filePath.find_last_of('.');

    if (dotPos == std::string::npos)
    {
        return "";
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

    return extension;
}

bool ScintillaHelper::isValidFile(const std::string& filePath)
{
    std::string extension = getFileExtension(filePath);
    return extension == ".scr";
}

std::string ScintillaHelper::getContentType(const std::string& filePath)
{
    std::string extension = getFileExtension(filePath);

    if (extension == ".java")
    {
        return "JAVA";
    }

    if (extension == ".js")
    {
        return "JS";
    }

    if (extension == ".xml")
    {
        return "XML";
    }

    if (extension == ".scr")
    {
        return "SCRIPT";
    }

    return "";
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

std::string ScintillaHelper::getCurrentDocument()
{
    HWND hSci = getCurrentEditor();

    if (!hSci) {
        return "";
    }

    // Get the length of the text in the document
    const int length = static_cast<int>(SendMessage(
        hSci,
        SCI_GETTEXTLENGTH,
        0,
        0
    ));

    if (length <= 0) {
        return "";
    }

    // Pre-allocate string buffer with space for the text plus the null terminator
    // C++17 guarantees contiguous storage compatible with C-style APIs
    std::string content(length + 1, '\0');

    // Retrieve the text directly into the string's internal buffer
    SendMessage(
        hSci,
        SCI_GETTEXT,
        length + 1,
        reinterpret_cast<LPARAM>(content.data())
    );

    // Resize to exclude the trailing null terminator added by Scintilla
    content.resize(length);

    return content;
}

void ScintillaHelper::replaceSelectedContent(const std::string& text)
{
    HWND editor = getCurrentEditor();

    if (editor == nullptr)
    {
        Logger::error("[SCINTILLA] Could not get the Editor");
        return;
    }

    ::SendMessage(
        editor,
        SCI_REPLACESEL,
        0,
        reinterpret_cast<LPARAM>(text.c_str())
    );
}

void ScintillaHelper::replaceCurrentDocument(const std::string& content)
{
    HWND hSci = getCurrentEditor();

    if (!hSci) {
        return;
    }

    // Save current caret position
    const int caretPos = static_cast<int>(SendMessage(
        hSci,
        SCI_GETCURRENTPOS,
        0,
        0
    ));

    // Begin an undo action group to treat the replacement as a single operation
    SendMessage(hSci, SCI_BEGINUNDOACTION, 0, 0);

    // Replace the entire document content
    SendMessage(hSci, SCI_SETTEXT, 0, reinterpret_cast<LPARAM>(content.c_str()));

    // Get the new document length to clamp the caret position
    const int newLength = static_cast<int>(SendMessage(
        hSci,
        SCI_GETTEXTLENGTH,
        0,
        0
    ));

    // Ensure the caret position is within the bounds of the new document
    const int clampedPos = (caretPos > newLength) ? newLength : caretPos;

    // Restore caret WITHOUT selection (SCI_SETSEL with identical start/end clears selection)
    // This also implicitly handles the anchor position correctly
    SendMessage(hSci, SCI_SETSEL, clampedPos, clampedPos);

    // Explicitly scroll the view to make the caret visible
    SendMessage(hSci, SCI_SCROLLCARET, 0, 0);

    // End the undo action group
    SendMessage(hSci, SCI_ENDUNDOACTION, 0, 0);
}