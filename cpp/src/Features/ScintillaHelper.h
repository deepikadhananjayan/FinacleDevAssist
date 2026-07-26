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
    static void showAutoComplete(const std::string& prefix, const std::vector<std::string>& suggestions);
    static std::string getCurrentFilePath();
    static bool isScriptFile(const std::string& filePath);
    static std::wstring getCurrentEditorFont();
    static int getCurrentEditorFontSize();
};