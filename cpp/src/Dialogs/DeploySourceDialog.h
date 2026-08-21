#pragma once

#include <windows.h>
#include <string>
#include <vector>

struct DeployEnvSelectParams
{
    std::vector<std::string> environmentNames;
    std::string selectedEnvironment;
};

INT_PTR CALLBACK DeployEnvSelectDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

std::string browserForFolder(HWND hParent);