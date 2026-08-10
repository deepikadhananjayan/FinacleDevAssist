#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "../Scintilla.h"

class ScintillaHelper
{
public:
    static HWND getCurrentEditor();
    static std::string getCurrentWord(HWND editor);
    static std::string getCurrentFilePath();
    static std::string getFileExtension(const std::string& filePath);
    static std::string getContentType(const std::string& filePath);
    static std::string getSelectedOrAllText();
    static void showAutoComplete(const std::string& prefix, const std::vector<std::string>& suggestions);
    static bool isValidFile(const std::string& filePath);
    static std::wstring getCurrentEditorFont();
    static int getCurrentEditorFontSize();
    static std::string getCurrentDocument();
    static void replaceSelectedContent(const std::string& content);
    static void replaceCurrentDocument(const std::string& content);
};