#pragma once

#include <Windows.h>
#include <commctrl.h>
#include "../Models/ValidationResult.h"

class ValidationPanel
{
public:
    static void init(HINSTANCE hInstance, HWND nppHandle);
    static void registerPanel();
    static void hidePanel();
    static void destroyPanel();

    static HWND getPanel();
    static void resize(int width, int height);

    static void showValidationResults(const ValidationResult& result);
    static void clear();

private:
    static bool _isRegistered;

    // Window Procedure
    static LRESULT CALLBACK panelProc(
        HWND hwnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

    // Instance and Parent Handles
    static HINSTANCE _hInstance;
    static HWND      _nppHandle;

    // Panel Controls
    static HWND _hPanel;
    static HWND _hHeader;
    static HWND _hListView;
    static HWND _hSummary;
    static HWND _hDeveloper;

    // Resources
    static HFONT _hFont;
    static HFONT _hBoldFont;
    static HFONT _hLinkFont;

    static std::vector<Issue> _issues;
};